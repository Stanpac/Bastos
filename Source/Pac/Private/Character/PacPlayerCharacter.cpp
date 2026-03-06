// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PacPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "NiagaraComponent.h"
#include "Bullets/PacBulletsComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/PacCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameModes/PacGameInstance.h"

APacPlayerCharacter::APacPlayerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Camera Component
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(GetCapsuleComponent());
	CameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f));
	CameraComponent->bUsePawnControlRotation = true;

	FirsPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMesh"));
	FirsPersonMesh->SetOnlyOwnerSee(true);
	FirsPersonMesh->bCastDynamicShadow = false;
	FirsPersonMesh->CastShadow = false;
	FirsPersonMesh->SetupAttachment(CameraComponent);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	CharacterType = EPacCharacterType::Player;
}

void APacPlayerCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller)){
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

// Called to bind functionality to input
void APacPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APacCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &APacCharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APacCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APacCharacter::Look);

		// Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &APacCharacter::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &APacCharacter::StopSprint);

		// Shooting
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &APacCharacter::Fire);

		// Reloading
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &APacCharacter::Reload);
	}
}

void APacPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	FOnGameplayEffectTagCountChanged& CoyoteTimeTagCountChangedDelegate = ActiveTags.RegisterGameplayTagEvent(PacGameplayTags::TAG_Gameplay_CoyoteTime, EGameplayTagEventType::NewOrRemoved);
	CoyoteTimeTagCountChangedDelegate.AddUObject(this , &APacPlayerCharacter::CheckCoyoteTimeTagCountChanged);

	if (UPacGameInstance* GameInstance = Cast<UPacGameInstance>(GetWorld()->GetGameInstance())) {
		if (GameInstance->GetSaveBullets().Num() > 0) {
			BulletsComponent->SetNewBullets(GameInstance->GetSaveBullets());
		}
	}
}

void APacPlayerCharacter::CheckCoyoteTimeTagCountChanged(FGameplayTag GameplayTag, int TagCount)
{
	if (TagCount > 0) {
		// If the tag is added, we can increase the JumpMaxCount
		JumpMaxCount++;
	} else {
		// If the tag is removed, we can decrease the JumpMaxCount
		JumpMaxCount--;
	}
}

