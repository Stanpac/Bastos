// Fill out your copyright notice in the Description page of Project Settings.


#include "Effect/PacEffectComponent.h"
#include "Effect/PacEffect.h"
#include "Utility/PacLogMacro.h"


UPacEffectComponent::UPacEffectComponent(const FObjectInitializer& ObjectInitializer)
{
}

bool UPacEffectComponent::HasEffect(FGameplayTag EffectTag) const
{
	if (EffectsMap.Contains(EffectTag)) {
		return true;
	}
	
	return false;
}

UPacEffect* UPacEffectComponent::GetEffect(FGameplayTag EffectTag) const
{
	if (EffectsMap.Contains(EffectTag)) {
		if (UPacEffect* PacEffect = EffectsMap[EffectTag]) {
			return PacEffect; 
		}
	}

	PAC_LOG_CATEGORY(PacLogEffect,"%s: Effect with tag %s not found", *FString(__FUNCTION__), *EffectTag.ToString());
	return nullptr;
}

bool UPacEffectComponent::TryAddOrUpdateEffect(TSubclassOf<UPacEffect> EffectClass, int32 StackCount)
{
	if (!EffectClass) {
		PAC_LOG_CATEGORY(PacLogEffect,"%s: Invalid Effect Class", *FString(__FUNCTION__));
		return false; 
	}

	UPacEffect* EffectCDO = EffectClass->GetDefaultObject<UPacEffect>();
	if (!EffectCDO || !EffectCDO->CanBeApplied(this)) {
		PAC_LOG_CATEGORY(PacLogEffect,"%s: Cannot apply Effect %s, either invalid or cannot be applied", *FString(__FUNCTION__), *EffectCDO->GetName());
		return false; 
	}

	FGameplayTag EffectTag = EffectCDO->EffectTag;
	UPacEffect* Effect = nullptr;
	if (HasEffect(EffectTag)) {
		Effect = GetEffect(EffectTag);
		Effect->TryModifyStackValue(StackCount);
	} else {
		if (StackCount <= 0) {
			PAC_LOG_CATEGORY(PacLogEffect,"%s: Can't remove Stack from a Effect who are not Apply, or Create a Effect With 0 Stack (%s)", *FString(__FUNCTION__), *EffectCDO->GetName());
			return false; 
		}
		
		Effect = NewObject<UPacEffect>(this, EffectClass);
		if (!Effect) {
			PAC_LOG_CATEGORY(PacLogEffect,"%s: Failed to create new Effect instance for %s", *FString(__FUNCTION__), *EffectCDO->GetName());
			return false; 
		}
		Effect->SetTargetEffectContainer(this);
		Effect->EffectApply(StackCount);
		OnEffectAdded.Broadcast(EffectTag, Effect, StackCount);
		EffectsMap.Add(EffectTag, Effect);
	}
	
	return true;
}

bool UPacEffectComponent::RemoveEffect(FGameplayTag EffectTag)
{
	if (!HasEffect(EffectTag)) {
		PAC_LOG_CATEGORY(PacLogEffect,"%s: Effect with tag %s not found", *FString(__FUNCTION__), *EffectTag.ToString());
		return false; 
	}

	UPacEffect* Effect = GetEffect(EffectTag);
	if (Effect) {
		Effect->EffectRemove();
		EffectsMap.Remove(EffectTag);
		PAC_LOG_CATEGORY(PacLogEffect,"%s: Effect %s removed", *FString(__FUNCTION__), *EffectTag.ToString());
		return true;
	}

	PAC_LOG_CATEGORY(PacLogEffect,"%s: Failed to remove Effect %s", *FString(__FUNCTION__), *EffectTag.ToString());
	return false;
}

void UPacEffectComponent::ClearAllEffects()
{
	for (auto& EffectPair : EffectsMap) {
		if (EffectPair.Value) {
			EffectPair.Value->EffectRemove(false);
		}
	}
	
	EffectsMap.Empty();
	PAC_LOG_CATEGORY(PacLogEffect, "%s: All EffectS cleared", *FString(__FUNCTION__));
}
