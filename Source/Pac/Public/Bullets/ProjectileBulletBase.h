// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BulletsBase.h"
#include "ProjectileBulletBase.generated.h"

class AProjectileBase;

/**
 *  Class that Spawn a Projectile and not a Raycast 
 */
UCLASS()
class PAC_API UProjectileBulletBase : public UBulletsBase
{
	GENERATED_BODY()
public:
	UProjectileBulletBase(const FObjectInitializer& ObjectInitializer);
	
	virtual void ActivateBullet() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay)
	TSubclassOf<AProjectileBase> ProjectileClass;

	UPROPERTY(BlueprintReadOnly)
	AProjectileBase* Projectile;
	
	UFUNCTION(BlueprintCallable, Category = Pac)
	void OnProjectileDestroyed();
};

