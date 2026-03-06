// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PacCharacter.h"

#include "AIController.h"
#include "EnhancedInputSubsystems.h"
#include "Effect/PacEffectComponent.h"
#include "Bullets/PacBulletsComponent.h"
#include "Character/PacCharacterMovementComponent.h"
#include "Character/PacHealthComponent.h"
#include "Character/PacPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameModes/PacGameMode.h"
#include "Utility/PacGameplayTags.h"
#include "Utility/PacLogMacro.h"

static FName NAME_PacCharacterCollisionProfile_Capsule(TEXT("PacCharacterCapsule"));

APacCharacter::APacCharacter(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer.SetDefaultSubobjectClass<UPacCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);
	CapsuleComp->SetCollisionProfileName(NAME_PacCharacterCollisionProfile_Capsule);

	// Set the Character Movement Component
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->GravityScale = 1.0f;
	MovementComponent->MaxAcceleration = 2400.0f;
	MovementComponent->BrakingFrictionFactor = 1.0f;
	MovementComponent->BrakingFriction = 6.0f;
	MovementComponent->GroundFriction = 8.0f;
	MovementComponent->BrakingDecelerationWalking = 1400.0f;
	MovementComponent->bUseControllerDesiredRotation = false;
	MovementComponent->bOrientRotationToMovement = false;
	MovementComponent->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	MovementComponent->bAllowPhysicsRotationDuringAnimRootMotion = false;
	MovementComponent->bCanWalkOffLedges = true;
	MovementComponent->SetCrouchedHalfHeight(65.0f);
	
	BulletsComponent = CreateDefaultSubobject<UPacBulletsComponent>(TEXT("PacBulletsComponent"));
	HealthComponent = CreateDefaultSubobject<UPacHealthComponent>(TEXT("PacHealthComponent"));
	EffectComponent = CreateDefaultSubobject<UPacEffectComponent>(TEXT("PacEffectComponent"));

	CharacterType = EPacCharacterType::AI;
}

void APacCharacter::OnAnyTagCountChanged(FGameplayTag GameplayTag, int I)
{
	if (I == 0) {
		Tags.Remove(GameplayTag.GetTagName());
		return;
	}

	if (I == 1) {
		Tags.AddUnique(GameplayTag.GetTagName());
		return;
	}

	PAC_ERROR("%s: Unexpected Tag Count Change %d for Tag %s", *FString(__FUNCTION__), I, *GameplayTag.ToString());
}

void APacCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	DefaultJumpZVelocity = GetCharacterMovement()->JumpZVelocity;
	
	FOnGameplayEffectTagCountChanged& DoubleJumpTagCountChangedDelegate = ActiveTags.RegisterGameplayTagEvent(PacGameplayTags::TAG_Effect_DoubleJump, EGameplayTagEventType::NewOrRemoved);
	DoubleJumpTagCountChangedDelegate.AddUObject(this, &APacCharacter::UpdateNbrOfJumps);

	// FOnGameplayEffectTagCountChanged& JumpHeigherTagCountChangedDelegate = ActiveTags.RegisterGameplayTagEvent(PacGameplayTags::TAG_Gameplay_JumpHeigher, EGameplayTagEventType::NewOrRemoved);
	// JumpHeigherTagCountChangedDelegate.AddUObject(this, &APacCharacter::UpdateJumpZVelocity);

	HealthComponent->OnDeath.AddDynamic(this, &APacCharacter::StartDeath);

	ActiveTags.RegisterGenericGameplayEvent().AddUObject(this, &APacCharacter::OnAnyTagCountChanged);
}

void APacCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (APacGameMode* PacGM = Cast<APacGameMode>(GetWorld()->GetAuthGameMode())) {
		if (CharacterType == EPacCharacterType::Player) {
			PacGM->SetCurrentPlayerCharacter(Cast<APacPlayerCharacter>(this));
		} else {
			PacGM->TryAddEnemy(this);
		}
	}
}

void APacCharacter::UnPossessed()
{
	Super::UnPossessed();
}

void APacCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>(); 

	if (Controller != nullptr) {
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void APacCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr) {
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

FVector APacCharacter::GetFacingDirection() const
{
	if (Controller == nullptr) {
		return FVector::ZeroVector;
	}

	// Get the forward vector of the controller's rotation
	const FRotator& ControlRotation = Controller->GetControlRotation();
	return FRotationMatrix(ControlRotation).GetScaledAxis(EAxis::X);
}

void APacCharacter::Jump()
{
	Super::Jump();
}

void APacCharacter::StopJumping()
{
	Super::StopJumping();
}

void APacCharacter::CheckJumpInput(float DeltaTime)
{
	Super::CheckJumpInput(DeltaTime);
}

void APacCharacter::OnJumped_Implementation()
{
	OnJumpedEvent.Broadcast();
}

void APacCharacter::Fire()
{
	if (!CanFire()) {
		PAC_WARNING_CATEGORY(PacCharacterLog, "%s : Cannot fire because CanFire() function returned false", *GetName());
		return;
	}

	K2_Fire();
	BulletsComponent->ActivateCurrentBullet();

	if (!bUseShootingAnimationTime) {
		FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &APacCharacter::OnFireTimerExpired);
		TimerManager.SetTimer(FireTimerHandle, Delegate, DefaultFireRate, false);
	}
	
	ActiveTags.UpdateTagCount(PacGameplayTags::TAG_Gameplay_PreventFromShooting, 1);
	OnShoot.Broadcast();
}

bool APacCharacter::CanFire() const
{
	// Check if we have the Tag to prevent from shooting
	if (ActiveTags.HasMatchingGameplayTag(PacGameplayTags::TAG_Gameplay_PreventFromShooting)) {
		return false;
	}

	// Check if we are in Death state
	if (ActiveTags.HasMatchingGameplayTag(PacGameplayTags::TAG_Gameplay_InDeath)) {
		return false;
	}

	// Check if we have a valid Bullets Component and if it has a bullet to shoot
	if (!BulletsComponent || !BulletsComponent->HasBullets()) {
		return false;
	}
	
	return true;
}

void APacCharacter::Reload()
{
	if (!CanReload()) {
		PAC_WARNING_CATEGORY(PacCharacterLog, "%s : Cannot reload because CanReload() function returned false", *GetName());
		return;
	}

	K2_Reload();

	if (!bUseReloadAnimationTime) {
		FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &APacCharacter::OnReloadTimerExpired);
		TimerManager.SetTimer(ReloadTimerHandle, Delegate, DefaultReloadTime, false);
	}
	
	ActiveTags.UpdateTagCount(PacGameplayTags::TAG_Gameplay_PreventFromReloading, 1);
	ActiveTags.UpdateTagCount(PacGameplayTags::TAG_Gameplay_PreventFromShooting, 1);

	OnReload.Broadcast();
}

bool APacCharacter::CanReload() const
{
	if (ActiveTags.HasMatchingGameplayTag(PacGameplayTags::TAG_Gameplay_PreventFromReloading)) {
		return false;
	}

	// Check if we are in Death state
	if (ActiveTags.HasMatchingGameplayTag(PacGameplayTags::TAG_Gameplay_InDeath)) {
		return false;
	}

	if (!BulletsComponent || BulletsComponent->GetActiveBulletIndex() <= 0) {
		return false;
	}
	
	return true;
}

void APacCharacter::Sprint()
{
	ActiveTags.UpdateTagCount(PacGameplayTags::TAG_Gameplay_Sprint, 1);
	K2_Sprint();

	OnSprint.Broadcast();
}

void APacCharacter::StopSprint()
{
	// Check if we are already stopped
	if (ActiveTags.UpdateTagCount(PacGameplayTags::TAG_Gameplay_Sprint, -1)) { 
		K2_StopSprint();
		OnStopSprint.Broadcast();
	}
}

void APacCharacter::StartDeath()
{
	// Notify the Gamemode that this character Is Dead 
	if (APacGameMode* PacGM = Cast<APacGameMode>(GetWorld()->GetAuthGameMode())){
		if (CharacterType == EPacCharacterType::Player) {
			PacGM->PlayerStartDeath(this);
		} else {
			PacGM->RemoveEnemy(this);
		}
	}

	if (Controller) {
		Controller->SetIgnoreMoveInput(true);
		Controller->SetIgnoreLookInput(true);
	}

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	GetCharacterMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
	
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	float EndDeathTimer = DeathTime;
	
	if (UseDeathMontage && !DeathMontages.IsEmpty()) {
		UAnimInstance* AnimInstance = GetCharacterMesh()->GetAnimInstance();
		if (AnimInstance) {
			int32 RandomIndex = FMath::RandRange(0, DeathMontages.Num() - 1);
			UAnimMontage* SelectedMontage = DeathMontages[RandomIndex];
			if (SelectedMontage) {
				AnimInstance->Montage_Play(SelectedMontage);
				EndDeathTimer = SelectedMontage->GetPlayLength();
			}
		}
	}
	
	// TODO Start Sequencer ?

	UpdateTagCount(PacGameplayTags::TAG_Gameplay_InDeath, 1);
				
	FireTimerHandle = FTimerHandle();
	GetWorldTimerManager().SetTimer(FireTimerHandle, FTimerDelegate::CreateUObject(this, &APacCharacter::EndDeath), EndDeathTimer , false);
}

void APacCharacter::EndDeath()
{
	SetActorHiddenInGame(true);
	K2_OnEndDeath();
	
	if (CharacterType == EPacCharacterType::Player) {
		DetachFromControllerPendingDestroy();
	}
	
	SetLifeSpan(0.1f);
}

FTransform APacCharacter::GetFeedBackShootTransForm() const
{
	FVector CamLoc;
	FRotator CamRot;

	if (TargetingSource == EFeedBackShootingSource::CameraTowardsFocus) {
		APlayerController* PC = Cast<APlayerController>(GetController());
		
		if (PC != nullptr) {
			PC->GetPlayerViewPoint(/*out*/ CamLoc, /*out*/ CamRot);
		} else if (AAIController* AIController = Cast<AAIController>(GetController())) {
			CamLoc = GetActorLocation() + FVector(0, 0, BaseEyeHeight);
			CamRot = GetControlRotation();
		}

		return FTransform(CamRot, CamLoc);
	}
	
	if (TargetingSource == EFeedBackShootingSource::PawnForward) {
		return GetTransform();
	}
	
	if (TargetingSource == EFeedBackShootingSource::WeaponSocket) {
		/* APacWeapon* Weapon = BulletsComponent->GetSpawnedWeaponActor();
		if (Weapon) {
			return Weapon->GetShootSocketTransform();
		} */
		return GetCharacterMesh()->GetSocketTransform(TargetingSocketName);
	}

	if (TargetingSource == EFeedBackShootingSource::PawnSocket) {
		return GetCharacterMesh()->GetSocketTransform(TargetingSocketName);
	}
	
	PAC_ERROR("%s, Invalid FeedBack Shoot Source %s", *GetName(), *UEnum::GetValueAsString(TargetingSource));
	return FTransform(); // Default case, return an identity transform
}

TArray<TSubclassOf<UBulletsBase>> APacCharacter::GetAllBullets() const
{
	if (BulletsComponent) {
		return BulletsComponent->GetBullets();
	}
	
	return TArray<TSubclassOf<UBulletsBase>>();
}

void APacCharacter::OnFireTimerExpired()
{
	ActiveTags.UpdateTagCount(PacGameplayTags::TAG_Gameplay_PreventFromShooting, -1);
}

void APacCharacter::OnReloadTimerExpired()
{
	BulletsComponent->SetActiveBulletIndex(0); 
 	
 	ActiveTags.UpdateTagCount(PacGameplayTags::TAG_Gameplay_PreventFromReloading, -1);
 	ActiveTags.UpdateTagCount(PacGameplayTags::TAG_Gameplay_PreventFromShooting, -1);

	OnReloadFinished.Broadcast();
}

void APacCharacter::UpdateNbrOfJumps(FGameplayTag GameplayTag, int TagCount)
{
	if (TagCount > 0) {
		// If the tag is added, we can increase the JumpMaxCount
		JumpMaxCount++;
	} else {
		// If the tag is removed, we can decrease the JumpMaxCount
		JumpMaxCount--;
	}
}

/*void APacCharacter::UpdateJumpZVelocity(FGameplayTag GameplayTag, int TagCount)
{
	if (TagCount > 0) {
		// If the tag is added, we can increase the JumpZVelocity
		GetCharacterMovement()->JumpZVelocity = DefaultJumpZVelocity * DefaultJumpHeightFactor;
	} else {
		// If the tag is removed, we can decrease the JumpZVelocity
		GetCharacterMovement()->JumpZVelocity = DefaultJumpZVelocity;
	}
}*/

void APacCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.Reset();
	TagContainer.AppendTags(ActiveTags.GetExplicitGameplayTags());
}

bool APacCharacter::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	return ActiveTags.HasMatchingGameplayTag(TagToCheck);
}

bool APacCharacter::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	return ActiveTags.HasAllMatchingGameplayTags(TagContainer);
}

bool APacCharacter::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	return ActiveTags.HasAnyMatchingGameplayTags(TagContainer);
}

void APacCharacter::UpdateTagsCount(const FGameplayTagContainer Container, int32 CountDelta)
{
	ActiveTags.UpdateTagCount(Container, CountDelta);
}

void APacCharacter::UpdateTagCount(const FGameplayTag Tag, int32 CountDelta)
{
	ActiveTags.UpdateTagCount(Tag, CountDelta);
}

bool APacCharacter::SetTagCount(const FGameplayTag Tag, int32 NewCount)
{
	return ActiveTags.SetTagCount(Tag, NewCount);
}

int32 APacCharacter::GetTagCount(const FGameplayTag Tag) const
{
	return ActiveTags.GetTagCount(Tag);
}

void APacCharacter::AddTemporaryTag(const FGameplayTag Tag, float Duration, int Count)
{
	if (Count <= 0 || Duration <= 0.0f) {
		PAC_WARNING_CATEGORY(PacCharacterLog, "Cannot add temporary tag %s with count %d and duration %f", *Tag.ToString(), Count, Duration);
		return;
	}
	ActiveTags.UpdateTagCount(Tag, Count);
	FTimerManager& TimerManager = GetWorldTimerManager();
	FTimerHandle TagTimerHandle = FTimerHandle();
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &APacCharacter::UpdateTagCount, Tag, -Count);
	TimerManager.SetTimer(TagTimerHandle, TimerDelegate, Duration, false);
}

void APacCharacter::RemoveTemporaryTag(const FGameplayTag Tag, float Duration, int Count)
{
	if (Count <= 0 || Duration <= 0.0f) {
		PAC_WARNING_CATEGORY(PacCharacterLog, "Cannot Remove temporary tag %s with count %d and duration %f", *Tag.ToString(), Count, Duration);
		return;
	}

	ActiveTags.UpdateTagCount(Tag, -Count);
	FTimerManager& TimerManager = GetWorldTimerManager();
	FTimerHandle TagTimerHandle = FTimerHandle();
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &APacCharacter::UpdateTagCount, Tag, Count);
	TimerManager.SetTimer(TagTimerHandle, TimerDelegate, Duration, false);
}

