// Copyright (c) 2022 ForeFront electronics

#include "DSSLiteModule.h"
#include "Modules/ModuleManager.h"
#include "WebSocketsModule.h"
#include "Engine/Engine.h"
#include "../ThirdParty/SignalR/Private/HubConnection.h"


DEFINE_LOG_CATEGORY(LogDSSLite);
#define LOCTEXT_NAMESPACE "FDSSLiteModule"

FDSSLiteModule* FDSSLiteModule::Singleton = nullptr;

FDSSLiteModule& FDSSLiteModule::Get()
{
    if ( Singleton == nullptr)
    {
        check(IsInGameThread());
        FModuleManager::LoadModuleChecked<FDSSLiteModule>("DSSLite");
    }
    check(Singleton);
    return *Singleton;
}

void FDSSLiteModule::StartupModule()
{
	
    Singleton = this;
    bInitialized = true;
    FModuleManager::LoadModuleChecked<FWebSocketsModule>("WebSockets");
}

void FDSSLiteModule::ShutdownModule()
{
    bInitialized = false;
	
}

TSharedPtr<IHubConnection> FDSSLiteModule::CreateHubConnection(const FString& InUrl, const FString& InToken, const TMap<FString, FString>& InHeaders)
{
    check(bInitialized);
    return MakeShared<FHubConnection>(InUrl, InToken, InHeaders);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDSSLiteModule, DSSLite)