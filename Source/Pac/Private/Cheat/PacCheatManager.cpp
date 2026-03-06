// Fill out your copyright notice in the Description page of Project Settings.


#include "Cheat/PacCheatManager.h"

#include "Character/PacHealthComponent.h"
#include "Character/PacPlayerCharacter.h"
#include "Utility/PacLogMacro.h"

DEFINE_LOG_CATEGORY(PacCheatLog);

UPacCheatManager::UPacCheatManager(const FObjectInitializer& ObjectInitializer)
{
}

void UPacCheatManager::CheatPlayerInvincibility()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetOuter())) {
		if (APacPlayerCharacter* PacCharacter = Cast<APacPlayerCharacter>(PlayerController->GetPawn())) {
			if (PacCharacter->HasMatchingGameplayTag(PacGameplayTags::TAG_Cheat_GodMode)) {
				PacCharacter->UpdateTagCount(PacGameplayTags::TAG_Cheat_GodMode, -10);
				PAC_LOG_CATEGORY(PacCheatLog, "PacManager: Player invincibility disabled.");
			} else {
				PacCharacter->UpdateTagCount(PacGameplayTags::TAG_Cheat_GodMode, 1);
				PAC_LOG_CATEGORY(PacCheatLog, "PacManager: Player invincibility enabled.");
			}
		} else {
			PAC_WARNING_CATEGORY(PacCheatLog, "PacManager: Player character not found or not a PacPlayerCharacter.");
		}
	} else {
		PAC_WARNING_CATEGORY(PacCheatLog, "PacManager: PlayerController not found.");
	}
}

 


