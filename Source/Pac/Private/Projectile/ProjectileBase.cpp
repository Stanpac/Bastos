// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/ProjectileBase.h"
#include "Components/BoxComponent.h"


AProjectileBase::AProjectileBase(FObjectInitializer const& ObjectInitializer) : Super(ObjectInitializer)
{
	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	
	PrimaryActorTick.bCanEverTick = false;
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	OnProjectileSpawned.Broadcast();
	OnProjectileSpawnedEvent();

	// Set LifeSpan of the projectile
	if (LifeTimeType != EProjectileLifeTimeType::CustomDestroy) {
		SetLifeSpan(LifeTime);
	}
}

void AProjectileBase::ProjectileHit(AActor* OtherActor)
{
	OnProjectileHit.Broadcast(OtherActor);
	
	if (LifeTimeType == EProjectileLifeTimeType::DestroyOnHit) {
		FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		TimerManager.SetTimerForNextTick([this](){ Destroy(); });
	}
}

void AProjectileBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnProjectileDestroyed.Broadcast();
	OnProjectileDestroyedEvent();
	
	OnProjectileSpawned.Clear();
	OnProjectileHit.Clear();
	OnProjectileDestroyed.Clear();
	
	Super::EndPlay(EndPlayReason);
}
