// Copyright Epic Games, Inc. All Rights Reserved.

#include "TomHawkTestGameMode.h"
#include "TomHawkTestCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATomHawkTestGameMode::ATomHawkTestGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
