// Fill out your copyright notice in the Description page of Project Settings.


#include "Spawner/PacCharacterSpawner.h"

#include "Character/PacCharacter.h"
#include "GameModes/PacGameMode.h"
#include "Utility/PacLogMacro.h"

APacCharacterSpawner::APacCharacterSpawner(FObjectInitializer const& ObjectInitializer)
{
}

void APacCharacterSpawner::OnEnemyUpdate(APacCharacter* Enemy, int Count)
{
	if (Count > ActivationEnemyCountTreshold || bIsActivated) {
		return;
	}
	
	ActivateSpawner();
}

void APacCharacterSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bStartAtBeginPlay) {
		return;
	}
	
	if (APacGameMode* PacGameMode = GetWorld()->GetAuthGameMode<APacGameMode>()) {
		if (PacGameMode->GetEnemies().Num() <= 0) {	
			return;
		}
		
		PacGameMode->OnEnemyUpdate.AddDynamic(this, &APacCharacterSpawner::OnEnemyUpdate);
	}
}

TArray<TSubclassOf<AActor>> APacCharacterSpawner::GetActorsToSpawn()
{
	TArray<TSubclassOf<AActor>> Result;

	if (CharacterWaves.Num() == 0) {
		return Result;
	}

	if (CurrentWaveIndex >= CharacterWaves.Num()) {
		if (bStopWhenWavesCompleted) {
			return Result;
		}
		CurrentWaveIndex = 0;
	}

	const FPacCharacterSpawnWaveData& CurrentWave = CharacterWaves[CurrentWaveIndex];
	if (!CurrentWave.CharacterClass) {
		PAC_ERROR(" Invalid Character Class in Wave %d in %s", CurrentWaveIndex, *GetNameSafe(this));
		return Result;
	}

	int32 CountToSpawn = CurrentWave.Count;
	if (CurrentWave.bUseVarianceCount) {
		CountToSpawn = FMath::RandRange(CurrentWave.CountMinMax.X, CurrentWave.CountMinMax.Y);
	}

	for (int32 i = 0; i < CountToSpawn; i++) {
		Result.Add(CurrentWave.CharacterClass);
	}

	CurrentWaveIndex++;

	return Result;
}

bool APacCharacterSpawner::DoesSpawningProcessContinue()
{
	if (!Super::DoesSpawningProcessContinue()) {
		return  false;
	}

	if (bStopWhenWavesCompleted && CurrentWaveIndex >= CharacterWaves.Num()) {
		return false;
	}

	return true;
}

#if WITH_EDITOR
void APacCharacterSpawner::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	// Update CountMinMax if Count or CountSpawnVariance is changed
	if (PropertyChangedEvent.Property && (
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(FPacCharacterSpawnWaveData, CountSpawnVariance) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(FPacCharacterSpawnWaveData, Count) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(FPacCharacterSpawnWaveData, bUseVarianceCount))) {
		for (FPacCharacterSpawnWaveData& WaveData : CharacterWaves) {
			if (WaveData.bUseVarianceCount) {
				WaveData.CountMinMax = FVector2D(FMath::Max(0, WaveData.Count - WaveData.CountSpawnVariance), WaveData.Count + WaveData.CountSpawnVariance);
			} else {
				WaveData.CountMinMax = FVector2D(0, 0);
			}
		}
	}
}
#endif


