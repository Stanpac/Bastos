// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PacSpawnerBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSpawnerEvent);
 
UCLASS(Abstract, Blueprintable, BlueprintType)
class PAC_API APacSpawnerBase : public AActor
{
	GENERATED_BODY()
	
public:
	APacSpawnerBase();

	UPROPERTY(BlueprintAssignable)
	FSpawnerEvent OnSpawningCompleteEvent;
	
protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Pac|SpawnPosition")
	bool bUseActorTransform = true;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Pac|SpawnPosition", meta = (EditCondition = "!bUseActorTransform", EditConditionHides, MakeEditWidget = true))
	TArray<FVector> SpawnPositions;

	// Do we want to have a random delay between each spawn ? 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pac|Delay")
	bool bUseVarianceDelay = true;

	// Delay between each spawn, if bUseVarianceDelay is false, this will be the only delay used
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pac|Delay")
	float SpawnDelay = 1.0f;

	// Variance of the spawn delay, Will Choose between SpawnDelay - SpawnDelayVariance and SpawnDelay + SpawnDelayVariance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pac|Delay", meta = (EditCondition = "bUseVarianceDelay", EditConditionHides))
	float SpawnDelayVariance = 0.5f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pac|Delay", meta = (EditCondition = "bUseVarianceDelay", EditConditionHides))
	FVector2D SpawnMinMaxDelay = FVector2D(1.0f, 10.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pac")
	bool bStartAtBeginPlay = false;

	// Maximum number of actors to spawn before stopping the spawner, if < 0, the spawner will never stop
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pac|Spawn")
	int32 MaxSpawnCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pac|Spawn")
	bool bDestroyOnSpawningComplete = true;

	FTimerHandle SpawnTimerHandle;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Pac|Spawn")
	int32 CurrentSpawnCount = 0;
	\
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Pac|Spawn")
	float TimeBeforeNextSpawn = 0.0f;


	bool bIsActivated = false;

	void ActivateSpawner(); 

	// Get the delay before the next spawn, if bUseVarianceDelay is true, it will return a random value between SpawnMinMax
	virtual float GetSpawnDelay();

	// Get the transform to spawn the actor at, if bUseActorTransform is true, it will return the actor's transform
	virtual FVector GetSpawnPosition() const;

	// Get the actors to spawn, Override this function in derived classes to return the actors you want to spawn
	virtual TArray<TSubclassOf<AActor>> GetActorsToSpawn() { return TArray<TSubclassOf<AActor>>(); }

	// Spawner Logic
	UFUNCTION(BlueprintCallable)
	virtual void SpawnActor();
	
	// Call to check if we can continue spawning actors ( called Just After Spawn Actors, before setting the timer for the next spawn)
	virtual bool DoesSpawningProcessContinue();
	
	// Call when the spawning process is complete ( max spawn count is reached)
	virtual void OnSpawningComplete();
	
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};
