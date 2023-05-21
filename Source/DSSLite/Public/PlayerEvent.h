// Copyright (c) 2022 Dynamic Servers Systems

#pragma once
#include "Engine/Engine.h"
#include"PlayerEvent.generated.h"

UENUM(BlueprintType)
enum class PlayerEvent : uint8 {
	CONNECTED = 0 UMETA(DisplayName = "Connected"),
	DISCONNECTED = 1  UMETA(DisplayName = "Disconnected")
};