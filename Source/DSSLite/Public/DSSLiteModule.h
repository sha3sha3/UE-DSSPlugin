// Copyright (c) 2022 ForeFront electronics

#pragma once

#include "Modules/ModuleManager.h"
#include "../ThirdParty/SignalR/Public/IHubConnection.h"


DECLARE_LOG_CATEGORY_EXTERN(LogDSSLite, Log, All);

class FDSSLiteModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	DSSLITE_API static FDSSLiteModule& Get();

	DSSLITE_API TSharedPtr<IHubConnection> CreateHubConnection(const FString& InUrl, const FString& InToken, const TMap<FString, FString>& InHeaders = TMap<FString, FString>());

private:
	virtual bool SupportsDynamicReloading() override
	{
		return false;
	}

	virtual bool SupportsAutomaticShutdown() override
	{
		return false;
	}
	/** Singleton Instance */
	static FDSSLiteModule* Singleton;

	/** Whether this module has been initialized */
	bool bInitialized = false;
};
