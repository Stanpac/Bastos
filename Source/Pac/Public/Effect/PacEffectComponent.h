// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Character/PacCharacter.h"
#include "Components/ActorComponent.h"
#include "PacEffectComponent.generated.h"

class UPacEffect;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEffectUpdate,FGameplayTag, Tag,  UPacEffect*, Effect, int, Statck);

/* *  Manage the Effect of a Character
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PAC_API UPacEffectComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UPacEffectComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UFUNCTION(BlueprintCallable, Category="Pac|Effects")
	bool HasEffect(FGameplayTag EffectTag) const;

	UFUNCTION(BlueprintCallable, Category="Pac|Effects")
	UPacEffect* GetEffect(FGameplayTag EffectTag) const;

	// if the Effect already exist, it will update the Stack Count
	UFUNCTION(BlueprintCallable, Category="Pac|Effects")
	bool TryAddOrUpdateEffect(TSubclassOf<UPacEffect> EffectClass, int32 StackCount = 1);

	// Call this to remove a Effect 
	UFUNCTION(BlueprintCallable, Category="Pac|Effects")
	bool RemoveEffect(FGameplayTag EffectTag);
	
	UFUNCTION(BlueprintCallable, Category="Pac|Effects")
	void ClearAllEffects();

	UFUNCTION(BlueprintCallable, Category="Pac|Effects")
	APacCharacter* GetPacCharacterOwner() const {
		return Cast<APacCharacter>(GetOwner());
	}

	// Event That Notifies when a Effect is Added (Other CallBack (Update and Remove) Are in the Effect itSelf)
	UPROPERTY(BlueprintAssignable, Category="Pac|Effects")
	FOnEffectUpdate OnEffectAdded;
	
protected:
	// Map of Currently active Effect ! 
	UPROPERTY(EditAnywhere, Category="Pac|Effects")
	TMap<FGameplayTag, UPacEffect*> EffectsMap;


};
