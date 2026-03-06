// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/PacWeapon.h"


APacWeapon::APacWeapon(const FObjectInitializer& ObjectInitializer)
{
	WeaponMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMeshComponent"));
	WeaponMeshComponent->SetCollisionProfileName(TEXT("NoCollision"));
	WeaponMeshComponent->SetGenerateOverlapEvents(false);
	WeaponMeshComponent->SetSimulatePhysics(false);
	WeaponMeshComponent->SetEnableGravity(false);
	WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	SetRootComponent(WeaponMeshComponent);
	
	ShootSocketName = "WeaponSocket";
}

FTransform APacWeapon::GetShootSocketTransform() const
{
	if (WeaponMeshComponent) {
		return WeaponMeshComponent->GetSocketTransform(ShootSocketName);
	}
	return FTransform::Identity;
}
