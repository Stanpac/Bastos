// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "PacEffect.generated.h"

class UPacEffectComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectEvent, UPacEffect*, Effect);

/**
 *  Define a Effect that can be applied to a Anyone Who has a PacEffectContainer ! 
 */
UCLASS(BlueprintType, Blueprintable)
class PAC_API UPacEffect : public UObject
{
	GENERATED_BODY()

	friend class UPacEffectComponent;
public:
	// Does this Effect should be shown in the UI ? 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bShowInUI = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Appearance, meta = (EditCondition = "bShowInUI", EditConditionHides))
	FName EffectName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Appearance, meta = (EditCondition = "bShowInUI", EditConditionHides))
	FText EffectDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Appearance, meta = (EditCondition = "bShowInUI", EditConditionHides))
	FSlateBrush EffectIcon;

	UFUNCTION(BlueprintCallable, Category = "Pac|Effect")
	UPacEffectComponent* GetTargetEffectContainer() const { return TargetEffectContainer; }

	UPROPERTY(BlueprintAssignable, Category = "Pac|Effect")
	FOnEffectEvent OnEffectUpdate;

	UPROPERTY(BlueprintAssignable, Category = "Pac|Effect")
	FOnEffectEvent OnEffectRemove;
	
#if WITH_EDITOR
	//~UObject overrides
	virtual bool ImplementsGetWorld() const override { return true; }
	//~End UObject overrides
#endif // WITH_EDITOR

protected:
	// Gameplay Tag that identify this Effect !
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay|Tag")
	FGameplayTag EffectTag;

	// Does this The Effect Tag need to be applied to the Owner of the Effect Container ?
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay|Tag")
	bool ApplyEffectTagToOwnerDurringEffect = false;

	// Additional Tags that will be granted to the Owner of the Effect Container when this Effect is running. (Remove when the Effect is removed)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay|Tag")
	FGameplayTagContainer AdditionnalTagsToApplyToOwner;
	
	// Does this Effect need to be remove When the Character Reloads ? 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gameplay)
	bool IsRemoveOnReload = true;

	// If this Effect is set to be removed on Reload, should it remove all stacks or just one ?
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gameplay, meta=(EditCondition="IsRemoveOnReload && CanBeStacked", EditConditionHides))
	bool RemoveAllStacksOnReload = false;

	// Does this Effect need to be remove When the Character Shoot ? 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gameplay)
	bool IsRemoveOnShoot = true;

	// If this Effect is set to be removed on Shoot, should it remove all stacks or just one ?
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gameplay, meta=(EditCondition="IsRemoveOnShoot && CanBeStacked", EditConditionHides))
	bool RemoveAllStacksOnShoot = false;

	// Can this Effect be Stacked ?
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gameplay)
	bool CanBeStacked = false;

	// -1 means infinite stack count
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gameplay, meta=(EditCondition="CanBeStacked", EditConditionHides, ClampMin="-1", UIMin="-1"))
	int32 MaxStackCount = -1;

	// Current Stack Count of the Effect !
	UPROPERTY(BlueprintReadOnly)
	int32 StackCount = 1;

	UFUNCTION(BlueprintCallable, Category = "Pac|Effect")
	void SetTargetEffectContainer(UPacEffectComponent* InTargetEffectContainer) { TargetEffectContainer = InTargetEffectContainer; }
	
	UFUNCTION(BlueprintCallable)
	bool CanBeApplied(UPacEffectComponent* InTargetEffectContainer) const;
	
	// Call By the Target Effect Container to try to modify the Stack Value of this Effect
	UFUNCTION(BlueprintCallable)
	int32 TryModifyStackValue(int32 StackValue = 1);

	// Call by the Target Effect Container when the Effect is applied !
	virtual void EffectApply(int32 InStackCount = 1);

	// Call by the Target Effect Container when the Effect is removed !
	virtual void EffectRemove(bool Clear = true); 
	
	virtual void EffectStackUpdate(int32 StackValue);

	UFUNCTION()
	void CheckRemoveOnReload();

	UFUNCTION()
	void CheckRemoveOnShoot();
	
	UFUNCTION(BlueprintImplementableEvent, Category="Pac|Effect", meta=(DisplayName="Effect Apply"))
	void K2_EffectApply();

	UFUNCTION(BlueprintImplementableEvent, Category="Pac|Effect", meta=(DisplayName="Effect Remove"))
	void K2_EffectRemove(bool TriggerEvent = true);
	
	UFUNCTION(BlueprintImplementableEvent, Category="Pac|Effect", meta=(DisplayName="Effect Stack Update"))
	void K2_EffectStackUpdate(int32 StackValueUpdate);

private:
	UPacEffectComponent* TargetEffectContainer = nullptr; // Effect Container that this Effect is applied to ! 
};
