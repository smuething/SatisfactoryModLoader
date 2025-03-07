// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "FactoryGame.h"
#include "Equipment/FGEquipment.h"
#include "FGInventoryComponent.h"
#include "Equipment/FGEquipmentAttachment.h"
#include "FGFoliageRemovalSubsystem.h"
#include "FGChainsaw.generated.h"


class IFGChainsawableInterface;

USTRUCT()
struct FACTORYGAME_API FPickedUpInstance
{
	GENERATED_BODY()

	FPickedUpInstance() :
		InstanceMesh( NULL ),
		Location( FVector::ZeroVector )
	{
	}

	FPickedUpInstance( UStaticMesh* instanceMesh, FVector location ) :
		InstanceMesh( instanceMesh ),
		Location( location )
	{
	}

	UPROPERTY()
	class UStaticMesh* InstanceMesh;

	UPROPERTY()
	FVector Location;
};

UCLASS()
class FACTORYGAME_API AFGChainsaw : public AFGEquipment
{
	GENERATED_BODY()
public:
	// Ctor
	AFGChainsaw();

	//~ Begin AActor interface
	virtual void Tick( float dt ) override;
	//~ End AActor interface

	// Begin AFGEquipment interface
	virtual bool ShouldSaveState() const override;
	virtual void DisableEquipment() override;
	virtual void UnEquip() override;
	// End

	// Replication
	virtual void GetLifetimeReplicatedProps( TArray< FLifetimeProperty >& OutLifetimeProps ) const override;

	/** @return true if we are sawing */
	UFUNCTION( BlueprintPure, Category="Chainsaw" )
	FORCEINLINE bool IsSawEngaged() const{ return mIsSawing; }

	/** @return true if we are sawing AND we have a valid saw component */
	UFUNCTION( BlueprintPure, Category = "Chainsaw" )
	FORCEINLINE bool IsSawing() const {  return mIsSawing && IsValidSawing( mSawingComponent, mSawingInstance ); }


	/** In percent, how long into our progress have we gone into sawing down our current tree */
	UFUNCTION( BlueprintPure, Category = "Chainsaw" )
	FORCEINLINE float SawProgress() const{ return mSawingProgress / mSawDownTreeTime; }

	/** returns true if the specified player has a chainsaw equipped */
	static bool DoesPlayerHaveChainsawEquipped( class AFGCharacterPlayer* player );

	UFUNCTION( BlueprintImplementableEvent, Category = "Chainsaw" )
	void CreatePhysicsFromFoliage( UStaticMesh* inMesh, FTransform inTransform );
protected:
	virtual void HandleDefaultEquipmentActionEvent( EDefaultEquipmentAction action, EDefaultEquipmentActionEvent actionEvent ) override;
	
	/**
	 * Consumes fuel, returns false if we are out of fuel
	 */
	bool ConsumeFuel( float dt );

	/**
	 * Start sawing, does no logic check if it's valid, caller is assumed to do so
	 * If client, calls server
	 */
	void StartSawing();

	UFUNCTION( BlueprintNativeEvent, Category = "Chainsaw" )
	bool CanStartSawing();

	/**
	 * Calls StartSawing on server
	 */
	UFUNCTION( Reliable, Server, WithValidation )
	void Server_StartSawing();

	/**
	 * Stops sawing, does no logic check if it's valid, caller is assumed to do so
	 * If client, calls server
	 */
	void StopSawing();

	/** 
	 * Removes the foliage we just cut down
	 */
	UFUNCTION( Reliable, Server )
	void Server_RemoveFoliageInstance( const struct FFoliageInstanceStableId& stableId, const FVector& effectLocation, const FTransform& instanceTransform );

	void RemoveFoliageInstance( const struct FFoliageInstanceStableId& stableId, const FVector& effectLocation, const FTransform& instanceTransform );

	UFUNCTION( Reliable, Server )
	void Server_RemoveChainsawableObject(const TScriptInterface<IFGChainsawableInterface> &chainsawableObject);
	void RemoveChainsawableObject(TScriptInterface<IFGChainsawableInterface> chainsawableObject);

	/**
	 * Removes surrounding foliage around the chainsawedObject and picks it up the within the Collateral pick-up radius
	*/
	void RemoveCollateralFoliage( class AFGFoliageRemovalSubsystem* removalSubsystem, const FVector& location );

	UFUNCTION( NetMulticast, Unreliable )
	void BroadcastPickup( const TArray<FPickedUpInstance>& pickups, class AFGFoliagePickup* instigatorPlayer );

	/**
	 * Calls StopSawing on server
	 */
	UFUNCTION( Reliable, Server, WithValidation )
	void Server_StopSawing();

	/** Return true we have any energy stored or if our owner has any fuel */
	UFUNCTION( BlueprintPure,  Category = "Chainsaw" )
	bool HasAnyFuel() const;

	/** Returns the current fuel class used for the chainsaw */
	UFUNCTION(BlueprintPure, Category = "Chainsaw")
	FORCEINLINE TSubclassOf<class UFGItemDescriptor> GetFuelClass() { return mFuelClass; }

	/** Start sawing on a new tree */
	void StartNewSawing( class USceneComponent* sawingComponent, int32 newIndex );

	/** Is it valid to saw on this component.
	 * @param newIndex: must be set to the corresponding index if sawingComponent is a UHierarchicalInstancedStaticMeshComponent
	 */
	bool IsValidSawing( class USceneComponent* sawingComponent, int32 newIndex ) const;
	
	/** Add foliage to player inventory from the component  */
	void AddToPlayerInventory( class USceneComponent* sawingComponent );

	bool CanPlayerPickupFoliageResourceForSeeds( class UHierarchicalInstancedStaticMeshComponent* meshComponent, bool excludeChainsawable, TArrayView< uint32 > seeds, TArray<FInventoryStack>& out_validStacks );

	/** Play pickup effect */
	void PlayEffect( FVector atLocation, USceneComponent* sawingComponent );

	///** Hides the outline. Convenience function */
	//void HideOutline();

	/** returns the static mesh of whatever the hell it is this is i hate this */
	UStaticMesh* GetStaticMesh( USceneComponent* sawingComponent );

	/** returns true if actor is a chainsawable actor, duh */
	bool IsChainsawableObject(UObject* object) const;
	
protected:
	/** The fuel we want to be able to use with the chainsaw */
	UPROPERTY( EditDefaultsOnly, Category="Chainsaw|Fuel" )
	TSubclassOf<class UFGItemDescriptor> mFuelClass;

	/** How much energy the chainsaw consumes. In megawatt seconds (MWs) */
	UPROPERTY( EditDefaultsOnly, Category = "Chainsaw|Fuel" )
	float mEnergyConsumption;

	/** How many seconds should it take to saw down a tree */
	UPROPERTY( EditDefaultsOnly, Category = "Chainsaw" )
	float mSawDownTreeTime;

	/** How large radius of automatic pick up of foliage is when using chainsaw */
	UPROPERTY( EditDefaultsOnly, Category = "Chainsaw|Collateral" )
	float mCollateralPickupRadius;

	/** If collateral pickups should exclude chainsawable foliage when using chainsaw */
	UPROPERTY( EditDefaultsOnly, Category = "Chainsaw|Collateral" )
	bool mExcludeChainsawableFoliage;

	/** The noise to make when using the chainsaw. */
	UPROPERTY( EditDefaultsOnly, Category = "Chainsaw" )
	TSubclassOf< class UFGNoise > mChainsawNoise;

	/**
	 * How much energy do we have stored left in the chainsaw (when we consume fuel from owners inventory
	 * then this is the energy stored here)
	 **/
	UPROPERTY( SaveGame, Replicated )
	float mEnergyStored;

	/** How much progress we have done when sawing on a tree */
	float mSawingProgress;

	/** Instance we are currently sawing on */
	int32 mSawingInstance;

	/** Component we are currently sawing on */
	class USceneComponent* mSawingComponent;

	/** if true, then we are using the chainsaw */
	uint8 mIsSawing:1;

	/** if true, then we are spinning the chainsaw up */
	uint8 mIsSpinningUp:1;
};

UCLASS()
class FACTORYGAME_API AFGChainsawAttachment : public AFGEquipmentAttachment
{
	GENERATED_BODY()
	
public:
	//~ Begin AActor interface
	virtual void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override;
	//~ End AActor interface
	
	/** Return true we have any energy stored or if our owner has any fuel */
	UFUNCTION( BlueprintPure, Category = "Chainsaw" )
	FORCEINLINE bool HasAnyFuel() const { return mHasAnyFuel; }
	
	UFUNCTION( BlueprintImplementableEvent )
	void OnHasAnyFuelUpdated( bool hasAnyFuel );

	UFUNCTION()
	void SetHasAnyFuel( bool newHasAnyFuel );

protected:
	UFUNCTION()
	void OnRep_HasAnyFuel();
	
	UPROPERTY( ReplicatedUsing = OnRep_HasAnyFuel )
	bool mHasAnyFuel;
	
};

