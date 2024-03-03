// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TomHawkTestHelpers.generated.h"

/**
 * A collection of utility methods for working on this project in a simpler way.
 */
UCLASS()
class TOMHAWKTEST_API UTomHawkTestHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Retrieve the forward socket name. */
	UFUNCTION(BlueprintPure, Category = Test)
	static FName GetSkateboardForwardSocketName();

	/** Retrieve the backward socket name. */
	UFUNCTION(BlueprintPure, Category = Test)
	static FName GetSkateboardBackwardSocketName();
};
