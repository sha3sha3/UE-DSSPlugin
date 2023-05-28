// Copyright (c) 2022 Dynamic Servers Systems


#include "DSSLiteSubsystem.h"
#include "Engine/Engine.h"
#include "DSSLiteModule.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerState.h"
#include "PlayerEvent.h"
#include "../ThirdParty/jwt-cpp/jwt.h"
#include <Runtime/Slate/Public/Framework/Application/SlateApplication.h>





UDSSLiteSubsystem::UDSSLiteSubsystem():
	bIsManuallyLaunched(false),
	bAutoClientTravel(true),
	bAutoServerClose(true)
{

}

void UDSSLiteSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	/*
	*
	*Only applicable for servers
	*/
	if (GetGameInstance()->IsDedicatedServerInstance() && !GetWorld()->IsPlayInEditor() && !GetWorld()->IsPlayInPreview())
	{
		if (FParse::Value(FCommandLine::Get(), TEXT("DSSPort"), DSSPort))
		{
			UE_LOG(LogDSSLite, Display, TEXT("DSSPort=%d"), DSSPort);
		}
		else
		{
			UE_LOG(LogDSSLite, Error, TEXT("DSSPort is unknown."));
			FGenericPlatformMisc::RequestExit(false);
			return;
		}
		FParse::Bool(FCommandLine::Get(), TEXT("IsManuallyLaunched"), bIsManuallyLaunched);
		FString AuthenticationKey;
		if (FParse::Value(FCommandLine::Get(), TEXT("AuthKey"), AuthenticationKey))
		{
			UE_LOG(LogDSSLite, Display, TEXT("AuthenticationKey=%s"), *AuthenticationKey);
		}
		else
		{
			UE_LOG(LogDSSLite, Error, TEXT("Authentication Key is unknown."));
			FGenericPlatformMisc::RequestExit(false);
			return;
		}
		jwt::builder<jwt::picojson_traits> JwtGenerator = jwt::create();
		
		JwtGenerator.set_payload_claim(TCHAR_TO_ANSI(*FString("port")), jwt::claim(std::string(TCHAR_TO_ANSI(*FString::FromInt(GetServerPort())))));
		JwtGenerator.set_payload_claim(TCHAR_TO_ANSI(*FString("role")), jwt::claim(std::string(TCHAR_TO_ANSI(*FString("server")))));
		JwtGenerator.set_payload_claim(TCHAR_TO_ANSI(*FString("key")), jwt::claim(std::string(TCHAR_TO_ANSI(*AuthenticationKey))));
		JwtGenerator.set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds{ 60 });//expire after 60 sec

		FString token = FString(UTF8_TO_TCHAR(JwtGenerator.sign(jwt::algorithm::hs256{ TCHAR_TO_ANSI(*AuthenticationKey) }).c_str()));
		UE_LOG(LogDSSLite, Display, TEXT("Token=%s"),*token);
		FString ConnString= FString::Printf(TEXT("http://127.0.0.1:%s"), *FString::FromInt(DSSPort));
		UE_LOG(LogDSSLite, Display, TEXT("Connecting to=%s"), *ConnString);
		ConnectWithToken(ConnString, token);
		

	}
	else if (GetGameInstance()->IsDedicatedServerInstance() && (GetWorld()->IsPlayInEditor() ||  GetWorld()->IsPlayInPreview()))
	{
		//testing in editor
		jwt::builder<jwt::picojson_traits> JwtGenerator = jwt::create();

		JwtGenerator.set_payload_claim(TCHAR_TO_ANSI(*FString("port")), jwt::claim(std::string(TCHAR_TO_ANSI(*FString::FromInt(GetServerPort())))));
		JwtGenerator.set_payload_claim(TCHAR_TO_ANSI(*FString("role")), jwt::claim(std::string(TCHAR_TO_ANSI(*FString("server")))));
		JwtGenerator.set_payload_claim(TCHAR_TO_ANSI(*FString("key")), jwt::claim(std::string("0000000000")));//no need for authorization
		JwtGenerator.set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds{ 60 });//expire after 60 sec

		FString token = FString(UTF8_TO_TCHAR(JwtGenerator.sign(jwt::algorithm::hs256{ "0000000000" }).c_str()));
		UE_LOG(LogDSSLite, Display, TEXT("Token=%s"), *token);
		FString ConnString = FString::Printf(TEXT("http://127.0.0.1:%s"), *FString::FromInt(DSSPort));
		UE_LOG(LogDSSLite, Display, TEXT("Connecting to=%s"), *ConnString);
		ConnectWithToken(ConnString, token);
	}
	else
	{
		
	}
}

void UDSSLiteSubsystem::Deinitialize()
{

}

void UDSSLiteSubsystem::ConnectWithToken(FString Connection, FString Token) {

	if (IsConnected())
		return;//already connected

	if(GetGameInstance()->IsDedicatedServerInstance())
		Connection.Append(TEXT("/ServersHub"));
	else
		Connection.Append(TEXT("/ClientsHub"));
	if (!Connection.ToLower().StartsWith("http"))
		Connection = "http://" + Connection;
	Hub = FDSSLiteModule::Get().CreateHubConnection(Connection, Token);
	Hub->Start();
	Hub->OnConnectionError().AddUObject(this, &ThisClass::ConnectionError);
	Hub->OnConnected().AddUObject(this, &UDSSLiteSubsystem::Connected);
	
	
	Hub->On(TEXT("OnConnect")).BindLambda([&](const TArray<FSignalRValue>& Arguments) 
		{
			FDateTime Timestamp= FDateTime::Now();
			bool bDateTimeParsed = FDateTime::Parse(Arguments[1].AsString(), Timestamp);
			ClientID = Arguments[0].AsString();
			CreatedOn = Timestamp;
		OnConnected.Broadcast(ClientID, Timestamp);

		if (GetGameInstance()->IsDedicatedServerInstance())
		{
			FGameModeEvents::OnGameModePostLoginEvent().AddUObject(this, &UDSSLiteSubsystem::OnPlayerConnect);

			FGameModeEvents::OnGameModeLogoutEvent().AddUObject(this, &UDSSLiteSubsystem::OnPlayerLogout);
		}
		});
}

void UDSSLiteSubsystem::Connect(FString Connection, FString PlayerName, FString SigningKey) {

	if (IsConnected())
		return;//already connected
	if (GetGameInstance()->IsDedicatedServerInstance())
		return;//callable on clients only, i don't advice to use it
	
	Connection.Append(TEXT("/ClientsHub"));
	if (!Connection.ToLower().StartsWith("http"))
		Connection = "http://" + Connection;///make sure ip contains http

	jwt::builder<jwt::picojson_traits> JwtGenerator = jwt::create();
	JwtGenerator.set_payload_claim(TCHAR_TO_ANSI(*FString("name")), jwt::claim(std::string(TCHAR_TO_ANSI(*PlayerName))));
	JwtGenerator.set_payload_claim(TCHAR_TO_ANSI(*FString("role")), jwt::claim(std::string(TCHAR_TO_ANSI(*FString("client")))));
	JwtGenerator.set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds{ 60 });//expire after 60 sec
	FString Token = FString(UTF8_TO_TCHAR(JwtGenerator.sign(jwt::algorithm::hs256{ TCHAR_TO_ANSI(*SigningKey) }).c_str()));


	Hub = FDSSLiteModule::Get().CreateHubConnection(Connection, Token);
	Hub->Start();
	Hub->OnConnectionError().AddUObject(this, &ThisClass::ConnectionError);
	Hub->OnConnected().AddUObject(this, &UDSSLiteSubsystem::Connected);


	Hub->On(TEXT("OnConnect")).BindLambda([&](const TArray<FSignalRValue>& Arguments)
		{
			FDateTime Timestamp = FDateTime::Now();
			bool bDateTimeParsed = FDateTime::Parse(Arguments[1].AsString(), Timestamp);
			ClientID = Arguments[0].AsString();
			CreatedOn = Timestamp;
			OnConnected.Broadcast(Arguments[0].AsString(), Timestamp);
		});
}

void UDSSLiteSubsystem::TravelAsync(FString MapName, bool bIsDungeon, FString InstanceID, TravelOptions TravelOptions, FString Tag, FVector Location, float Yaw, FString CharacterName)
{
	if (!IsConnected())
		return;
	switch (TravelOptions)
	{
	case TravelOptions::NONE:CharacterName=="" ? Hub->Send(TEXT("Travel"), MapName, bIsDungeon, InstanceID) : Hub->Send(TEXT("Travel"), MapName, bIsDungeon, InstanceID, CharacterName);
		break;
	case TravelOptions::TAG:CharacterName == "" ? Hub->Send(TEXT("TravelWithTag"), MapName, bIsDungeon, InstanceID, Tag): Hub->Send(TEXT("TravelWithTag"), MapName, bIsDungeon, InstanceID, Tag, CharacterName);
		break;
	case TravelOptions::COORDINATES:CharacterName == "" ? Hub->Send(TEXT("TravelWithCoordinates"), MapName, bIsDungeon, InstanceID, Location.X, Location.Y, Location.Z, Yaw ): Hub->Send(TEXT("TravelWithCoordinates"), MapName, bIsDungeon, InstanceID, Location.X, Location.Y, Location.Z, Yaw, CharacterName);
		break;
	default:
		break;
	}
	
	if (bIsDungeon)
	{
		UE_LOG(LogDSSLite, Display, TEXT("Dungeon Traveling to Map %s, InstanceID %s."), *MapName, *InstanceID);
	}
	else
	{
		UE_LOG(LogDSSLite, Display, TEXT("Traveling to Map %s."), *MapName);
	}
}

bool UDSSLiteSubsystem::IsConnected() const
{
	if (Hub == nullptr)
		return false;
	else
		return Hub->IsConnected();
}

void UDSSLiteSubsystem::Disconnect()
{
	if (!IsConnected()) 
	{
		OnDisconnected.Broadcast();//already disconnected
		return;
	}
	Hub->Stop();
}

void UDSSLiteSubsystem::Disconnected()
{
	OnDisconnected.Broadcast();
	if(GetGameInstance()->IsDedicatedServerInstance())
		FGenericPlatformMisc::RequestExit(false);
	UE_LOG(LogDSSLite, Warning, TEXT("Disconnected from DSSLite Server."));
}

void UDSSLiteSubsystem::ConnectionError(const FString& ErrorMSG)
{
	OnConnectionError.Broadcast(ErrorMSG);
	UE_LOG(LogDSSLite, Error, TEXT("Connection Error %s."), *ErrorMSG);
}
void UDSSLiteSubsystem::Connected()
{
	Hub->OnClosed().AddUObject(this, &UDSSLiteSubsystem::Disconnected);

	if (GetGameInstance()->IsDedicatedServerInstance())
	{
		Hub->On(TEXT("ServerClose")).BindLambda([&](const TArray<FSignalRValue>& Arguments)
			{
				ServerTerminate.Broadcast();
				if(bAutoServerClose)
				{
					Disconnect();
					UE_LOG(LogDSSLite, Display, TEXT("Quetting Server at port %d"),GetServerPort());
					FGenericPlatformMisc::RequestExit(false);
				}
			});

		Hub->On(TEXT("OnServerClientTravel")).BindLambda([&](const TArray<FSignalRValue>& Arguments)
			{
				TravelOptions Options = (TravelOptions)Arguments[5].AsInt();
				FString UrlOptions;
				switch (Options)
				{
				case TravelOptions::NONE:
					OnServerTravel.Broadcast(Arguments[0].AsString(), Arguments[1].AsString(), Arguments[2].AsInt(), Arguments[3].AsString(), Arguments[4].AsString(), Options, "", FVector(0.f, 0.f, 0.f), -1.f);
					break;
				case TravelOptions::TAG:
					OnServerTravel.Broadcast(Arguments[0].AsString(), Arguments[1].AsString(), Arguments[2].AsInt(), Arguments[3].AsString(), Arguments[4].AsString(), Options, Arguments[6].AsString(), FVector(0.f, 0.f, 0.f), -1.f);
					break;
				case TravelOptions::COORDINATES:
					OnServerTravel.Broadcast(Arguments[0].AsString(), Arguments[1].AsString(), Arguments[2].AsInt(), Arguments[3].AsString(), Arguments[4].AsString(), Options, "", FVector(Arguments[6].AsFloat(), Arguments[7].AsFloat(), Arguments[8].AsFloat()), Arguments[9].AsFloat());UrlOptions = FString::Printf(TEXT("?mode=2?Location=X=%f,Y=%f,Z=%f?Rotation=%f"), Arguments[6].AsFloat(), Arguments[7].AsFloat(), Arguments[8].AsFloat(), Arguments[9].AsFloat());
					break;
				default:
					break;
				}
			});
		
	}
	else
	{
		Hub->On(TEXT("ClientTravel")).BindLambda([&](const TArray<FSignalRValue>& Arguments)
			{
				ShowLoadingScreen.Broadcast();
				TravelOptions Options = (TravelOptions)Arguments[4].AsInt();
				FString UrlOptions;
				switch (Options)
				{
				case TravelOptions::NONE:
					OnTravel.Broadcast(Arguments[0].AsString(), Arguments[1].AsInt(), Arguments[2].AsString(), Arguments[3].AsString(), Options,"", FVector(0.f, 0.f, 0.f),-1.f);
					UrlOptions = "?mode=0";
					break;
				case TravelOptions::TAG:
					OnTravel.Broadcast(Arguments[0].AsString(), Arguments[1].AsInt(), Arguments[2].AsString(), Arguments[3].AsString(),Options, Arguments[5].AsString(), FVector(0.f, 0.f, 0.f), -1.f);
					UrlOptions = "?mode=1#"+ Arguments[5].AsString();
					break;
				case TravelOptions::COORDINATES:
					OnTravel.Broadcast(Arguments[0].AsString(), Arguments[1].AsInt(), Arguments[2].AsString(), Arguments[3].AsString(), Options,"",FVector(Arguments[5].AsFloat(), Arguments[6].AsFloat(), Arguments[7].AsFloat()), Arguments[8].AsFloat());
					UrlOptions = FString::Printf(TEXT("?mode=2?Location=X=%f,Y=%f,Z=%f?Rotation=%f"), Arguments[5].AsFloat(), Arguments[6].AsFloat(), Arguments[7].AsFloat(), Arguments[8].AsFloat());
					break;
				default:
					break;
				}
				
				if(bAutoClientTravel)
				{

					const FString Command = FString::Printf(TEXT("Travel %s:%d?PlayerName=%s%s"), *Arguments[0].AsString(), Arguments[1].AsInt(), *Arguments[2].AsString(), *UrlOptions);
					UE_LOG(LogDSSLite, Display, TEXT("Traveling to %s:%d, Character %s, Command %s"), *Arguments[0].AsString(), Arguments[1].AsInt(), *Arguments[2].AsString(), *Command);
					APlayerController* TargetPC= GetGameInstance()->GetWorld()->GetFirstPlayerController();
					if (TargetPC != nullptr) 
					{
						TargetPC->ConsoleCommand(*Command, true);
					}
					else
					{
						GEngine->Exec(GetGameInstance()->GetWorld(), *Command);
					}
				}
			});
	}
}

void UDSSLiteSubsystem::OnPlayerConnect(AGameModeBase* GameModeBase, APlayerController* PlayerController)
{
	FString PlayerName = PlayerController->PlayerState->GetPlayerName();
	Hub->Send(TEXT("PlayerEvent"), PlayerName, (int)PlayerEvent::CONNECTED, GameModeBase->GetNumPlayers());
}

void UDSSLiteSubsystem::OnPlayerLogout(AGameModeBase* GameModeBase, AController* PlayerController)
{
	FString PlayerName = PlayerController->PlayerState->GetPlayerName();
	Hub->Send(TEXT("PlayerEvent"), PlayerName, (int)PlayerEvent::DISCONNECTED, GameModeBase->GetNumPlayers()-1);
}
