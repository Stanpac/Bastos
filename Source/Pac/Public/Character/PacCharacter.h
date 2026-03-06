// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/Character.h"
#include "Utility/GameplayTagCountContainerInterface.h"
#include "Utility/PacGameplayTags.h"
#include "PacCharacter.generated.h"

class UPacEffectComponent;
class UPacBulletsComponent;
class UPacHealthComponent;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPacCharacterEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPacCharacterReloadEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPacCharacterShootEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPacCharacterSprintEvent);


UENUM(BlueprintType)
enum class EFeedBackShootingSource : uint8
{
	// From the player's camera towards camera focus
	CameraTowardsFocus,
	// From the pawn's center, in the pawn's orientation
	PawnForward,
	// From the weapon's socket (Is same as PawnSocket Because Weapon no longer Exist
	WeaponSocket,
	// From the pawn's socket
	PawnSocket,
};

UENUM()
enum class EPacCharacterType : uint8
{
	AI,
	Player,
};

/** 
 *  Base Class for all Characters in the Pac project.
 */
UCLASS(PrioritizeCategories = "Pac")
class PAC_API APacCharacter : public ACharacter, public IGameplayTagAssetInterface, public IGameplayTagCountContainerInterface
{
	GENERATED_BODY()
public:
	APacCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~IGameplayTagAssetInterface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	//~End IGameplayTagAssetInterface

	//~IGameplayTagCountContainerInterface
	virtual void UpdateTagsCount(const FGameplayTagContainer Container, int32 CountDelta) override;
	virtual void UpdateTagCount(const FGameplayTag Tag, int32 CountDelta) override;
	virtual bool SetTagCount(const FGameplayTag Tag, int32 NewCount) override;
	virtual int32 GetTagCount(const FGameplayTag Tag) const override;
	virtual void AddTemporaryTag(const FGameplayTag Tag, float Duration, int Count = 1) override;
	virtual void RemoveTemporaryTag(const FGameplayTag Tag, float Duration, int Count = 1) override;
	//~End IGameplayTagCountContainerInterface
	
	/** Called for movement input */
	virtual void Move(const FInputActionValue& Value);

	/** Called for looking input */
	virtual void Look(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	FVector GetFacingDirection() const;

	/* ---------------------------------- Jumping ---------------------------------- */
	
	/** Called for Jumping input */
	virtual void Jump() override;
	virtual void StopJumping() override;
	
	UFUNCTION(BlueprintCallable)
	virtual void CheckJumpInput(float DeltaTime) override;

	// Called when the character has jumped */
	virtual void OnJumped_Implementation() override;

	/** Called when the character has jumped */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Pac|Character")
	FPacCharacterEvent OnJumpedEvent;
	
	/* ---------------------------------- Shooting ---------------------------------- */
	
	/** Called for Shooting input */
	UFUNCTION(BlueprintCallable)
	virtual void Fire();

	UFUNCTION(Blueprintable)
	virtual bool CanFire() const;
	
	// Fire When the Character is shooting
	UFUNCTION(BlueprintImplementableEvent, Category="Pac|Character", DisplayName="Fire", meta=(ScriptName = "Fire"))
	void K2_Fire();

	// Fire When the Character is shooting
	UPROPERTY(BlueprintAssignable, Category="Pac|Character")
	FPacCharacterShootEvent OnShoot;

	/* ---------------------------------- Reload ---------------------------------- */
	
	/** Called for Reloading input */
	virtual void Reload();
	virtual bool CanReload() const;

	// Fire When the Character is reloading
	UFUNCTION(BlueprintImplementableEvent, Category="Pac|Character", DisplayName="Reload", meta=(ScriptName = "Reload"))
	void K2_Reload();

	// Fire When the Character is reloading
	UPROPERTY(BlueprintAssignable, Category="Pac|Character")
	FPacCharacterReloadEvent OnReload;

	// Fire When the Character Finish Reloading
	UPROPERTY(BlueprintAssignable, Category="Pac|Character")
	FPacCharacterReloadEvent OnReloadFinished;

	/* ---------------------------------- Sprint ---------------------------------- */

	/** Called for sprinting input */
	virtual void Sprint();
	
	/** Called for stopping sprinting input */
	UFUNCTION(BlueprintCallable)
	virtual void StopSprint();

	// Fire When the Character is Sprinting
	UPROPERTY(BlueprintAssignable, Category="Pac|Character")
	FPacCharacterReloadEvent OnSprint;

	// Fire When the Character Finish Sprinting
	UPROPERTY(BlueprintAssignable, Category="Pac|Character")
	FPacCharacterReloadEvent OnStopSprint;

	UFUNCTION(BlueprintCallable)
	FTransform GetFeedBackShootTransForm() const;

	UFUNCTION(BlueprintCallable)
	virtual USkeletalMeshComponent* GetCharacterMesh() const { return GetMesh(); }

	UFUNCTION(BlueprintCallable)
	UPacEffectComponent* GetEffectComponent() const { return EffectComponent; }

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Victory", meta=(ScriptName = "Victory"))
	void K2_Victory();

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Defeat", meta=(ScriptName = "Defeat"))
	void K2_Defeat();

	UFUNCTION(BlueprintCallable, Category = "Pac|Character")
	TArray<TSubclassOf<UBulletsBase>> GetAllBullets() const;
	
protected:
	// Where the Player Store All Object that he can Have/Use
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pac")
	TObjectPtr<UPacBulletsComponent> BulletsComponent;

	// Health Component Used to manage the health of the character and All that is Associated
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pac")
	TObjectPtr<UPacHealthComponent> HealthComponent;

	// Buff Component Used to manage the Buffs of the character and All that is Associated
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pac")
	TObjectPtr<UPacEffectComponent> EffectComponent;

	// What is the Siurce of the Shoot for the Feedback
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pac|Feeback")
	EFeedBackShootingSource TargetingSource = EFeedBackShootingSource::CameraTowardsFocus;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pac|Feeback", meta = (EditCondition ="TargetingSource == EFeedBackShootingSource::PawnSocket", EditConditionHides))
	FName TargetingSocketName = "ShootSocket";
	
	FGameplayTagCountContainer ActiveTags;
	
	// Use Animation Time to determine the end of the Shooting or a Custom Time
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pac|Fire")
	bool bUseShootingAnimationTime = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pac|Fire", meta = (EditCondition = "!bUseShootingAnimationTime", EditConditionHides))
	float DefaultFireRate = 0.2f;

	// Use Animation Time to determine the end of the realoading or a Custom Time
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pac|Reload")
	bool bUseReloadAnimationTime = false;

	// Custom Time to use when the bUseAnimationTime is false
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pac|Reload", meta = (EditCondition = "!bUseReloadAnimationTime", EditConditionHides, ClampMin = "0", UIMin = "0"))
	float DefaultReloadTime = 0.5f;

	UFUNCTION(BlueprintCallable, Category = "Pac|Fire")
	void OnFireTimerExpired();

	UFUNCTION(BlueprintCallable, Category = "Pac|Reload")
	void OnReloadTimerExpired();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pac|Death")
	bool UseDeathMontage = true;

	// Death montage, take a random one from the array, and call EndDeath when the montage is finished !
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pac|Death", meta = (EditCondition = "UseDeathMontage", EditConditionHides))
	TArray<UAnimMontage*> DeathMontages;

	// time Before Calling EndDeath After StartDeath if There is no DeathMontage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pac|Death", meta = (EditCondition = "!UseDeathMontage", EditConditionHides))
	float DeathTime = 0.1f;

	float DefaultJumpZVelocity;

	// The Type of the Character, AI or Player
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pac")
	EPacCharacterType CharacterType;
	
	FTimerHandle FireTimerHandle;
	FTimerHandle ReloadTimerHandle;
	
	void OnAnyTagCountChanged(FGameplayTag GameplayTag, int I);
	
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	// Start the Death Process, will be called when the Character is Dead
	UFUNCTION()
	virtual void StartDeath();

	// End the Death Process
	UFUNCTION()
	virtual void EndDeath();

	// Blueprint Event Called When the Character End Death Process
	UFUNCTION(BlueprintImplementableEvent, Category = "Pac|Character", DisplayName = "EndDeath", meta=(ScriptName = "EndDeath"))
	void K2_OnEndDeath();

	// Callback When The TAG TAG_Gameplay_DoubleJump is added or removed
	void UpdateNbrOfJumps(FGameplayTag GameplayTag, int TagCount);

	// Callback When The TAG TAG_Gameplay_JumpHeigher is added or removed
	void UpdateJumpZVelocity(FGameplayTag GameplayTag, int TagCount);
	
	// Event that will be called when the Character sprint
	UFUNCTION(BlueprintImplementableEvent, Category="Pac|Character", DisplayName="Sprint", meta=(ScriptName = "Sprint"))
	void K2_Sprint();

	// Event that will be called when the Character stop sprinting
	UFUNCTION(BlueprintImplementableEvent, Category="Pac|Character", DisplayName="StopSprint", meta=(ScriptName = "StopSprint"))
	void K2_StopSprint();
};
	

