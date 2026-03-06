// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PacCharacter.h"
#include "Player/PacPlayerController.h"
#include "PacPlayerCharacter.generated.h"

class UCameraComponent;
class UPacInventoryComponent;
class UInputMappingContext;
class UInputAction;

/**
 * Base Class for Player Character in the Pac project.
 */
UCLASS()
class PAC_API APacPlayerCharacter : public APacCharacter
{
	GENERATED_BODY()
public:
	APacPlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Pac|Character")
	APacPlayerController* GetPacPlayerController() const { return Cast<APacPlayerController>(GetController()); }

	virtual USkeletalMeshComponent* GetCharacterMesh() const override { return FirsPersonMesh; }

	UFUNCTION(BlueprintCallable, Category = "Pac|Character")
	UCameraComponent* GetCameraComponent() const { return CameraComponent; }

	UFUNCTION(BlueprintCallable, Category = "Pac|Character")
	UPacHealthComponent* GetHealthComponent() const { return HealthComponent; }
	
protected:
	// First Person Mesh
	UPROPERTY(VisibleAnywhere , BlueprintReadOnly, Category = "Pac")
	TObjectPtr<USkeletalMeshComponent> FirsPersonMesh;

	// Camera Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pac")
	TObjectPtr<UCameraComponent> CameraComponent;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pac|Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pac|Input", meta=(AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pac|Input", meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pac|Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Sprint Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pac|Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pac|Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	/** Reload Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pac|Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;

	UFUNCTION()
	void CheckCoyoteTimeTagCountChanged(FGameplayTag GameplayTag, int TagCount);
	
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
};
