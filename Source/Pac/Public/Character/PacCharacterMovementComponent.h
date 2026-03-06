// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PacCharacterMovementComponent.generated.h"

/** 
 *	Custom Character Movement Component for PacCharacter.
 */
UCLASS()
class PAC_API UPacCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
public:
	UPacCharacterMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Default value use to multiply the MaxSpeed when the Character is sprinting
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pac|Movement")
	float DefaultSprintFactor = 2.0f;

	virtual bool CanAttemptJump() const override;
	virtual float GetMaxSpeed() const override;
};
