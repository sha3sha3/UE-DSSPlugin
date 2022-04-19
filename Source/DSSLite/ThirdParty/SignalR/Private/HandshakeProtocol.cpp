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

#include "HandshakeProtocol.h"
#include "IHubProtocol.h"
#include "JsonHubProtocol.h"
#include "Serialization/JsonSerializer.h"
#include "DSSLiteModule.h"

FString FHandshakeProtocol::CreateHandshakeMessage(TSharedPtr<IHubProtocol> InProtocol)
{
    TMap<FString, TSharedPtr<FJsonValue>> Values
    {
        { "protocol", MakeShared<FJsonValueString>(InProtocol->Name().ToString()) },
        { "version", MakeShared<FJsonValueNumber>(InProtocol->Version()) },
    };
    TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>();
    Obj->Values = Values;

    FString Out;
    TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Out);
    FJsonSerializer::Serialize(Obj.ToSharedRef(), JsonWriter);

    return Out + FJsonHubProtocol::RecordSeparator;
}

TTuple<TSharedPtr<FJsonObject>, FString> FHandshakeProtocol::ParseHandshakeResponse(const FString& Response)
{
    int32 Pos = -1;
    if(!Response.FindChar(FJsonHubProtocol::RecordSeparator, Pos))
    {
        return MakeTuple<TSharedPtr<FJsonObject>, FString>(nullptr, Response.Mid(0));
    }

    FString HandshakeMessage = Response.Mid(0, Pos);

    TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();

    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(HandshakeMessage);
    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject))
    {
        UE_LOG(LogDSSLite, Warning, TEXT("Cannot unserialize handshake message: %s"), *JsonReader->GetErrorMessage());
        JsonObject = nullptr;
    }

    FString RemainingData = Response.Mid(Pos + 1);

    return MakeTuple(JsonObject, RemainingData);
}
