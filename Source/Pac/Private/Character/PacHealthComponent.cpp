// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PacHealthComponent.h"

#include "Character/PacCharacter.h"
#include "Engine/DamageEvents.h"
#include "Utility/PacGameplayTags.h"
#include "Utility/PacLogMacro.h"

UPacHealthComponent::UPacHealthComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
	
}

float UPacHealthComponent::GetHealthNormalized() const
{
	const float& Health = GetHealth();
	const float& MaxHealth = GetMaxHealth();

	return ((MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f);
}

void UPacHealthComponent::TakeDamage(int DamageAmount, AActor* Instigator)
{
	if (DamageAmount <= 0) {
		PAC_ERROR_CATEGORY(PacLog, "%s, UPacHealthComponent::DoDamage: Invalid Damage Amount %d", *GetName(), DamageAmount);
		return;
	}

	if (!IsAlive()) {
		return;
	}
	
	if (APacCharacter* OwningCharacter = Cast<APacCharacter>(GetOwner())) {
		if (OwningCharacter->HasMatchingGameplayTag(PacGameplayTags::TAG_Cheat_GodMode) ) {
			return;  // Do not apply damage if In God Mode
		}
		
		if (OwningCharacter->HasMatchingGameplayTag(PacGameplayTags::TAG_Effect_DamageImmunity) ) {
			return;  // Do not apply damage if immune
		}
	}
	
	float OldHealth = CurrentHealth;
	CurrentHealth -= DamageAmount;
	
	if (CurrentHealth <= 0) {
		CurrentHealth = 0;
		StartDeath();
	}
	
	OnTakeDamage.Broadcast(this, OldHealth, CurrentHealth, Instigator);
	OnHealthChanged.Broadcast(this, OldHealth, CurrentHealth, Instigator);
}

void UPacHealthComponent::Heal(int HealAmount, AActor* Instigator)
{
	if (HealAmount <= 0) {
		PAC_ERROR_CATEGORY(PacLog, "%s, UPacHealthComponent::Heal: Invalid Heal Amount %d", *GetName(), HealAmount);
		return;
	}

	if (!IsAlive()) {
		return;
	}

	float OldHealth = CurrentHealth;
	CurrentHealth += HealAmount;
	if (CurrentHealth > StartMaxHealth) {
		CurrentHealth = StartMaxHealth;
	}
	
	OnHeal.Broadcast(this, OldHealth, CurrentHealth, Instigator);
	OnHealthChanged.Broadcast(this, OldHealth, CurrentHealth, Instigator);
}

void UPacHealthComponent::StartDeath()
{
	OnDeath.Broadcast();
}

void UPacHealthComponent::SetHealth(int NewHealth)
{
	if (NewHealth < 0) {
		PAC_ERROR_CATEGORY(PacLog, "%s, UPacHealthComponent::SetHealth: Invalid Health %d", *GetName(), NewHealth);
		return;
	}

	if (NewHealth > StartMaxHealth) {
		NewHealth = StartMaxHealth;
	}

	float OldHealth = CurrentHealth;
	CurrentHealth = NewHealth;

	OnHealthChanged.Broadcast(this, OldHealth, CurrentHealth, GetOwner());
}

void UPacHealthComponent::ResetHealth()
{
	CurrentHealth = StartHealth;
	OnHealthChanged.Broadcast(this, 0.0f, CurrentHealth, nullptr);
}

void UPacHealthComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (StartHealth <= 0 || StartMaxHealth <= 0) {
		LOG_SCREEN_SIMPLE("%s, Invalid Health or MaxHealth in UPacHealthComponent", *GetOwner()->GetName());
	}

	CurrentHealth = StartHealth;
	OnHealthChanged.Broadcast(this, 0.0f, CurrentHealth, GetOwner());
}
