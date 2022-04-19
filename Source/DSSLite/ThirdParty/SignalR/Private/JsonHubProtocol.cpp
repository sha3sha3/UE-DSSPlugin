/*
 * MIT License
 *
 * Copyright (c) 2020-2021 FrozenStorm Interactive
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "JsonHubProtocol.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/Base64.h"
#include "DSSLiteModule.h"

FName FJsonHubProtocol::Name() const
{
    return "json";
}

int FJsonHubProtocol::Version() const
{
    return 1;
}

TSharedPtr<FJsonValue> SerializeValue(const FSignalRValue& InValue)
{
    switch (InValue.GetType())
    {
    case FSignalRValue::EValueType::Null:
        {
            return MakeShared<FJsonValueNull>();
        }
    case FSignalRValue::EValueType::Boolean:
        {
            return MakeShared<FJsonValueBoolean>(InValue.AsBool());
        }
    case FSignalRValue::EValueType::Number:
        {
            return MakeShared<FJsonValueNumber>(InValue.AsNumber());
        }
    case FSignalRValue::EValueType::String:
        {
            return MakeShared<FJsonValueString>(InValue.AsString());
        }
    case FSignalRValue::EValueType::Object:
        {
            TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
            const auto& Object = InValue.AsObject();
            for (const auto& Val : Object)
            {
                JsonObject->SetField(Val.Key, SerializeValue(Val.Value));
            }
            return MakeShared<FJsonValueObject>(JsonObject);
        }
    case FSignalRValue::EValueType::Array:
        {
            TArray<TSharedPtr<FJsonValue>> JsonArray;
            const auto& Array = InValue.AsArray();
            for (const auto& Val : Array)
            {
                JsonArray.Add(SerializeValue(Val));
            }
            return MakeShared<FJsonValueArray>(JsonArray);
        }
    case FSignalRValue::EValueType::Binary:
        {
            return MakeShared<FJsonValueString>(FBase64::Encode(InValue.AsBinary()));
        }
    default:
        {
            UE_LOG(LogDSSLite, Error, TEXT("Cannot convert JSON value to string"));
            break;
        }
    }

    return nullptr;
}

FString FJsonHubProtocol::SerializeMessage(const FHubMessage* InMessage) const
{
    TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();

    switch (InMessage->MessageType)
    {
    case ESignalRMessageType::Invocation:
        {
            const FInvocationMessage* InvocationMessage = StaticCast<const FInvocationMessage*>(InMessage);
            JsonObject->SetNumberField(TEXT("type"), StaticCast<int>(InvocationMessage->MessageType));
            if (!InvocationMessage->InvocationId.IsEmpty())
            {
                JsonObject->SetStringField(TEXT("invocationId"), InvocationMessage->InvocationId);
            }
            JsonObject->SetStringField(TEXT("target"), InvocationMessage->Target);
            JsonObject->SetField(TEXT("arguments"), SerializeValue(InvocationMessage->Arguments));
            if (InvocationMessage->StreamIds.Num() > 0)
            {
                TArray<TSharedPtr<FJsonValue>> SteamIdsJsonArray;
                for (const FString& StreamId : InvocationMessage->StreamIds)
                {
                    SteamIdsJsonArray.Add(MakeShared<FJsonValueString>(StreamId));
                }
                JsonObject->SetArrayField(TEXT("streamIds"), SteamIdsJsonArray);
            }
            break;
        }
    case ESignalRMessageType::Completion:
        {
            const FCompletionMessage* CompletionMessage = StaticCast<const FCompletionMessage*>(InMessage);
            JsonObject->SetNumberField(TEXT("type"), StaticCast<int>(CompletionMessage->MessageType));
            JsonObject->SetStringField(TEXT("invocationId"), CompletionMessage->InvocationId);
            if (!CompletionMessage->Error.IsEmpty())
            {
                JsonObject->SetStringField(TEXT("error"), CompletionMessage->Error);
            }
            else if (CompletionMessage->HasResult)
            {
                JsonObject->SetField(TEXT("result"), SerializeValue(CompletionMessage->Result));
            }
            break;
        }
    case ESignalRMessageType::Ping:
        {
            const FPingMessage* PingMessage = StaticCast<const FPingMessage*>(InMessage);
            JsonObject->SetNumberField(TEXT("type"), StaticCast<int>(PingMessage->MessageType));
            break;
        }
    case ESignalRMessageType::Close:
        {
            const FCloseMessage* CloseMessage = StaticCast<const FCloseMessage*>(InMessage);
            check(CloseMessage != nullptr);
            JsonObject->SetNumberField(TEXT("type"), StaticCast<int>(CloseMessage->MessageType));
            if (CloseMessage->Error.IsSet())
            {
                JsonObject->SetStringField(TEXT("error"), CloseMessage->Error.GetValue());
            }
            break;
        }
    default:
        break;
    }

    FString Out;
    TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Out);
    if(FJsonSerializer::Serialize(JsonObject, JsonWriter))
    {
        return Out + RecordSeparator;
    }
    else
    {
        UE_LOG(LogDSSLite, Error, TEXT("Cannot convert JSON object to string"));
        return TEXT("");
    }
}

TArray<TSharedPtr<FHubMessage>> FJsonHubProtocol::ParseMessages(const FString& InStr) const
{
    TArray<TSharedPtr<FHubMessage>> Messages;

    FString TmpStr(InStr);

    int32 Pos = 0;
    while(TmpStr.FindChar(RecordSeparator, Pos))
    {
        FString MessagePayload = TmpStr.Mid(0, Pos);
        TSharedPtr<FHubMessage> Message = ParseMessage(MessagePayload);
        if (Message.IsValid())
        {
            Messages.Add(ParseMessage(MessagePayload));
        }

        TmpStr = TmpStr.Mid(Pos + 1);
    }

    return Messages;
}

FSignalRValue DeserializeValue(TSharedPtr<FJsonValue> InValue)
{
    switch (InValue->Type)
    {
    case EJson::Boolean:
        return FSignalRValue(InValue->AsBool());
    case EJson::Number:
        return FSignalRValue(InValue->AsNumber());
    case EJson::String:
        return FSignalRValue(InValue->AsString());
    case EJson::Array:
        {
            TArray<FSignalRValue> OutValues;
            for (const TSharedPtr<FJsonValue>& Val : InValue->AsArray())
            {
                OutValues.Add(DeserializeValue(Val));
            }
            return FSignalRValue(MoveTemp(OutValues));
        }
    case EJson::Object:
        {
            TMap<FString, FSignalRValue> OutValues;
            for (const auto& Pair : InValue->AsObject()->Values)
            {
                OutValues.Add(Pair.Key, DeserializeValue(Pair.Value));
            }
            return FSignalRValue(MoveTemp(OutValues));
        }
    case EJson::Null:
        return FSignalRValue(nullptr);
    default:
        return nullptr;
    }
}

TSharedPtr<FHubMessage> FJsonHubProtocol::ParseMessage(const FString& MessagePayload) const
{
    TSharedPtr<FJsonValue> JsonValue;
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(MessagePayload);
    if (!FJsonSerializer::Deserialize(JsonReader, JsonValue))
    {
        UE_LOG(LogDSSLite, Error, TEXT("Cannot unserialize SignalR message: %s: %s"), *JsonReader->GetErrorMessage(), *MessagePayload);
        return nullptr;
    }
    else if (JsonValue->Type != EJson::Object)
    {
        UE_LOG(LogDSSLite, Error, TEXT("Message is not a 'object' type"));
        return nullptr;
    }
    else
    {
        TSharedPtr<FJsonObject> Obj = JsonValue->AsObject();
        if (!Obj->HasTypedField<EJson::Number>(TEXT("type")))
        {
            UE_LOG(LogDSSLite, Error, TEXT("Field 'type' not found in message %s"), *MessagePayload);
            return nullptr;
        }
        else
        {
            TSharedPtr<FHubMessage> Message;

            switch (StaticCast<ESignalRMessageType>(Obj->GetNumberField(TEXT("type"))))
            {
            case ESignalRMessageType::Invocation:
            {
                if (!Obj->HasTypedField<EJson::String>(TEXT("target")))
                {
                    UE_LOG(LogDSSLite, Error, TEXT("Field 'target' not found in invocation message %s"), *MessagePayload);
                    return nullptr;
                }
                else if (!Obj->HasTypedField<EJson::Array>(TEXT("arguments")))
                {
                    UE_LOG(LogDSSLite, Error, TEXT("Field 'arguments' not found in invocation message %s"), *MessagePayload);
                    return nullptr;
                }

                FString Target = Obj->GetStringField(TEXT("target"));

                TArray<TSharedPtr<FJsonValue>> JsonArguments = Obj->GetArrayField(TEXT("arguments"));
                TArray<FSignalRValue> Arguments;
                for (const auto& JsonArgument : JsonArguments)
                {
                    Arguments.Add(DeserializeValue(JsonArgument));
                }

                FString InvocationId;
                if (Obj->HasTypedField<EJson::String>(TEXT("invocationId")))
                {
                    InvocationId = Obj->GetStringField(TEXT("invocationId"));
                }

                Message = MakeShared<FInvocationMessage>(InvocationId, Target, Arguments);

                // TODO: Stream Ids

                break;
            }
            case ESignalRMessageType::Completion:
            {
                if (!Obj->HasTypedField<EJson::String>(TEXT("invocationId")))
                {
                    UE_LOG(LogDSSLite, Error, TEXT("Field 'invocationId' not found in completion message %s"), *MessagePayload);
                    return nullptr;
                }

                FString InvocationId = Obj->GetStringField(TEXT("invocationId"));

                FString Error;
                if (Obj->HasTypedField<EJson::String>(TEXT("error")))
                {
                    Error = Obj->GetStringField(TEXT("error"));
                }

                bool bHasResult = false;
                FSignalRValue Result;
                if (Obj->HasField(TEXT("result")))
                {
                    bHasResult = true;
                    Result = DeserializeValue(Obj->TryGetField(TEXT("result")));
                }

                if (!Error.IsEmpty() && bHasResult)
                {
                    UE_LOG(LogDSSLite, Error, TEXT("Fields 'error' and 'result' properties are mutually exclusive in completion message %s"), *MessagePayload);
                    return nullptr;
                }

                Message = MakeShared<FCompletionMessage>(InvocationId, Error, Result, bHasResult);
                break;
            }
            case ESignalRMessageType::Ping:
            {
                Message = MakeShared<FPingMessage>();
                break;
            }
            case ESignalRMessageType::Close:
            {
                TSharedPtr<FCloseMessage> CloseMessage = MakeShared<FCloseMessage>();

                FString Error;
                if (Obj->TryGetStringField(TEXT("error"), Error))
                {
                    CloseMessage->Error = Error;
                }

                bool bAllowReconnect;
                if (Obj->TryGetBoolField(TEXT("allowReconnect"), bAllowReconnect))
                {
                    CloseMessage->bAllowReconnect = bAllowReconnect;
                }

                Message = CloseMessage;
                break;
            }
            default:
                break;
            }

            return Message;
        }
    }
}
