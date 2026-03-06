// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/PacGameMode.h"

#include "Character/PacPlayerCharacter.h"
#include "Effect/PacEffectComponent.h"
#include "GameModes/PacWorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Utility/PacLogMacro.h"

APacGameMode::APacGameMode(const FObjectInitializer& ObjectInitializer)
{
	// Initialize the current player character to nullptr
	CurrentPlayerCharacter = nullptr;

	// Initialize the enemies array
	Enemies.Empty();

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void APacGameMode::SetCurrentPlayerCharacter(APacPlayerCharacter* NewPlayerCharacter)
{
	if (NewPlayerCharacter) {
		CurrentPlayerCharacter = NewPlayerCharacter;
	} else {
		PAC_WARNING_CATEGORY(PacLog, "Attempted to set CurrentPlayerCharacter to nullptr");
	}
}

void APacGameMode::TryAddEnemy(APacCharacter* NewEnemy)
{
	if (NewEnemy && !Enemies.Contains(NewEnemy)) {
		Enemies.Add(NewEnemy);
		OnEnemyUpdate.Broadcast(NewEnemy, Enemies.Num());
		PAC_LOG_CATEGORY(PacLog, "Added new enemy: %s to list in Gamemode", *NewEnemy->GetName());
	} else {
		PAC_WARNING_CATEGORY(PacLog, "Attempted to add a nullptr enemy or an enemy already in the list");
	}
}

void APacGameMode::RemoveEnemy(APacCharacter* EnemyToRemove)
{
	if (EnemyToRemove && Enemies.Contains(EnemyToRemove)) {
		Enemies.Remove(EnemyToRemove);
		OnEnemyUpdate.Broadcast(EnemyToRemove, Enemies.Num());
		PAC_LOG_CATEGORY(PacLog, "Removed enemy: %s from list in Gamemode", *EnemyToRemove->GetName());
	} else {
		PAC_WARNING_CATEGORY(PacLog, "Attempted to remove a nullptr enemy or an enemy not in the list");
	}

	if (Enemies.Num() == 0) {
		if (SpawnerInLevel.Num() > 0) {
			return;
		}

		PlayerWinLevel();
	}
}

void APacGameMode::RegisterSpawner(APacSpawnerBase* Spawner)
{
	if (Spawner) {
		SpawnerInLevel.AddUnique(Spawner);
	}
}

void APacGameMode::UnregisterSpawner(APacSpawnerBase* Spawner)
{
	if (Spawner && SpawnerInLevel.Contains(Spawner)) {
		SpawnerInLevel.Remove(Spawner);
	}
}

void APacGameMode::PlayerStartDeath(APacCharacter* Character)
{
	for (APacCharacter* Enemy : Enemies) {
		if (Enemy) {
			Enemy->K2_Victory();
		}
	}

	Enemies.Empty();
	SpawnerInLevel.Empty();
}

void APacGameMode::StartLevelTimer()
{
	CurrentLevelTimer = 0.0f;
	bLevelTimer = true;
}

void APacGameMode::PlayerWinLevel()
{
	GetCurrentPlayerCharacter()->GetEffectComponent()->ClearAllEffects();
	bLevelTimer = false;

	OnPlayerWinLevel.Broadcast();
	
	Enemies.Empty();
	SpawnerInLevel.Empty();
}

void APacGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (bLevelTimer) {
		CurrentLevelTimer += DeltaSeconds;
		TimerUpdateDelegate.Broadcast(CurrentLevelTimer);
	}
}

void APacGameMode::BeginPlay()
{
	Super::BeginPlay();
	APacWorldSettings* WorldSetting =  Cast<APacWorldSettings>(GetWorld()->GetWorldSettings());
	if (WorldSetting) {
		if (WorldSetting->bTimerEnabled) {
			StartLevelTimer();
		}

		if (WorldSetting->BackgroundMusic) {
			UGameplayStatics::PlaySound2D(this, WorldSetting->BackgroundMusic, 1.0f, 1.0f, 0.0f, nullptr, this, false);
		}
	} 
}


