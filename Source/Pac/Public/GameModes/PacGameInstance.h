// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PacGameInstance.generated.h"

class UBulletsBase;

/**
 *  Base GameInstance for the Pac project.
 */
UCLASS()
class PAC_API UPacGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPacGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Pac|Levels")
	void SwitchToLobby();

	UFUNCTION(BlueprintCallable, Category = "Pac|Levels")
	void OpenNextLevel();

	UFUNCTION(BlueprintCallable, Category = "Pac|Levels")
	void OpenMenuLevel();

	UFUNCTION(BlueprintCallable, Category = "Pac|Levels")
	TArray<TSubclassOf<UBulletsBase>>  GetSaveBullets() const { return SaveBullets; }

protected:
	UPROPERTY(EditAnywhere, Category = "Pac|Levels")
	TArray<TSoftObjectPtr<UWorld>> LevelsOrder;

	UPROPERTY(EditAnywhere, Category = "Pac|Levels")
	TSoftObjectPtr<UWorld> MainMenuLevel;

	UPROPERTY(EditAnywhere, Category = "Pac|Levels")
	TSoftObjectPtr<UWorld> LobbyLevel;
	
	int CurrentLevelIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pac|Levels")
	TArray<TSubclassOf<UBulletsBase>> SaveBullets;
};
