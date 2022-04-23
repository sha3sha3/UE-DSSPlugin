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

#include "Connection.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "WebSocketsModule.h"
#include "DSSLiteModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IHttpRequest.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

FConnection::FConnection(const FString& InHost, const FString& InToken, const TMap<FString, FString>& InHeaders):
    Host(InHost),
    Token(InToken),
    Headers(InHeaders)   
{
}

void FConnection::Connect()
{
    Negotiate();
}

bool FConnection::IsConnected()
{
    return Connection.IsValid() && Connection->IsConnected();
}

void FConnection::Send(const FString& Data)
{
    if (Connection.IsValid())
    {
        Connection->Send(Data);
    }
    else
    {
        UE_LOG(LogDSSLite, Error, TEXT("Cannot send data to non connected websocket."));
    }
}

void FConnection::Close(int32 Code, const FString& Reason)
{
    if(Connection.IsValid())
    {
        Connection->Close(Code, Reason);
    }
    else
    {
        UE_LOG(LogDSSLite, Error, TEXT("Cannot close non connected websocket."));
    }
}

IWebSocket::FWebSocketConnectedEvent& FConnection::OnConnected()
{
    return OnConnectedEvent;
}

IWebSocket::FWebSocketConnectionErrorEvent& FConnection::OnConnectionError()
{
    return OnConnectionErrorEvent;
}

IWebSocket::FWebSocketClosedEvent& FConnection::OnClosed()
{
    return OnClosedEvent;
}

IWebSocket::FWebSocketMessageEvent& FConnection::OnMessage()
{
    return OnMessageEvent;
}

void FConnection::Negotiate()
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();


    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->OnProcessRequestComplete().BindSP(AsShared(), &FConnection::OnNegotiateResponse);
    HttpRequest->SetURL(Host + TEXT("/negotiate?negotiateVersion=1&access_token="+Token));
    HttpRequest->ProcessRequest();
}

void FConnection::OnNegotiateResponse(FHttpRequestPtr InRequest, FHttpResponsePtr InResponse, bool bConnectedSuccessfully)
{
    if(InResponse->GetResponseCode() != 200)
    {
        UE_LOG(LogDSSLite, Error, TEXT("Negotiate failed with status code %d"), InResponse->GetResponseCode());
        return;
    }

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(InResponse->GetContentAsString());

    if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
    {
        if(JsonObject->HasField(TEXT("error")))
        {
            // TODO
        }
        else
        {
            if (JsonObject->HasField(TEXT("ProtocolVersion")))
            {
                UE_LOG(LogDSSLite, Error, TEXT("Detected a connection attempt to an ASP.NET SignalR Server. This client only supports connecting to an ASP.NET Core SignalR Server. See https://aka.ms/signalr-core-differences for details."));
                return;
            }

            if (JsonObject->HasTypedField<EJson::String>(TEXT("url")))
            {
                FString RedirectionUrl = JsonObject->GetStringField(TEXT("url"));
                FString AccessToken = JsonObject->GetStringField(TEXT("accessToken"));
                // TODO: redirection
                return;
            }

            if (JsonObject->HasTypedField<EJson::Array>(TEXT("availableTransports")))
            {
                // check if support WebSockets with Text format
                bool bIsCompatible = false;
                for (TSharedPtr<FJsonValue> TransportData : JsonObject->GetArrayField(TEXT("availableTransports")))
                {
                    if(TransportData.IsValid() && TransportData->Type == EJson::Object)
                    {
                        TSharedPtr<FJsonObject> TransportObj = TransportData->AsObject();
                        if(TransportObj->HasTypedField<EJson::String>(TEXT("transport")) && TransportObj->GetStringField(TEXT("transport")) == TEXT("WebSockets") && TransportObj->HasTypedField<EJson::Array>(TEXT("transferFormats")))
                        {
                            for (TSharedPtr<FJsonValue> TransportFormatData : TransportObj->GetArrayField(TEXT("transferFormats")))
                            {
                                if (TransportFormatData.IsValid() && TransportFormatData->Type == EJson::String && TransportFormatData->AsString() == TEXT("Text"))
                                {
                                    bIsCompatible = true;
                                }
                            }
                        }
                    }
                }

                if(!bIsCompatible)
                {
                    UE_LOG(LogDSSLite, Error, TEXT("The server does not support WebSockets which is currently the only transport supported by this client."));
                    return;
                }
            }

            if (JsonObject->HasTypedField<EJson::String>(TEXT("connectionId")))
            {
                ConnectionId = JsonObject->GetStringField(TEXT("connectionId"));
            }

            if (JsonObject->HasTypedField<EJson::String>(TEXT("connectionToken")))
            {
                ConnectionId = JsonObject->GetStringField(TEXT("connectionToken"));
            }

            StartWebSocket();
        }
    }
    else
    {
        UE_LOG(LogDSSLite, Error, TEXT("Cannot parse negotiate response: %s"), *InResponse->GetContentAsString());
    }
}

void FConnection::StartWebSocket()
{
    const FString COnver = ConvertToWebsocketUrl(Host+"?access_token="+Token);
    Connection = FWebSocketsModule::Get().CreateWebSocket(COnver, FString(), Headers);

    if(Connection.IsValid())
    {
        Connection->OnConnected().AddLambda([Self = TWeakPtr<FConnection>(AsShared())]()
        {
            if (TSharedPtr<FConnection> SharedSelf = Self.Pin())
            {
                SharedSelf->OnConnectedEvent.Broadcast();
            }
        });
        Connection->OnConnectionError().AddLambda([Self = TWeakPtr<FConnection>(AsShared())](const FString& ErrString)
        {
            UE_LOG(LogDSSLite, Warning, TEXT("Websocket err: %s"), *ErrString);

            if (TSharedPtr<FConnection> SharedSelf = Self.Pin())
            {
                SharedSelf->OnConnectionErrorEvent.Broadcast(ErrString);
            }
        });
        Connection->OnClosed().AddLambda([Self = TWeakPtr<FConnection>(AsShared())](int32 StatusCode, const FString& Reason, bool bWasClean)
        {
            if (TSharedPtr<FConnection> SharedSelf = Self.Pin())
            {
                SharedSelf->OnClosedEvent.Broadcast(StatusCode, Reason, bWasClean);
            }
        });
        Connection->OnMessage().AddLambda([Self = TWeakPtr<FConnection>(AsShared())](const FString& MessageString)
        {
            if (TSharedPtr<FConnection> SharedSelf = Self.Pin())
            {
                SharedSelf->OnMessageEvent.Broadcast(MessageString);
            }
        });

        Connection->Connect();
    }
    else
    {
        UE_LOG(LogDSSLite, Error, TEXT("Cannot start websocket."));
    }
}

FString FConnection::ConvertToWebsocketUrl(const FString& Url)
{
    const FString TrimmedUrl = Url.TrimStartAndEnd();

    if (TrimmedUrl.StartsWith(TEXT("https://")))
    {
        return TEXT("wss") + TrimmedUrl.RightChop(5);
    }
    else if (TrimmedUrl.StartsWith(TEXT("http://")))
    {
        return TEXT("ws") + TrimmedUrl.RightChop(4);
    }
    else
    {
        return Url;
    }
}
