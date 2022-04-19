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

#pragma once

#include "CoreMinimal.h"
#include "MessageType.h"
#include "../Public/SignalRValue.h"

struct FHubMessage
{
protected:
    FHubMessage(ESignalRMessageType InMessageType) :
        MessageType(InMessageType)
    {
    }

public:
    virtual ~FHubMessage() {}

    const ESignalRMessageType MessageType;
};

 struct FBaseInvocationMessage : FHubMessage
{
 protected:
     FBaseInvocationMessage(const FString& InInvocationId, ESignalRMessageType InMessageType): FHubMessage(InMessageType),
         InvocationId(InInvocationId)
     {
     }

 public:
     const FString InvocationId;
};

struct FInvocationMessage : FBaseInvocationMessage
{
    FInvocationMessage(const FString& InInvocationId, const FString& InTarget, const TArray<FSignalRValue>& InArgs, const TArray<FString>& InStreamIds = TArray<FString>()) :
        FBaseInvocationMessage(InInvocationId, ESignalRMessageType::Invocation),
        Target(InTarget),
        Arguments(InArgs),
        StreamIds(InStreamIds)
    {
    }

    FInvocationMessage(FString&& InInvocationId, FString&& InTarget, TArray<FSignalRValue>&& InArgs, TArray<FString>&& InStreamIds = TArray<FString>()) :
        FBaseInvocationMessage(InInvocationId, ESignalRMessageType::Invocation),
        Target(InTarget),
        Arguments(InArgs),
        StreamIds(InStreamIds)
    {
    }

    FString Target;
    TArray<FSignalRValue> Arguments;
    TArray<FString> StreamIds;
};

struct FCompletionMessage : FBaseInvocationMessage
{
    FCompletionMessage(const FString& InInvocationId, const FString& InError, const FSignalRValue& InResult, bool InHasResult):
        FBaseInvocationMessage(InInvocationId, ESignalRMessageType::Completion),
        Error(InError),
        HasResult(InHasResult),
        Result(InResult)
    { }

    FCompletionMessage(FString&& InInvocationId, FString&& InError, FSignalRValue&& InResult, bool InHasResult) :
        FBaseInvocationMessage(InInvocationId, ESignalRMessageType::Completion),
        Error(InError),
        HasResult(InHasResult),
        Result(InResult)
    {
    }

    FString Error;
    bool HasResult;
    FSignalRValue Result;
};

struct FPingMessage : FHubMessage
{
    FPingMessage() : FHubMessage(ESignalRMessageType::Ping)
    {
    }
};

struct FCloseMessage : FHubMessage
{
    FCloseMessage() : FHubMessage(ESignalRMessageType::Close)
    {
    }

    TOptional<FString> Error;
    TOptional<bool> bAllowReconnect;
};

class DSSLITE_API IHubProtocol
{
public:
    virtual ~IHubProtocol();

    virtual FName Name() const = 0;
    virtual int Version() const = 0;

    virtual FString SerializeMessage(const FHubMessage*) const = 0;
    virtual TArray<TSharedPtr<FHubMessage>> ParseMessages(const FString&) const = 0;
};
