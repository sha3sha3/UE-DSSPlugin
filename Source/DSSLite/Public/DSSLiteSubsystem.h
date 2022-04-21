// Copyright (c) 2022 Dynamic Servers Systems

#pragma once

#include "CoreMinimal.h"
#include "Misc/DateTime.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TravelingOptions.h"
#include "DSSLiteSubsystem.generated.h"

class IHubConnection;

UCLASS(DisplayName="DSSLiteSubsystem")
class DSSLITE_API UDSSLiteSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ConnectWithToken", Keywords = ""), Category = "DSSLiteSubsystem")
    void ConnectWithToken(FString Connection, FString Token);
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Connect", Keywords = ""), Category = "DSSLiteSubsystem")
	void Connect(FString Connection, FString PlayerName, FString SigningKey);/*key should not be base64 encoded*/

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FConnected, FString, ClientID, FString, ConnectionID, FDateTime, Timestamp);/*on connected to dss*/
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_EightParams(FTravel, FString, ServerIP, int64, ServerPort, FString, PlayerName, FString, ServerConnectionID,TravelOptions, Options, FString, Tag, FVector, Coordinates, float, Yaw);//on travel async
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FServerTerminate);//on receive termination request
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDisconnected);//on disconnected from DSS
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FConnectionError, FString, Error);/*on connection error happened after connection attempt*/
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FShowLoadingScreen);/*triggered on travel*/
	
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnConnected", Keywords = ""), Category = "DSSLiteSubsystem")
	FConnected OnConnected;
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnDisconnected", Keywords = ""), Category = "DSSLiteSubsystem")
	FDisconnected OnDisconnected;
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnConnectionError", Keywords = ""), Category = "DSSLiteSubsystem")
	FConnectionError OnConnectionError;
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "ShowLoadingScreen", Keywords = ""), Category = "DSSLiteSubsystem")
	FShowLoadingScreen ShowLoadingScreen;
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnTravel", Keywords = ""), Category = "DSSLiteSubsystem")
	FTravel OnTravel;
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "ServerTerminate", Keywords = ""), Category = "DSSLiteSubsystem")
	FServerTerminate ServerTerminate;

	UFUNCTION()
	void Disconnected();
	UFUNCTION()
	void ConnectionError(const FString& ErrorMSG);
	UFUNCTION()
	void Connected();

	

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "OnClientTravelAsync", Keywords = ""), Category = "DSSLiteSubsystem")
	void OnClientTravelAsync(FString MapName, bool bIsDungeon, FString InstanceID, TravelOptions TravelOptions, FString Tag, FVector Location, float Yaw) {
		if (GetGameInstance()->IsDedicatedServerInstance() && IsConnected())
			return;
		TravelAsync(MapName, bIsDungeon, InstanceID, TravelOptions, Tag, Location, Yaw);
	}
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "OnServerTravelAsync", Keywords = ""), Category = "DSSLiteSubsystem")
	void OnServerTravelAsync(FString MapName, bool bIsDungeon, FString InstanceID, TravelOptions TravelOptions, FString Tag, FVector Location, float Yaw,FString CharacterName) {
		if (!GetGameInstance()->IsDedicatedServerInstance() && IsConnected())
			return;
		TravelAsync(MapName, bIsDungeon, InstanceID, TravelOptions, Tag, Location, Yaw, CharacterName);
	}
	

	UFUNCTION(BlueprintPure, meta = (DisplayName = "GetConnectionID", Keywords = ""), Category = "DSSLiteSubsystem")
	FString GetConnectionID() const{
		return ConnectionID;
	}

	UFUNCTION(BlueprintPure, meta = (DisplayName = "GetClientID", Keywords = ""), Category = "DSSLiteSubsystem")
	FString GetClientID() const {
		return ClientID;
	}

	UFUNCTION(BlueprintPure, meta = (DisplayName = "GetCreationDate", Keywords = ""), Category = "DSSLiteSubsystem")
	FDateTime GetCreationDate() const {
		return CreatedOn;
	}

	UFUNCTION(BlueprintPure, meta = (DisplayName = "GetServerPort", Keywords = ""), Category = "DSSLiteSubsystem")
	int32 GetServerPort() const{
		UWorld* World = GetGameInstance()->GetWorld();
		return (World && World->IsServer()) ? World->URL.Port : -1;
	}

	UFUNCTION(BlueprintPure, meta = (DisplayName = "IsConnected", Keywords = ""), Category = "DSSLiteSubsystem")
		bool IsConnected() const;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Disconnect", Keywords = ""), Category = "DSSLiteSubsystem")
		void Disconnect();

	UPROPERTY(BlueprintReadOnly, Category = "DSSLiteSubsystem")
	bool bIsManuallyLaunched;
	UPROPERTY(BlueprintReadWrite, Category = "DSSLiteSubsystem")
	bool bAutoClientTravel;//Travel player to new server automatically, set false to override the logic
	UPROPERTY(BlueprintReadWrite, Category = "DSSLiteSubsystem")
	bool bAutoServerClose;//terminate UE4 server automatically when it is empty, set false to override the logic

	

	UDSSLiteSubsystem();

private:
	TSharedPtr<IHubConnection> Hub = nullptr;
	FString ConnectionID = "";
	FString ClientID = "";
	FDateTime CreatedOn = FDateTime::Now();
	void TravelAsync(FString MapName, bool bIsDungeon, FString InstanceID, TravelOptions TravelOptions, FString Tag, FVector Location, float Yaw, FString CharacterName="");
	
	
protected:
	int DSSPort = 5000;//make sure to use the same port on the server
};

