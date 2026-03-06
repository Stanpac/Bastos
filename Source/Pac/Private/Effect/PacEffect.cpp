// Fill out your copyright notice in the Description page of Project Settings.


#include "Effect/PacEffect.h"

#include "Effect/PacEffectComponent.h"
#include "Character/PacCharacter.h"
#include "Utility/PacLogMacro.h"

bool UPacEffect::CanBeApplied(UPacEffectComponent* InTargetEffectContainer) const
{
	if (!InTargetEffectContainer) {
		return false;
	}

	if (IsRemoveOnReload || IsRemoveOnShoot) {
		if (InTargetEffectContainer->GetPacCharacterOwner() == nullptr) {
			return false;  // This Effect can only be applied to characters that can reload or shoot 
		}
	}

	if (InTargetEffectContainer->HasEffect(EffectTag)) {
		UPacEffect* ExistingEffect = InTargetEffectContainer->GetEffect(EffectTag);
		if (!ExistingEffect || !ExistingEffect->CanBeStacked) {
			return false;
		}

		if (MaxStackCount != -1 && ExistingEffect->StackCount >= MaxStackCount) {
			return false; // Cannot stack, max stack count reached 
		}
	}

	return true; // Effect can be applied
}

int32 UPacEffect::TryModifyStackValue(int32 StackValue)
{
	if (StackValue == 0) {
		PAC_LOG_CATEGORY(PacLogEffect, "%s: Invalid stack value %d, must be different from 0", *FString(__FUNCTION__), StackValue);
		return 0; 
	}
	
	int32 NewStackCount  = FMath::Clamp(StackCount + StackValue, 0,CanBeStacked ? MaxStackCount == -1 ? INT32_MAX : MaxStackCount : 1);
	if (NewStackCount == StackCount) {
		PAC_LOG_CATEGORY(PacLogEffect, "%s: Effect %s stack count unchanged (%d)", *FString(__FUNCTION__), *EffectTag.ToString(), StackCount);
		return 0; // No change in stack count
	}
	
	if (NewStackCount <= 0 && TargetEffectContainer) {
		TargetEffectContainer->RemoveEffect(EffectTag);
		return 0;
	}
	
	int32 StackDifference = NewStackCount - StackCount;
	StackCount = NewStackCount;
	EffectStackUpdate(StackDifference);
	PAC_LOG_CATEGORY(PacLogEffect, "%s: Effect %s now has %d stacks", *FString(__FUNCTION__), *EffectTag.ToString(), StackCount);

	return StackDifference; // Return the number of stacks added or removed
}

void UPacEffect::EffectApply(int32 InStackCount)
{
	check(StackCount > 0);
	
	if (CanBeStacked) {
		StackCount = InStackCount;
	}
	
	PAC_LOG_CATEGORY(PacLogEffect, "%s: Effect %s applied with %d stacks", *FString(__FUNCTION__), *EffectTag.ToString(), StackCount);
	
	if (IGameplayTagCountContainerInterface* TagContainerInterface = Cast<IGameplayTagCountContainerInterface>(TargetEffectContainer->GetOwner())) {
		if (ApplyEffectTagToOwnerDurringEffect) {
			TagContainerInterface->UpdateTagCount(EffectTag, StackCount);
		}

		if (AdditionnalTagsToApplyToOwner.Num() > 0) {
			TagContainerInterface->UpdateTagsCount(AdditionnalTagsToApplyToOwner, 1);
		}
	}

	if (IsRemoveOnReload) {
		check(TargetEffectContainer->GetPacCharacterOwner());
		TargetEffectContainer->GetPacCharacterOwner()->OnReload.AddDynamic(this, &UPacEffect::CheckRemoveOnReload);
	}

	if (IsRemoveOnShoot) {
		check(TargetEffectContainer->GetPacCharacterOwner());
		TargetEffectContainer->GetPacCharacterOwner()->OnShoot.AddDynamic(this, &UPacEffect::CheckRemoveOnShoot);
	}
	
	K2_EffectApply();
}

void UPacEffect::EffectRemove(bool clear)
{
	if (IGameplayTagCountContainerInterface* TagContainerInterface = Cast<IGameplayTagCountContainerInterface>(TargetEffectContainer->GetOwner())) {
		if (ApplyEffectTagToOwnerDurringEffect) {
			TagContainerInterface->UpdateTagCount(EffectTag, -StackCount);
		}

		if (AdditionnalTagsToApplyToOwner.Num() > 0) {
			TagContainerInterface->UpdateTagsCount(AdditionnalTagsToApplyToOwner, -1);
		}
	}

	// Event Clear 
	if (IsRemoveOnReload) {
		check(TargetEffectContainer->GetPacCharacterOwner());
		TargetEffectContainer->GetPacCharacterOwner()->OnReload.RemoveDynamic(this, &UPacEffect::CheckRemoveOnReload);
	}

	if (IsRemoveOnShoot) {
		check(TargetEffectContainer->GetPacCharacterOwner());
		TargetEffectContainer->GetPacCharacterOwner()->OnShoot.RemoveDynamic(this, &UPacEffect::CheckRemoveOnShoot);
	}
	
	OnEffectRemove.Broadcast(this);
	
	K2_EffectRemove(clear);
}

void UPacEffect::EffectStackUpdate(int32 StackValue)
{
	if (ApplyEffectTagToOwnerDurringEffect) {
		if (IGameplayTagCountContainerInterface* TagContainerInterface = Cast<IGameplayTagCountContainerInterface>(TargetEffectContainer->GetOwner())) {
			TagContainerInterface->UpdateTagCount(EffectTag, StackValue);
		}
	}
	K2_EffectStackUpdate(StackValue);
	
	OnEffectUpdate.Broadcast(this);
}

void UPacEffect::CheckRemoveOnReload()
{
	if (!IsRemoveOnReload) {
		PAC_ERROR_CATEGORY(PacLogEffect, "%s: Effect %s is not set to be removed on reload, but is Call like it", *FString(__FUNCTION__), *EffectTag.ToString());
		return;
	}
	
	if (CanBeStacked && RemoveAllStacksOnReload && TargetEffectContainer) {
		TargetEffectContainer->RemoveEffect(EffectTag);
	} else {
		TryModifyStackValue(-1);
	}
}

void UPacEffect::CheckRemoveOnShoot()
{
	if (!IsRemoveOnShoot) {
		PAC_ERROR_CATEGORY(PacLogEffect, "%s: Effect %s is not set to be removed on Shoot, but is Call like it", *FString(__FUNCTION__), *EffectTag.ToString());
		return;
	}

	if (CanBeStacked && RemoveAllStacksOnShoot && TargetEffectContainer) {
		TargetEffectContainer->RemoveEffect(EffectTag);
	} else {
		TryModifyStackValue(-1);
	}
}


