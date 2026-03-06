// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BulletsBase.generated.h"

class APacCharacter;
class UPacBulletsComponent;
class UBulletDefinition;

UENUM(BlueprintType)
enum class EPacBulletTargetingSource : uint8
{
	// From the player's camera towards camera focus
	CameraTowardsFocus,
	// From the pawn's center, in the pawn's orientation
	PawnForward,
	// From the weapon's socket
	WeaponSocket,
	// From feedBackSocket from the PacCharacter
	FeedBackSocket,
};

/**
 *  Base Class For Bullets 
 */
UCLASS(BlueprintType, Blueprintable, PrioritizeCategories = ("Pac", "Gameplay", "Appearance"))
class PAC_API UBulletsBase : public UObject
{
	GENERATED_BODY()
public:
	UBulletsBase(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	FText BulletName;
	 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	FText BulletDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	FSlateBrush CursorBrush;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	FSlateBrush InventoryBrush;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	UStaticMesh* PickUpMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = SFX)
	USoundBase* FireSoundClue;

	// Call PreActivateAbility And ActivateAbility
	UFUNCTION()
	void CallActivateBullet();

	// Call when the Process of the bullet is Cancelled
	UFUNCTION()
	virtual void CancelBullet();

	// Call from blueprints to forcibly end the Bullet 
	UFUNCTION(BlueprintCallable, Category = Gameplay, DisplayName = "Cancel Bullet", meta=(ScriptName = "CancelAbility"))
	virtual void K2_CancelBullet();

	// Call when the Ability is Ended
	UFUNCTION()
	virtual void EndBullet(bool bWasCancelled);

	// Call from blueprints to forcibly end the ability without canceling it
	UFUNCTION(BlueprintCallable, Category = Gameplay, DisplayName="End Bullet", meta=(ScriptName = "EndAbility"))
	virtual void K2_EndBullet();
	
	void SetBulletsComponent(UPacBulletsComponent* InBulletsComponent) { BulletsComponent = InBulletsComponent; }
	
	UFUNCTION(BlueprintCallable, Category = "Pac|Bullets")
	float GetTraceDistance() const { return TraceDistance; }

	UFUNCTION(BlueprintCallable, Category = "Pac|Bullets")
	float GetSweepRadius() const { return SweepRadius; }
	
#if WITH_EDITOR
	//~UObject overrides
	virtual bool ImplementsGetWorld() const override { return true; }
	//~End UObject overrides
#endif // WITH_EDITOR

	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|BaseBullet")
	EPacBulletTargetingSource TargetingSource = EPacBulletTargetingSource::CameraTowardsFocus;
	
	// Distance of the trace for the shoot
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|BaseBullet")
	float TraceDistance = 10000.0f;

	// Radius of the Trace 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|BaseBullet")
	float SweepRadius = 0.f;

	/** If the Bullet should be automatically ended after the ActivateBullet is called !
	 * @warning  if false the bullet will not be ended automatically, you must call EndBullet manually
	 */ 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|BaseBullet")
	bool AutoEnd = true;

	// Call Just Before the Bullet is Activated
	virtual void PreActivateBullet();

	// Handle the Bullet Proceses
	virtual void ActivateBullet();

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "ActivateBullet", meta=(ScriptName = "ActivateBullet"))
	void K2_ActivateBullet();

	bool bHasBlueprintActivate;
	
	// Blueprint event, will be called if a Bullet ends normally or abnormally
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnEndAbility", meta=(ScriptName = "OnEndAbility"))
	void K2_OnEnBullet(bool bWasCancelled);

	bool bHasBlueprintOnEnd;

	// Start targeting, Return the first hit result
	UFUNCTION(BlueprintCallable)
	FHitResult StartTrageting();

	// Call when the target is Found	
	UFUNCTION(BlueprintImplementableEvent)
	void OnTargetFound(const FHitResult HitResult);

	// Get the transform to use for targeting based on the TargetingSource.
	UFUNCTION(BlueprintCallable, Category = "Pac|Bullets")
	FTransform GetTargetingTransform(ACharacter* SourceCharacter) const;

	int32 FindFirstDamageableHitResult(const TArray<FHitResult>& HitResults);

	// Determine the trace channel to use for the weapon trace(s)
	virtual ECollisionChannel DetermineTraceChannel() const;

	UPacBulletsComponent* BulletsComponent = nullptr;
	
	// Get the Avatar PacCharacter that owns this Bullet
	UFUNCTION(BlueprintCallable)
	APacCharacter* GetAvatarCharacter() const;
};

