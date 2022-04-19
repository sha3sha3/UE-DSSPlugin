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

#include "CallbackManager.h"
#include "Misc/ScopeLock.h"

FCallbackManager::FCallbackManager()
{
}

FCallbackManager::~FCallbackManager()
{
    Clear(TEXT(""));
}

TTuple<FName, IHubConnection::FOnMethodCompletion&> FCallbackManager::RegisterCallback()
{
    FName Id = GenerateCallbackId();

    FScopeLock Lock(&CallbacksLock);
    IHubConnection::FOnMethodCompletion& qssq = Callbacks.Add(Id);

    return TTuple<FName, IHubConnection::FOnMethodCompletion&>(Id, qssq);
}

bool FCallbackManager::InvokeCallback(FName InCallbackId, const FSignalRValue& InArguments, bool InRemoveCallback)
{
    IHubConnection::FOnMethodCompletion Callback;

    {
        FScopeLock Lock(&CallbacksLock);

        if(!Callbacks.Contains(InCallbackId))
        {
            return false;
        }

        Callback = Callbacks[InCallbackId];

        if (InRemoveCallback)
        {
            Callbacks.Remove(InCallbackId);
        }
    }

    Callback.ExecuteIfBound(InArguments);
    return true;
}

bool FCallbackManager::RemoveCallback(FName InCallbackId)
{
    {
        FScopeLock Lock(&CallbacksLock);
        return Callbacks.Remove(InCallbackId) != 0;
    }
}

void FCallbackManager::Clear(const FString& ErrorMessage)
{
    {
        FScopeLock Lock(&CallbacksLock);

        for (auto& El : Callbacks)
        {
            El.Value.ExecuteIfBound(FSignalRValue()); // TODO send error message
        }

        Callbacks.Empty();
    }
}

FName FCallbackManager::GenerateCallbackId()
{
    const auto CallbackId = CurrentId++;
    return *FString::FromInt(CallbackId);
}
