// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PacWeapon.generated.h"

UCLASS()
class PAC_API APacWeapon : public AActor
{
	GENERATED_BODY()

public:
	APacWeapon(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(VisibleAnywhere , BlueprintReadOnly, Category = "Pac")
	TObjectPtr<USkeletalMeshComponent> WeaponMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pac")
	FName ShootSocketName = "WeaponSocket";

	UFUNCTION(BlueprintCallable, Category = "Pac") 
	FTransform GetShootSocketTransform() const;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnShoot();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnReload();
};
