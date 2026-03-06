// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullets/ProjectileBulletBase.h"

#include "Character/PacCharacter.h"
#include "Projectile/ProjectileBase.h"
#include "Utility/PacLogMacro.h"

UProjectileBulletBase::UProjectileBulletBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UProjectileBulletBase::ActivateBullet()
{
	FTransform TargetTransform = GetTargetingTransform(GetAvatarCharacter());
	Projectile = GetWorld()->SpawnActor<AProjectileBase>(ProjectileClass, TargetTransform.GetLocation(), TargetTransform.Rotator());
	Projectile->SetInstigator(GetAvatarCharacter());
	
	if (bHasBlueprintActivate) {
		K2_ActivateBullet();
	}

	if (AutoEnd) {
		EndBullet(false);
	} else {
		Projectile->OnProjectileDestroyed.AddUObject(this, &UProjectileBulletBase::OnProjectileDestroyed);
	}
}

void UProjectileBulletBase::OnProjectileDestroyed()
{
	EndBullet(false);
} 
