// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PacGameMode.generated.h"

class APacSpawnerBase;
class APacCharacter;
class APacPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyUpdate, APacCharacter*, Enemy, int, Count);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimerUpdate, float, CurrentTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerWinLevelEvent);

/**
 *  Base Gamemode for the Pac project.
 */
UCLASS(Blueprintable, BlueprintType)
class PAC_API APacGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	APacGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Pac|GameMode")
	void SetCurrentPlayerCharacter(APacPlayerCharacter* NewPlayerCharacter);

	UFUNCTION(BlueprintCallable, Category = "Pac|GameMode")
	void TryAddEnemy(APacCharacter* NewEnemy);

	UFUNCTION(BlueprintCallable, Category = "Pac|GameMode")
	void RemoveEnemy(APacCharacter* EnemyToRemove);

	UPROPERTY(BlueprintAssignable, Category = "Pac|GameMode")
	FOnEnemyUpdate OnEnemyUpdate;

	UFUNCTION(BlueprintCallable, Category = "Pac|GameMode")
	TArray<APacCharacter*> GetEnemies() const { return Enemies; }

	UFUNCTION(BlueprintCallable, Category = "Pac|GameMode")
	APacPlayerCharacter* GetCurrentPlayerCharacter() const { return CurrentPlayerCharacter; }

	UFUNCTION(BlueprintCallable, Category = "Pac|GameMode")
	void RegisterSpawner(APacSpawnerBase* Spawner);

	UFUNCTION(BlueprintCallable, Category = "Pac|GameMode")
	void UnregisterSpawner(APacSpawnerBase* Spawner);

	UFUNCTION(BlueprintCallable, Category = "Pac|GameMode")
	void PlayerStartDeath(APacCharacter* Character);

	UFUNCTION(BlueprintCallable, Category = "Pac|GameMode")
	void StartLevelTimer();

	UFUNCTION(BlueprintCallable, Category = "Pac|GameMode")
	void PlayerWinLevel();

	UPROPERTY(BlueprintAssignable, Category = "Pac|GameMode")
	FOnTimerUpdate TimerUpdateDelegate;

 	UPROPERTY(BlueprintAssignable, Category = "Pac|GameMode")
	FPlayerWinLevelEvent OnPlayerWinLevel;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pac")
	float CurrentLevelTimer = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pac")
	bool bLevelTimer = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pac")
	TObjectPtr<APacPlayerCharacter> CurrentPlayerCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pac")
	TArray<APacCharacter*> Enemies;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pac")
	TArray<APacSpawnerBase*> SpawnerInLevel;

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
};
