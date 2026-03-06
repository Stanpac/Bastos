// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PacHealthComponent.generated.h"

class APacCharacter;
class UPacAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPacDeathEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FPacHealthChanged, UPacHealthComponent*, HealthComponent, int, OldValue, int, NewValue, AActor*, Instigator);

/*
 * Manage the Health of a Character
 *	Need to have a PacAbilitySystemComponent to be able to use the Health Component
 */
UCLASS(ClassGroup=(Pac), meta=(BlueprintSpawnableComponent))
class PAC_API UPacHealthComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UPacHealthComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Returns the current health value.
	UFUNCTION(BlueprintCallable, Category = "Pac|Health")
	int GetHealth() const { return CurrentHealth; }

	// Returns the current maximum health value.
	UFUNCTION(BlueprintCallable, Category = "Pac|Health")
	int GetMaxHealth() const { return StartMaxHealth > 0 ? StartMaxHealth : 1.f;}

	// Returns the current health in the range [0.0, 1.0].
	UFUNCTION(BlueprintCallable, Category = "Pac|Health")
	float GetHealthNormalized() const;

	// Set the Health (Clamp Between 0 and Max Health)
	UFUNCTION(BlueprintCallable, Category = "Pac|Health")
	void SetHealth(int NewHealth);

	// Set the Health to the Start Health Value
	UFUNCTION(BlueprintCallable, Category = "Pac|Health")
	void ResetHealth();
	
	// Damage Process (- Health)
	UFUNCTION(BlueprintCallable, Category = "Pac|Health")
	void TakeDamage(int DamageAmount, AActor* Instigator);

	// Healing Process (+ Health)
	UFUNCTION(BlueprintCallable, Category = "Pac|Health")
	void Heal(int HealAmount, AActor* Instigator);

	virtual bool IsAlive() const { return CurrentHealth > 0; }

	UFUNCTION(BlueprintCallable, Category = "Pac|Death")
	void StartDeath();

	
	// Fire When the Health is Changed (e.g. DoDamage, Heal)
	UPROPERTY(BlueprintAssignable)
	FPacHealthChanged OnHealthChanged;

	// Fire when Max Health is Changed 
	UPROPERTY(BlueprintAssignable)
	FPacHealthChanged OnMaxHealthChanged;

	/** Fire When the Character take Damage (OnHealthChanged is Trigger Too)
	 *  We Want To handle Process Who is Related to the take Damage But not the change of Health
	 */
	UPROPERTY(BlueprintAssignable)
	FPacHealthChanged OnTakeDamage;

	/** Fire When the Character Heal (OnHealthChanged is Trigger Too)
	 *  We Want To handle Process Who is Related to Healing But not the change of Health
	 */
	UPROPERTY(BlueprintAssignable)
	FPacHealthChanged OnHeal;

	// Fire When the Character Start Death Process
	UPROPERTY(BlueprintAssignable)
	FPacDeathEvent OnDeath;

protected:
	UPROPERTY(EditAnywhere, Category="Pac|Health", meta=(ClampMin="0.0", UIMin="0.0"))
	int StartHealth = 3;

	UPROPERTY(EditAnywhere, Category="Pac|Health", meta=(ClampMin="0.0", UIMin="0.0"))
	int StartMaxHealth = 3;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Pac|Health")
	int CurrentHealth = 0;

private:

	virtual void InitializeComponent() override;
};
