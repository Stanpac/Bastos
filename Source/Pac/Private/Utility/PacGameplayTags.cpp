// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/PacGameplayTags.h"

#include "Utility/PacLogMacro.h"

namespace PacGameplayTags
{
	// Gameplay Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gameplay_MovementStopped, "Gameplay.MovementStopped", "Movement Stopped");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gameplay_JumpNotAllowed, "Gameplay.JumpNotAllowed", "Jump Not Allowed");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gameplay_Sprint, "Gameplay.Sprint", "If the Character is sprinting");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gameplay_IsReloading, "Gameplay.IsReloading", "If the Character is reloading");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gameplay_PreventFromShooting, "Gameplay.PreventFromShooting", "Prevent From Shooting");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gameplay_PreventFromReloading, "Gameplay.PreventFromReloading", "Prevent From Reloading");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gameplay_CoyoteTime, "Gameplay.CoyoteTime", "allows the character to jump for a short time after leaving a ledge");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gameplay_InDeath, "Gameplay.InDeath", "In Death Gameplay Tag");

	// Cheat Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Cheat_GodMode, "Cheat.GodMode", "God Mode Cheat Tag");

	// Effects Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_Kamikaze, "Effect.Kamikaze", "Kamikaze Effect Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_DoubleJump, "Effect.DoubleJump", "Double Jump Effect Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_DamageBoost, "Effect.DamageBoost", "Damage Boost Effect Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_DamageImmunity, "Effect.DamageImmunity", "Damage Immunity Effect Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_Propulsion, "Effect.Propulsion", "Propulsion Effect Tag");
}

void FGameplayTagCountContainer::Notify_StackCountChange(const FGameplayTag& Tag)
{	
	// The purpose of this function is to let anyone listening on the EGameplayTagEventType::AnyCountChange event know that the 
	// stack count of a GE that was backing this GE has changed. We do not update our internal map/count with this info, since that
	// map only counts the number of GE/sources that are giving that tag.
	FGameplayTagContainer TagAndParentsContainer = Tag.GetGameplayTagParents();
	for (auto CompleteTagIt = TagAndParentsContainer.CreateConstIterator(); CompleteTagIt; ++CompleteTagIt)
	{
		const FGameplayTag& CurTag = *CompleteTagIt;
		FDelegateInfo* DelegateInfo = GameplayTagEventMap.Find(CurTag);
		if (DelegateInfo)
		{
			int32 TagCount = GameplayTagCountMap.FindOrAdd(CurTag);
			DelegateInfo->OnAnyChange.Broadcast(CurTag, TagCount);
		}
	}
}

FOnGameplayEffectTagCountChanged& FGameplayTagCountContainer::RegisterGameplayTagEvent(const FGameplayTag& Tag, EGameplayTagEventType EventType)
{
	FDelegateInfo& Info = GameplayTagEventMap.FindOrAdd(Tag);

	if (EventType == EGameplayTagEventType::NewOrRemoved)
	{
		return Info.OnNewOrRemove;
	}

	return Info.OnAnyChange;
}

void FGameplayTagCountContainer::Reset(bool bResetCallbacks)
{
	GameplayTagCountMap.Reset();
	ExplicitTagCountMap.Reset();
	ExplicitTags.Reset();

	if (bResetCallbacks)
	{
		GameplayTagEventMap.Reset();
		OnAnyTagChangeDelegate.Clear();
	}
}

bool FGameplayTagCountContainer::UpdateExplicitTags(const FGameplayTag& Tag, const int32 CountDelta, const bool bDeferParentTagsOnRemove)
{
	const bool bTagAlreadyExplicitlyExists = ExplicitTags.HasTagExact(Tag);

	// Need special case handling to maintain the explicit tag list correctly, adding the tag to the list if it didn't previously exist and a
	// positive delta comes in, and removing it from the list if it did exist and a negative delta comes in.
	if (!bTagAlreadyExplicitlyExists)
	{
		// Brand new tag with a positive delta needs to be explicitly added
		if (CountDelta > 0)
		{
			ExplicitTags.AddTag(Tag);
		}
		// Block attempted reduction of non-explicit tags, as they were never truly added to the container directly
		else
		{
			// only warn about tags that are in the container but will not be removed because they aren't explicitly in the container
			if (ExplicitTags.HasTag(Tag))
			{
				PAC_WARNING("Attempted to remove tag: %s from tag count container, but it is not explicitly in the container!", *Tag.ToString());
			}
			return false;
		}
	}

	// Update the explicit tag count map. This has to be separate than the map below because otherwise the count of nested tags ends up wrong
	int32& ExistingCount = ExplicitTagCountMap.FindOrAdd(Tag);

	ExistingCount = FMath::Max(ExistingCount + CountDelta, 0);

	// If our new count is 0, remove us from the explicit tag list
	if (ExistingCount <= 0)
	{
		// Remove from the explicit list
		ExplicitTags.RemoveTag(Tag, bDeferParentTagsOnRemove);
	}

	return true;
}

bool FGameplayTagCountContainer::GatherTagChangeDelegates(const FGameplayTag& Tag, const int32 CountDelta, TArray<FDeferredTagChangeDelegate>& TagChangeDelegates)
{
	// Check if change delegates are required to fire for the tag or any of its parents based on the count change
	FGameplayTagContainer TagAndParentsContainer = Tag.GetGameplayTagParents();
	bool CreatedSignificantChange = false;
	for (auto CompleteTagIt = TagAndParentsContainer.CreateConstIterator(); CompleteTagIt; ++CompleteTagIt)
	{
		const FGameplayTag& CurTag = *CompleteTagIt;

		// Get the current count of the specified tag. NOTE: Stored as a reference, so subsequent changes propagate to the map.
		int32& TagCountRef = GameplayTagCountMap.FindOrAdd(CurTag);

		const int32 OldCount = TagCountRef;

		// Apply the delta to the count in the map
		int32 NewTagCount = FMath::Max(OldCount + CountDelta, 0);
		TagCountRef = NewTagCount;

		// If a significant change (new addition or total removal) occurred, trigger related delegates
		const bool SignificantChange = (OldCount == 0 || NewTagCount == 0);
		CreatedSignificantChange |= SignificantChange;
		if (SignificantChange)
		{
			TagChangeDelegates.AddDefaulted();
			TagChangeDelegates.Last().BindLambda([Delegate = OnAnyTagChangeDelegate, CurTag, NewTagCount]()
			{
				Delegate.Broadcast(CurTag, NewTagCount);
			});
		}

		FDelegateInfo* DelegateInfo = GameplayTagEventMap.Find(CurTag);
		if (DelegateInfo)
		{
			TagChangeDelegates.AddDefaulted();
			TagChangeDelegates.Last().BindLambda([Delegate = DelegateInfo->OnAnyChange, CurTag, NewTagCount]()
			{
				Delegate.Broadcast(CurTag, NewTagCount);
			});

			if (SignificantChange)
			{
				TagChangeDelegates.AddDefaulted();
				TagChangeDelegates.Last().BindLambda([Delegate = DelegateInfo->OnNewOrRemove, CurTag, NewTagCount]()
				{
					Delegate.Broadcast(CurTag, NewTagCount);
				});
			}
		}
	}

	return CreatedSignificantChange;
}

bool FGameplayTagCountContainer::UpdateTagMap_Internal(const FGameplayTag& Tag, int32 CountDelta)
{
	if (!UpdateExplicitTags(Tag, CountDelta, false))
	{
		return false;
	}

	TArray<FDeferredTagChangeDelegate> DeferredTagChangeDelegates;
	bool bSignificantChange = GatherTagChangeDelegates(Tag, CountDelta, DeferredTagChangeDelegates);
	for (FDeferredTagChangeDelegate& Delegate : DeferredTagChangeDelegates)
	{
		Delegate.Execute();
	}

	return bSignificantChange;
}

bool FGameplayTagCountContainer::UpdateTagMapDeferredParentRemoval_Internal(const FGameplayTag& Tag, int32 CountDelta, TArray<FDeferredTagChangeDelegate>& DeferredTagChangeDelegates)
{
	if (!UpdateExplicitTags(Tag, CountDelta, true))
	{
		return false;
	}

	return GatherTagChangeDelegates(Tag, CountDelta, DeferredTagChangeDelegates);
}
	
