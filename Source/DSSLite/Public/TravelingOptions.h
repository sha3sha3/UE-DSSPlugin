// Copyright (c) 2022 Dynamic Servers Systems

#pragma once
#include "Engine/Engine.h"
#include"TravelingOptions.generated.h"

UENUM(BlueprintType)
enum class TravelOptions : uint8 {
	NONE = 0 UMETA(DisplayName = "NONE"),
	TAG = 1  UMETA(DisplayName = "TAG"),
	COORDINATES = 2     UMETA(DisplayName = "COORDINATES")
};