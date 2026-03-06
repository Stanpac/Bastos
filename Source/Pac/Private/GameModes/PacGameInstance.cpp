// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/PacGameInstance.h"

#include "Character/PacPlayerCharacter.h"
#include "GameModes/PacGameMode.h"
#include "Kismet/GameplayStatics.h"

UPacGameInstance::UPacGameInstance(const FObjectInitializer& ObjectInitializer)
{
	
}

void UPacGameInstance::SwitchToLobby()
{
	if (GetWorld()) {
		APacGameMode* PacGM = Cast<APacGameMode>(GetWorld()->GetAuthGameMode());
		if (PacGM && PacGM->GetCurrentPlayerCharacter()) {
			SaveBullets = PacGM->GetCurrentPlayerCharacter()->GetAllBullets();
		}
	}
	
	if (LobbyLevel.LoadSynchronous()) {
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, LobbyLevel);
	}
}

void UPacGameInstance::OpenNextLevel()
{
	if (LevelsOrder.Num() == 0) {
		UE_LOG(LogTemp, Warning, TEXT("No Levels in LevelsOrder"));
		return;
	}

	if (GetWorld()) {
		APacGameMode* PacGM = Cast<APacGameMode>(GetWorld()->GetAuthGameMode());
		if (PacGM && PacGM->GetCurrentPlayerCharacter()) {
			SaveBullets = PacGM->GetCurrentPlayerCharacter()->GetAllBullets();
		}
	}
	 
	if (CurrentLevelIndex >= LevelsOrder.Num()) {
		CurrentLevelIndex = 0;
	}

	TSoftObjectPtr<UWorld> NextWorld = LevelsOrder[CurrentLevelIndex];

	if (NextWorld.LoadSynchronous()) {
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, NextWorld);
	}
	
	CurrentLevelIndex++;
}

void UPacGameInstance::OpenMenuLevel()
{
	if (MainMenuLevel.LoadSynchronous()) {
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, MainMenuLevel);
	}
}
