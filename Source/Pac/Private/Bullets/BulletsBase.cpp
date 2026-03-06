// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullets/BulletsBase.h"

#include "AIController.h"
#include "Bullets/PacBulletsComponent.h"
#include "Character/PacCharacter.h"
#include "Character/PacHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Physics/PacCollisionChannels.h"
#include "Utility/PacLogMacro.h"
#include "Weapon/PacWeapon.h"

static float DrawBulletTracesDuration = 0.0f;
static FAutoConsoleVariableRef CVarDrawBulletTraceDuraton(
		TEXT("Pac.Shoot.DrawBulletTraceDuration"),
		DrawBulletTracesDuration,
		TEXT("Should we do debug drawing for bullet traces (if above zero, sets how long (in seconds))"),
		ECVF_Default);

static float DrawBulletHitDuration = 0.0f;
static FAutoConsoleVariableRef CVarDrawBulletHits(
	TEXT("Pac.Shoot.DrawBulletHitDuration"),
	DrawBulletHitDuration,
	TEXT("Should we do debug drawing for bullet impacts (if above zero, sets how long (in seconds))"),
	ECVF_Default);

static float DrawBulletHitRadius = 3.0f;
static FAutoConsoleVariableRef CVarDrawBulletHitRadius(
	TEXT("Pac.Shoot.DrawBulletHitRadius"),
	DrawBulletHitRadius,
	TEXT("When bullet hit debug drawing is enabled (see DrawBulletHitDuration), how big should the hit radius be? (in uu)"),
	ECVF_Default);

static float DebugBullet = 0.0f;
static FAutoConsoleVariableRef CVarDebugAbility(
		TEXT("Pac.Shoot.DebugBulletActivation"),
		DebugBullet,
		TEXT("Should we Log on Screen the Bullet Activation and Deactivation (if > 0, sets how long (in seconds))"),
		ECVF_Cheat);

static float DebugActorInRange = 0.0f;
static FAutoConsoleVariableRef CVarDebugActorInRange(
		TEXT("Pac.Shoot.DrawActorInRange"),
		DebugActorInRange,
		TEXT("Should we Draw a Debug Sphere When there is a Zone Create By the Bullet (if > 0, sets how long (in seconds))"),
		ECVF_Cheat);


UBulletsBase::UBulletsBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	auto ImplementedInBlueprint = [](const UFunction* Func) -> bool
	{
		return Func && ensure(Func->GetOuter())
			&& Func->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass());
	};
	{
		static FName FuncName = FName(TEXT("K2_ActivateBullet"));
		UFunction* ActivateFunction = GetClass()->FindFunctionByName(FuncName);
		bHasBlueprintActivate = ImplementedInBlueprint(ActivateFunction);
	}
	{
		static FName FuncName = FName(TEXT("K2_OnEnBullet"));
		UFunction* ActivateFunction = GetClass()->FindFunctionByName(FuncName);
		bHasBlueprintOnEnd = ImplementedInBlueprint(ActivateFunction);
	}
}

void UBulletsBase::CallActivateBullet()
{
	PreActivateBullet();
	ActivateBullet();
}

void UBulletsBase::CancelBullet()
{
	EndBullet(true);
}

void UBulletsBase::K2_CancelBullet()
{
	CancelBullet();
}

void UBulletsBase::EndBullet(bool bWasCancelled)
{
	if (DebugBullet > 0) {
		LOG_SCREEN(-1, DebugBullet , FColor::Green,"End Bullet: %s, WasCancel: %s", *GetName(), bWasCancelled ? TEXT("True") : TEXT("False"));
	}
	
	if (bHasBlueprintOnEnd) {
		K2_OnEnBullet(bWasCancelled);
	}

	if (BulletsComponent) {
		BulletsComponent->ActivatedBulletsInstances.Remove(this);
	}

	MarkAsGarbage();
}

void UBulletsBase::K2_EndBullet()
{
	EndBullet(false);
}

void UBulletsBase::PreActivateBullet()
{
}

void UBulletsBase::ActivateBullet()
{
	if (DebugBullet > 0) {
		LOG_SCREEN(-1, DebugBullet , FColor::Green,"Activate Bullet: %s", *GetName());
	}
	
	if (bHasBlueprintActivate) {
		K2_ActivateBullet();
	}

	if (FireSoundClue) {
		UGameplayStatics::PlaySound2D(GetWorld(), FireSoundClue, 1, 1, 0, nullptr, nullptr, false);
	}

	FHitResult Impact = StartTrageting();

	if (Impact.bBlockingHit) {
#if ENABLE_DRAW_DEBUG
		if (DrawBulletHitDuration > 0.0f) {
			DrawDebugPoint(GetWorld(), Impact.ImpactPoint, DrawBulletHitRadius, FColor::Green, false, DrawBulletHitDuration);
		}
#endif
		check(BulletsComponent);
		BulletsComponent->OnBulletHit.Broadcast(Impact, GetClass());
	}
	
	OnTargetFound(Impact);
	if (AutoEnd) {
		EndBullet(false);
	}
}

FHitResult UBulletsBase::StartTrageting()
{
	ACharacter* const AvatarCharacter = GetAvatarCharacter();
	
	if (AvatarCharacter) {
		const FTransform TargetTransform  = GetTargetingTransform(AvatarCharacter);
		const FVector StartTrace = TargetTransform.GetTranslation();
		const FVector Direction = TargetTransform.GetUnitAxis(EAxis::X);
		const FVector EndTrace = StartTrace + Direction * TraceDistance;

		TArray<FHitResult> HitResults;
		
		FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), /*bTraceComplex=*/ true, /*IgnoreActor=*/ AvatarCharacter);
		TraceParams.bReturnPhysicalMaterial = true;

		// Ignore any actors attached to the avatar doing the shooting !
		TArray<AActor*> AttachedActors;
		AvatarCharacter->GetAttachedActors(/*out*/ AttachedActors);
		TraceParams.AddIgnoredActors(AttachedActors);

#if ENABLE_DRAW_DEBUG
		if (DrawBulletTracesDuration > 0.0f) {
			static float DebugThickness = 1.0f;
			DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Red, false, DrawBulletTracesDuration, 0, DebugThickness);
		}
#endif // ENABLE_DRAW_DEBUG
		
		const ECollisionChannel TraceChannel = DetermineTraceChannel();
		
		// Trace and process instant hit if something was hit
		// First trace without using sweep radius
		if (FindFirstDamageableHitResult(HitResults) == INDEX_NONE) {
			GetWorld()->LineTraceMultiByChannel(HitResults, StartTrace, EndTrace, TraceChannel, TraceParams);
		}

		if (FindFirstDamageableHitResult(HitResults) == INDEX_NONE) {
			
			// If this Bullet didn't hit anything with a line trace and supports a sweep radius, try that 
			if (SweepRadius > 0.0f) {
				TArray<FHitResult> SweepHits;
				GetWorld()->SweepMultiByChannel(SweepHits, StartTrace, EndTrace, FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(SweepRadius), TraceParams);
				
				// If the trace with sweep radius enabled hit a Damageable object, check if we should use its hit results
				const int32 FirstPawnIdx = FindFirstDamageableHitResult(SweepHits);
				if (SweepHits.IsValidIndex(FirstPawnIdx))
				{
					// If we had a blocking hit in our line trace that occurs in SweepHits before our
					// hit pawn, we should just use our initial hit results since the Pawn hit should be blocked
					bool bUseSweepHits = true;
					for (int32 Idx = 0; Idx < FirstPawnIdx; ++Idx)
					{
						if (HitResults.Num() == 0) {
							break;
						}
						
						const FHitResult& CurHitResult = HitResults[Idx];

						auto Pred = [&CurHitResult](const FHitResult& Other)
						{
							return Other.HitObjectHandle == CurHitResult.HitObjectHandle;
						};
						
						if (CurHitResult.bBlockingHit && HitResults.ContainsByPredicate(Pred)) {
							bUseSweepHits = false;
							break;
						}
					}
					

					if (bUseSweepHits) {
						HitResults = SweepHits;
					}
				}
			}
		}

		FHitResult Hit(ForceInit);
		if (HitResults.Num() > 0) {
			Hit = HitResults.Last();
		} else {
			Hit.TraceStart = StartTrace;
			Hit.TraceEnd = EndTrace;
		}

		return Hit;
	}

	PAC_ERROR("%s, Avatar Pawn is null", *GetName());
	return FHitResult(ForceInit);
}

FTransform UBulletsBase::GetTargetingTransform(ACharacter* SourceCharacter) const
{
	AController* SourcePawnController = SourceCharacter->GetController();
	FVector CamLoc;
	FRotator CamRot;

	if (TargetingSource == EPacBulletTargetingSource::CameraTowardsFocus) {
		APlayerController* PC = Cast<APlayerController>(SourcePawnController);
		
		if (PC != nullptr) {
			PC->GetPlayerViewPoint(/*out*/ CamLoc, /*out*/ CamRot);
		} else if (AAIController* AIController = Cast<AAIController>(SourcePawnController)) {
			CamLoc = SourceCharacter->GetActorLocation() + FVector(0, 0, SourceCharacter->BaseEyeHeight);
			CamRot = SourcePawnController->GetControlRotation();
		}

		return FTransform(CamRot, CamLoc);
	}
	
	if (TargetingSource == EPacBulletTargetingSource::PawnForward) {
		return SourceCharacter->GetTransform();
	}

	if (TargetingSource == EPacBulletTargetingSource::WeaponSocket) {
		if (BulletsComponent) {
			return GetAvatarCharacter()->GetFeedBackShootTransForm();
		}
	}

	if (TargetingSource == EPacBulletTargetingSource::FeedBackSocket) {
		if (BulletsComponent) {
			return GetAvatarCharacter()->GetFeedBackShootTransForm();
		}
	}
	
	PAC_ERROR("%s, Invalid Targeting Source %s", *GetName(), *UEnum::GetValueAsString(TargetingSource));
	return FTransform(); // Default case, return an identity transform
}

int32 UBulletsBase::FindFirstDamageableHitResult(const TArray<FHitResult>& HitResults)
{
	for (int32 Idx = 0; Idx < HitResults.Num(); ++Idx) {
		const FHitResult& CurHitResult = HitResults[Idx];
		if (CurHitResult.HitObjectHandle.DoesRepresentClass(APawn::StaticClass())) {
			// If we hit a pawn, we're good
			return Idx;
		}
		
		AActor* HitActor = CurHitResult.HitObjectHandle.FetchActor();
		if ((HitActor != nullptr) && (HitActor->GetAttachParentActor() != nullptr) && (Cast<APawn>(HitActor->GetAttachParentActor()) != nullptr)) {
			// If we hit something attached to a pawn, we're good
			return Idx;
		}

		if ((HitActor != nullptr) && HitActor->FindComponentByClass<UPacHealthComponent>()) {
			// If we hit an actor with a health component, we're good
			return Idx;
		}
	}

	return INDEX_NONE;
}

ECollisionChannel UBulletsBase::DetermineTraceChannel() const
{
	return PAC_TRACE_CHANNEL_Ability;
}

APacCharacter* UBulletsBase::GetAvatarCharacter() const
{
	if (BulletsComponent) {
		return Cast<APacCharacter>(BulletsComponent->GetOwner());
	}
	
	return nullptr;
}
