// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Utility/PacGameplayTags.h"
#include "GameplayTagCountContainerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UGameplayTagCountContainerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *  Interface for Object that Posses a FGameplayTagCountContainer
 *  Provide BP function to access the GameplayTagCountContainer
 */
class PAC_API IGameplayTagCountContainerInterface
{
	GENERATED_BODY()

public:
	/** NEED TO BE IMPLEMENTED
	 * Update the specified container of tags by the specified delta, potentially causing an additional or removal from the explicit tag list
	 * 
	 * @param Container		Container of tags to update
	 * @param CountDelta	Delta of the tag count to apply
	 */
	UFUNCTION(BlueprintCallable, Category = GameplayTags)
	virtual void UpdateTagsCount(const FGameplayTagContainer Container, int32 CountDelta) = 0;
	
	/** NEED TO BE IMPLEMENTED
	 *  Update the specified tag by the specified delta, potentially causing an additional or removal from the explicit tag list
	 *  
	 * @param Container		Container of tags to update
	 * @param CountDelta	Delta of the tag count to apply
	 */
	UFUNCTION(BlueprintCallable, Category = GameplayTags)
	virtual void UpdateTagCount(const FGameplayTag Tag, int32 CountDelta) = 0;

	/** NEED TO BE IMPLEMENTED
	 * Set the specified tag count to a specific value
	 * 
	 * @param Tag			Tag to update
	 * @param Count			New count of the tag
	 * 
	 * @return True if tag was *either* added or removed. (E.g., we had the tag and now dont. or didnt have the tag and now we do. We didn't just change the count (1 count -> 2 count would return false).
	 */
	UFUNCTION(BlueprintCallable, Category = GameplayTags)
	virtual bool SetTagCount(const FGameplayTag Tag, int32 NewCount) = 0;

	/** NEED TO BE IMPLEMENTED
	* return the hierarchical count for a specified tag
	* e.g. if A.B & A.C were added, GetTagCount("A") would return 2.
	*
	* @param Tag			Tag to update
	*
	* @return the count of the passed in tag
	*/
	UFUNCTION(BlueprintCallable, Category = GameplayTags)
	virtual int32 GetTagCount(const FGameplayTag Tag) const = 0;

	/** NEED TO BE IMPLEMENTED
	 * Add a temporary tag with a specified duration and count.
	 * @param Tag			The tag to add
	 * @param Duration		The duration for which the tag should be active
	 * @param Count			The count of the tag to add (default is 1)
	 */
	UFUNCTION(BlueprintCallable, Category = GameplayTags)
	virtual void AddTemporaryTag(const FGameplayTag Tag, float Duration, int Count = 1) = 0;
	
	/** NEED TO BE IMPLEMENTED
	 * Remove a temporary tag with a specified duration and count.
	 * @param Tag			The tag to remove
	 * @param Duration 		The duration for which the tag should be removed 
	 * @param Count			The count of the tag to remove (default is 1)
	 */
	UFUNCTION(BlueprintCallable, Category = GameplayTags)
	virtual void RemoveTemporaryTag(const FGameplayTag Tag, float Duration, int Count = 1) = 0;
	
};
 