// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "PacWorldSettings.generated.h"

class USoundCue;

/**
 *  Base World Settings for the Pac project.
 */
UCLASS()
class PAC_API APacWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	APacWorldSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pac")
	bool bTimerEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pac")
	bool IsLobby = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pac")
	USoundCue* BackgroundMusic = nullptr;
};
