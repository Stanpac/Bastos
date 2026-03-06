// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullets/PacBulletsComponent.h"

#include "Weapon/PacWeapon.h"
#include "Bullets/BulletsBase.h"
#include "Character/PacPlayerCharacter.h"
#include "Utility/PacLogMacro.h"


UPacBulletsComponent::UPacBulletsComponent(const FObjectInitializer& ObjectInitializer)
{
	bWantsInitializeComponent = true;
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UPacBulletsComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Bullets.Num() <= 0 && DefaultBulletClass) {
		PAC_LOG("%s No bullets defined, initializing with default bullet", *FString(__FUNCTION__));
		InitializeBullets(DefaultBulletClass);
	}
}

void UPacBulletsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnNewBulletAdd.Clear();
	OnBulletCycle.Clear();
	OnBulletActivate.Clear();
	OnBulletHit.Clear();
	Super::EndPlay(EndPlayReason);
}

TSubclassOf<UBulletsBase> UPacBulletsComponent::GetActiveBullet() const
{
	if (Bullets.IsValidIndex(ActiveBulletIndex) && ActiveBulletIndex >= 0) {
		return Bullets[ActiveBulletIndex];
	}
	
	PAC_ERROR("PacWeaponComponent::GetActiveBullet: Invalid ActiveBulletIndex %d", ActiveBulletIndex);
	return nullptr;
}

void UPacBulletsComponent::CycleActiveBullet()
{
	if (Bullets.Num() <= 0) {
		PAC_ERROR("%s: No bullets available", *FString(__FUNCTION__));
	}
	
	const int32 OldIndex = (ActiveBulletIndex < 0 ? 0 : ActiveBulletIndex);
	int32 NewIndex = ActiveBulletIndex;
	NewIndex = (NewIndex + 1);
	
	if (NewIndex >= Bullets.Num()) {
		// On PacCharacter, we want to Force reload if we reach the end of the bullets list
		if (APacCharacter* Character = Cast<APacCharacter>(GetOwner())) {
			Character->Reload();
		}
	} else if (Bullets[NewIndex] != nullptr) {
		SetActiveBulletIndex(NewIndex);
	}
}

void UPacBulletsComponent::SetActiveBulletIndex(int32 NewIndex)
{
	if (Bullets.IsValidIndex(NewIndex) && (ActiveBulletIndex != NewIndex)) {
		OnBulletCycle.Broadcast(Bullets[NewIndex], ActiveBulletIndex, NewIndex, NumBullets);
		ActiveBulletIndex = NewIndex;
	}
}

void UPacBulletsComponent::AddBulletAt(int32 BulletIndex, TSubclassOf<UBulletsBase> BulletClass)
{
	if (Bullets.IsValidIndex(BulletIndex) && (Bullets[BulletIndex] != nullptr)) {
		Bullets[BulletIndex] = BulletClass;
		OnNewBulletAdd.Broadcast(BulletClass, BulletIndex);
	}
}

void UPacBulletsComponent::InitializeBullets(TSubclassOf<UBulletsBase> BulletClass)
{
	if (!BulletClass) {
		PAC_ERROR("PacWeaponComponent::InitializeBullets: Invalid Bullet Definition");
		return;
	}

	Bullets.Empty();
	Bullets.SetNum(NumBullets);
	
	for (int32 i = 0; i < NumBullets; ++i) {
		Bullets[i] = BulletClass;
	}
	
	SetActiveBulletIndex(0);
}

TArray<TSubclassOf<UBulletsBase>> UPacBulletsComponent::GetBullets() const
{
	TArray<TSubclassOf<UBulletsBase>> BulletClasses;
	for (TSubclassOf<UBulletsBase>  BulletClass : Bullets) {
		if (BulletClass) {
			BulletClasses.Add(BulletClass);
		}
	}
	return BulletClasses;
}

void UPacBulletsComponent::ActivateCurrentBullet()
{
	TSubclassOf<UBulletsBase> BulletClass = GetActiveBullet();
	
	check(GetWorld()) // Safe verification because We Use this GetWorld in UBulletsBase

	if (!BulletClass) {
		PAC_ERROR("%s: No active bullet class defined", *FString(__FUNCTION__));
		return;
	}
	
	UBulletsBase* CurrentBullet = NewObject<UBulletsBase>(this, BulletClass);
	ActivatedBulletsInstances.Add(CurrentBullet);
	
	CurrentBullet->SetBulletsComponent(this);
	
	CurrentBullet->CallActivateBullet();
	OnBulletActivate.Broadcast(BulletClass);
	
	CycleActiveBullet();
}

void UPacBulletsComponent::SetNewBullets(TArray<TSubclassOf<UBulletsBase>> InBullets)
{
	if (InBullets.IsEmpty()) {
		PAC_ERROR("%s Invalid Bullets Array", *FString(__FUNCTION__));
		return;
	}
	
	NumBullets = InBullets.Num();
	Bullets = InBullets;
	
	for (int32 i = 0; i < NumBullets; ++i) {
		OnNewBulletAdd.Broadcast(Bullets[i], i);
	}
	
	if (ActiveBulletIndex >= NumBullets || ActiveBulletIndex < 0) {
		SetActiveBulletIndex(0);
	}
}

