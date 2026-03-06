// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PacBulletsComponent.generated.h"

class APacWeapon;
class UPacAbility;
class UBulletDefinition;
class UPacAbilitySystemComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBulletActivate, TSubclassOf<UBulletsBase>, BulletClass);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNewBulletAdd,  TSubclassOf<UBulletsBase>, BulletClass, int32, BulletIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBulletHit, const FHitResult&, HitResult,  TSubclassOf<UBulletsBase>, BulletClass);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnBulletCycle, TSubclassOf<UBulletsBase>, BulletClass, int32, OldIndex, int32, NewIndex, int32, MaxBullets);


/* 
 *  Manage the Bullet of a Character
 *  Without this a Character will not be able to use Bullet Ability !
 *  Pawn need to Have a ASC for Use Ability !
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PAC_API UPacBulletsComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class UBulletsBase;
public:
	UPacBulletsComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Cycle through the available bullets (+1 to ActiveBulletIndex)
	UFUNCTION(BlueprintCallable, Category="Pac")
	void CycleActiveBullet();

	// Set the active bullet index to a specific value
	UFUNCTION(BlueprintCallable, Category="pac")
	void SetActiveBulletIndex(int32 NewIndex);

	// Add a bullet at a specific index
	UFUNCTION(BlueprintCallable)
	void AddBulletAt(int32 BulletIndex, TSubclassOf<UBulletsBase> BulletClass);

	// Initialize the bullets with a specific bullet definition (use for default or initial bullets)
	UFUNCTION(BlueprintCallable, Category="Pac|Bullets")
	void InitializeBullets(TSubclassOf<UBulletsBase> BulletClass);
	
	UFUNCTION(BlueprintCallable, Category="Pac|Bullets")
	void ActivateCurrentBullet();
	
	// Override All Bullets with a new set of bullets 
	UFUNCTION(BlueprintCallable, Category="Pac|Bullets")
	void SetNewBullets(TArray<TSubclassOf<UBulletsBase>> InBulletsClasses);
	
	// Get the currently active Bullet Class
	UFUNCTION(BlueprintCallable, Category="Pac|Bullets")
	TSubclassOf<UBulletsBase> GetActiveBullet() const;
	
	// Get the currently active bullet index
	UFUNCTION(BlueprintCallable)
	int32 GetActiveBulletIndex() const { return ActiveBulletIndex; }

	// Get the number of bullets available in the component
	UFUNCTION(BlueprintCallable)
	int32 GetNumBullets() const { return NumBullets; }
	
	// Get the list of all bullets currently available in the component
	UFUNCTION(BlueprintCallable, Category="Pac|Bullets")
	TArray<TSubclassOf<UBulletsBase>> GetBullets() const;

	// Check if the component has any bullets available
	UFUNCTION(BlueprintCallable, Category="Pac|Bullets")
	bool HasBullets() const { return !Bullets.IsEmpty(); }
	
	// Fire When the active bullet is changed 
	UPROPERTY(BlueprintAssignable, Category="Pac|Bullets")
	FOnBulletCycle OnBulletCycle;

	// Fire When a new bullet is added to Bullets 
	UPROPERTY(BlueprintAssignable, Category="Pac|Bullets")
	FOnNewBulletAdd OnNewBulletAdd;

	// Fire When a bullet is activated
	UPROPERTY(BlueprintAssignable, Category="Pac|Bullets")
	FOnBulletActivate OnBulletActivate;

	// Fire when a bullet hits something
	UPROPERTY(BlueprintAssignable, Category="Pac|Bullets")
	FOnBulletHit OnBulletHit;

protected:
	// The Default bullet To have at Start In all Slots 
	UPROPERTY(EditAnywhere, Category="Pac|Bullets")
	TSubclassOf<UBulletsBase> DefaultBulletClass;

	// Array of All Cuurent bullet (Can be Edit Before Start for Precise build)
	UPROPERTY(EditAnywhere, Category="Pac|Bullets")
	TArray<TSubclassOf<UBulletsBase>> Bullets;
	
	// The number of slots available for bullets ! 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Pac|Bullets")
	int32 NumBullets = 4;

	UPROPERTY(VisibleAnywhere, Category="Pac|Bullets")
	int32 ActiveBulletIndex = 0;
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	TObjectPtr<APacWeapon> SpawnedWeaponActor;

	UPROPERTY(Transient)
	TArray<UBulletsBase*> ActivatedBulletsInstances;
};
