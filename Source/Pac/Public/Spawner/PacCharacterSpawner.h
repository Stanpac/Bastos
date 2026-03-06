// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PacSpawnerBase.h"
#include "PacCharacterSpawner.generated.h"

class APacGameMode;
class APacCharacter;


USTRUCT(BlueprintType)
struct FPacCharacterSpawnWaveData
{
	GENERATED_BODY()

	// Class of the character to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<APacCharacter> CharacterClass;

	// Number of characters to spawn of this class
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseVarianceCount = false;

	// Variance of the number of characters to spawn, will spawn between Count - CountSpawnVariance and Count + CountSpawnVariance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bUseVarianceCount", EditConditionHides))
	int32 CountSpawnVariance = 0;

	// Min and Max number of characters to spawn, updated automatically when Count or CountSpawnVariance is changed
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (EditCondition = "bUseVarianceCount", EditConditionHides))
	FVector2D CountMinMax = FVector2D(0, 0);

	FPacCharacterSpawnWaveData()
		: CharacterClass(nullptr), Count(1)
	{}
};

UCLASS(Blueprintable, BlueprintType)
class PAC_API APacCharacterSpawner : public APacSpawnerBase
{
	GENERATED_BODY()
public:
	APacCharacterSpawner(FObjectInitializer const& ObjectInitializer = FObjectInitializer::Get());

	/** If true, The Spawner Will Stop when all waves are completed !
	 *	If false, the Spawner will restart from the first wave when all waves are completed
	 *  The Spawner Will Stop if it's reach the MaxSpawnCount regardless of this setting
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pac|Waves")
	bool bStopWhenWavesCompleted = true;
	
	// Waves of characters to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pac|Waves")
	TArray<FPacCharacterSpawnWaveData> CharacterWaves;

	// When the number of enemies in the world is below this threshold, the Spawner will be activated
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pac", meta = (EditCondition = "!bStartAtBeginPlay", EditConditionHides, DisplayAfter = "bStartAtBeginPlay", ClampMin = "0", UIMin = "0"))
	int32 ActivationEnemyCountTreshold = 10;

protected:
	UFUNCTION()
	void OnEnemyUpdate(APacCharacter* Enemy, int Count);
	
	virtual void BeginPlay() override;
	virtual TArray<TSubclassOf<AActor>> GetActorsToSpawn() override;

	virtual bool DoesSpawningProcessContinue() override;
	
	int32 CurrentWaveIndex = 0;

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};
