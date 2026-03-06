// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"


class UPacAbilitySystemComponent;
class UBoxComponent;

UENUM(BlueprintType)
enum class EProjectileLifeTimeType : uint8
{
	DestroyOnHit,
	DestroyAfterTime,
	CustomDestroy // Need to be Destroyed Manually
};

DECLARE_MULTICAST_DELEGATE(FOnProjectileEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnProjectileHit, AActor*);

/**
 *  Base Class for all Projectiles
 */
UCLASS(Blueprintable, BlueprintType)
class PAC_API AProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AProjectileBase(FObjectInitializer const& ObjectInitializer = FObjectInitializer::Get());
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pac|Components")
	TObjectPtr<UBoxComponent> CollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pac")
	EProjectileLifeTimeType LifeTimeType;

	// Time before the Projectile is Destroyed, Use as SafeGuard if the Projectile is not Destroyed by other means
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pac", meta = (EditCondition = "LifeTimeType != EProjectileLifeTimeType::CustomDestroy", EditConditionHides))
	float LifeTime = 5.0f;

	// Called when the Projectile StartDestruction
	FOnProjectileEvent OnProjectileDestroyed;
	
	// Called when the Projectile is Spawned
	FOnProjectileEvent OnProjectileSpawned;

	// Called when the Projectile is Spawned
	FOnProjectileHit OnProjectileHit;
	
	/*------------- Callbacks for BP -------------*/
	// Called when the Projectile StartDestruction
	UFUNCTION(BlueprintImplementableEvent, Category = "Pac")
	void OnProjectileDestroyedEvent();
	
	// Called when the Projectile is Spawned
	UFUNCTION(BlueprintImplementableEvent, Category = "Pac")
	void OnProjectileSpawnedEvent();
	
	/*------------- End Callbacks for BP -------------*/

protected:	
	UFUNCTION(BlueprintCallable, Category = "Pac|Projectile")
	void ProjectileHit(AActor* OtherActor);
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
