// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "PacCheatManager.generated.h"


PAC_API DECLARE_LOG_CATEGORY_EXTERN(PacCheatLog, Log, All); 

/**
 *  PacManager is a custom cheat manager for the Pac project.
 */
UCLASS()
class PAC_API UPacCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	UPacCheatManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Enable player invincibility cheat
	UFUNCTION(Exec, Category = "Pac|Cheat")
	void CheatPlayerInvincibility();
	
};
