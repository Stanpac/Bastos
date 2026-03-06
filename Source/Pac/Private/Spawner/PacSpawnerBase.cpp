// Fill out your copyright notice in the Description page of Project Settings.


#include "Spawner/PacSpawnerBase.h"

#include "GameModes/PacGameMode.h"
#include "Utility/PacLogMacro.h"


APacSpawnerBase::APacSpawnerBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APacSpawnerBase::ActivateSpawner()
{
	bIsActivated = true;
	PAC_LOG(" Activating Spawner : %s", *GetNameSafe(this));
	SpawnActor();
}

float APacSpawnerBase::GetSpawnDelay()
{
	if (bUseVarianceDelay) {
		return FMath::RandRange(SpawnMinMaxDelay.X, SpawnMinMaxDelay.Y);
	}
	return SpawnDelay;
}

FVector APacSpawnerBase::GetSpawnPosition() const
{
	if (bUseActorTransform) {
		return GetActorLocation();
	}

	if (SpawnPositions.Num() > 0) {
		int32 RandomIndex = FMath::RandRange(0, SpawnPositions.Num() - 1);
		return SpawnPositions[RandomIndex];
	}

	return FVector::ZeroVector;
}

void APacSpawnerBase::SpawnActor()
{
	if (MaxSpawnCount == 0) {
		PAC_ERROR(" MaxSpawnCount is set to 0 in %s, cannot spawn any actor", *GetNameSafe(this));
		return;
	}
	
	TArray<TSubclassOf<AActor>> ActorsToSpawn = GetActorsToSpawn();
	if (ActorsToSpawn.IsEmpty()) {
		PAC_LOG(" No Actors to Spawn in %s", *GetNameSafe(this));
		OnSpawningComplete();
		return;
	}

	for (auto ToSpawn : ActorsToSpawn) {
		if (!ToSpawn) {
			PAC_LOG(" Invalid Actor to Spawn in %s", *GetNameSafe(this));
			continue;
		}

		FVector SpawnPos = GetActorLocation() + GetSpawnPosition();
		FActorSpawnParameters SpawnParameters = FActorSpawnParameters();
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ToSpawn, SpawnPos, GetActorRotation(), SpawnParameters);
		if (SpawnedActor) {
			CurrentSpawnCount++;
			PAC_LOG(" Spawned Actor : %s in %s", *GetNameSafe(SpawnedActor), *GetNameSafe(this));
		} else {
			PAC_LOG(" Failed to Spawn Actor in %s", *GetNameSafe(this));
		}

		if (MaxSpawnCount > 0 && CurrentSpawnCount >= MaxSpawnCount) {
			break;
		}
	}

	if (!DoesSpawningProcessContinue()) {
		OnSpawningComplete();
		return;
	}

	PAC_LOG(" Start New Delay for next spawn : %f seconds, in %s", TimeBeforeNextSpawn, *GetNameSafe(this));
	
	// Set timer for next spawn
	TimeBeforeNextSpawn = GetSpawnDelay();
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &APacSpawnerBase::SpawnActor, TimeBeforeNextSpawn, false);
}

bool APacSpawnerBase::DoesSpawningProcessContinue()
{
	if (CurrentSpawnCount >= (MaxSpawnCount < 0 ? INT32_MAX : MaxSpawnCount)) {
		return false;
	}

	return true;
}

void APacSpawnerBase::OnSpawningComplete()
{
	PAC_LOG("Spawning Complete in %s", *GetNameSafe(this));
	OnSpawningCompleteEvent.Broadcast();

	APacGameMode* PacGM = Cast<APacGameMode>(GetWorld()->GetAuthGameMode());
	if (PacGM) {
		PacGM->UnregisterSpawner(this);
	}
	
	if (bDestroyOnSpawningComplete) {
		Destroy();
	}
}

void APacSpawnerBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (bStartAtBeginPlay) {
		ActivateSpawner();
	}

	APacGameMode* PacGM = Cast<APacGameMode>(GetWorld()->GetAuthGameMode());
	if (PacGM) {
		PacGM->RegisterSpawner(this);
	}
}

#if WITH_EDITOR
void APacSpawnerBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property && (
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(APacSpawnerBase, bUseVarianceDelay) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(APacSpawnerBase, SpawnDelay) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(APacSpawnerBase, SpawnDelayVariance))) {
		if (bUseVarianceDelay){
			SpawnMinMaxDelay = FVector2D(SpawnDelay - SpawnDelayVariance, SpawnDelay + SpawnDelayVariance);
		} 
	}

	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(APacSpawnerBase, bUseActorTransform)){
		if (!bUseActorTransform){
			SpawnPositions.Empty();
		}
	}
}
#endif // WITH_EDITOR
