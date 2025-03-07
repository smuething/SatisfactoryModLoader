// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "FactoryGame.h"
#include "Buildables/FGBuildableFactory.h"
#include "Replication/FGReplicationDetailInventoryComponent.h"
#include "Replication/FGReplicationDetailActor_Storage.h"
#include "FGBuildableStorage.generated.h"

/**
 * Base class for all storage boxes, large and small. May have an arbitrary number of inputs and outputs.
 */
UCLASS( Abstract )
class FACTORYGAME_API AFGBuildableStorage : public AFGBuildableFactory
{
	GENERATED_BODY()
public:
	/** Decide on what properties to replicate */
	virtual void GetLifetimeReplicatedProps( TArray< FLifetimeProperty >& OutLifetimeProps ) const override;

	/** Constructor */
	AFGBuildableStorage();

	// Begin AActor interface
	virtual void BeginPlay() override;
	// End AActor interface

	//~ Begin IFGDismantleInterface
	virtual void GetDismantleRefund_Implementation( TArray< FInventoryStack >& out_refund, bool noBuildCostEnabled ) const override;
	//~ End IFGDismantleInterface

	// Begin IFGReplicationDetailActorOwnerInterface
	virtual UClass* GetReplicationDetailActorClass() const override { return AFGReplicationDetailActor_Storage::StaticClass(); };
	virtual void OnReplicationDetailActorRemoved() override;
	// End IFGReplicationDetailActorOwnerInterface

	/** Get the storage inventory from this storage box. */
	UFUNCTION( BlueprintPure, Category = "Inventory" )
	FORCEINLINE class UFGInventoryComponent* GetStorageInventory() { return mStorageInventoryHandlerData.GetActiveInventoryComponent(); }

	/** Get the initial (non-repdetail) inventory. This is only available on Server */
	FORCEINLINE class UFGInventoryComponent* GetInitialStorageInventory() { return mStorageInventory; }

protected:
	virtual void GetAllReplicationDetailDataMembers(TArray<FReplicationDetailData*>& out_repDetailData) override
	{
		Super::GetAllReplicationDetailDataMembers( out_repDetailData );
		out_repDetailData.Add( &mStorageInventoryHandlerData );
	}

protected:
	friend class AFGReplicationDetailActor_Storage;

#if WITH_EDITOR // Virtual should only exist in editor builds since its used for testing item throughput
	virtual void Factory_Tick(float dt) override;
#endif
	
	// Begin Factory_ interface
	virtual void Factory_CollectInput_Implementation() override;
	// End Factory_ interface

	// Begin FGBuildableFactory interface
	virtual void OnRep_ReplicationDetailActor() override;
	// End FGBuildableFactory interface

public:
	/** How far apart in Z do multiple storages stack. */
	UPROPERTY( EditDefaultsOnly, Category = "Storage" )
	float mStackingHeight;

	/** Default resources in a storage @todo Why this special case here, add the stuff in blueprint instead first time we're being built... I guess this is only used for the tutorial? */
	UPROPERTY( EditDefaultsOnly, Category = "Inventory", Meta = (NoAutoJson = true) )
	TArray< FItemAmount > mDefaultResources;

	/** The size of the inventory for this storage. */
	UPROPERTY( EditDefaultsOnly, Category = "Inventory" )
	int32 mInventorySizeX;
	
	/** The size of the inventory for this storage. */
	UPROPERTY( EditDefaultsOnly, Category = "Inventory" )
	int32 mInventorySizeY;

private:
	//UPROPERTY( Replicated, Transient, ReplicatedUsing = OnRep_ReplicationDetailActor )
	//class AFGReplicationDetailActor_Storage* mReplicationDetailActor;

	/** The inventory to store everything in. Don't use this directly, use mStorageInventoryHandler->GetActiveInventoryComponent() */
	UPROPERTY( SaveGame )
	class UFGInventoryComponent* mStorageInventory;

	/** Maintainer of the active storage component for this actor. Use this to get the active inventory component. */
	FReplicationDetailData mStorageInventoryHandlerData;
	
	/** Cached input connections (No need for UPROPERTY as they are referenced in component array) */
	TArray<class UFGFactoryConnectionComponent*> mCachedInputConnections;

#if WITH_EDITOR
	bool mHasResolvedBeltSpeed = 0.f;
	TArray< float > mSpeedOfIncomingBelts;
	float mTimeStartThroughputCheck = 0.f;
	int32 mNumItemsThroughputCheck = 0;
#endif

private:
	class AFGReplicationDetailActor_Storage* GetCastRepDetailsActor() const { return Cast<AFGReplicationDetailActor_Storage>( mReplicationDetailActor ); };
};
