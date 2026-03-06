// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PacCharacterMovementComponent.h"

#include "GameplayTagAssetInterface.h"
#include "Utility/PacGameplayTags.h"

UPacCharacterMovementComponent::UPacCharacterMovementComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

bool UPacCharacterMovementComponent::CanAttemptJump() const
{
	if (IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(GetOwner())) {
		if (TagInterface->HasMatchingGameplayTag(PacGameplayTags::TAG_Gameplay_JumpNotAllowed)) {
			return false;
		}
	}
	
	return Super::CanAttemptJump();
}

float UPacCharacterMovementComponent::GetMaxSpeed() const
{
	if (IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(GetOwner())) {
		if (TagInterface->HasMatchingGameplayTag(PacGameplayTags::TAG_Gameplay_MovementStopped)) {
			return 0;
		}

		if (TagInterface->HasMatchingGameplayTag(PacGameplayTags::TAG_Gameplay_Sprint)) {
			return Super::GetMaxSpeed() * DefaultSprintFactor;
		}
	}
	
	return Super::GetMaxSpeed();
}

