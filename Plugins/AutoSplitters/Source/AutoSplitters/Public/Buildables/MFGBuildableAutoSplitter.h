// ILikeBanas

#pragma once

#include "CoreMinimal.h"

#include <array>

#include "FGFactoryConnectionComponent.h"
#include "Buildables/FGBuildableAttachmentSplitter.h"
#include "Buildables/FGBuildableConveyorBase.h"

#include "MFGBuildableAutoSplitter.generated.h"

UENUM(BlueprintType, Meta = (BitFlags))
enum class EOutputState : uint8
{
	Automatic UMETA(DisplayName = "Automatic"),
	Connected UMETA(DisplayName = "Connected"),
	AutoSplitter UMETA(DisplayName = "AutoSplitter")
};

constexpr int32 Flag(EOutputState flag)
{
	return 1 << static_cast<int32>(flag);
}

constexpr bool IsSet(int32 BitField, EOutputState flag)
{
	return BitField & Flag(flag);
}

constexpr int32 SetFlag(int32 BitField, EOutputState flag)
{
	return BitField | Flag(flag);
}

constexpr int32 ClearFlag(int32 BitField, EOutputState flag)
{
	return BitField & ~Flag(flag);
}

constexpr int32 SetFlag(int32 BitField, EOutputState flag, bool Enabled)
{
	if (Enabled)
		return SetFlag(BitField,flag);
	else
		return ClearFlag(BitField,flag);
}

/**
 * 
 */
UCLASS()
class AUTOSPLITTERS_API AMFGBuildableAutoSplitter : public AFGBuildableAttachmentSplitter
{
public:
	virtual void PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override;
private:
	GENERATED_BODY()

	virtual void BeginPlay() override;

	AMFGBuildableAutoSplitter();
	
protected:

	virtual void Factory_Tick(float dt) override;
	virtual bool Factory_GrabOutput_Implementation( UFGFactoryConnectionComponent* connection, FInventoryItem& out_item, float& out_OffsetBeyond, TSubclassOf< UFGItemDescriptor > type ) override;	
	virtual void FillDistributionTable(float dt) override;

private:

	void SetupDistribution(bool LoadingSave = false);
	void PrepareCycle(bool AllowCycleExtension, bool Reset = false);

public:

	static constexpr int32 MAX_INVENTORY_SIZE = 16;
	static constexpr int32 MAX_ASSIGNMENTS_PER_OUTPUT = 16;
	static constexpr float EXPONENTIAL_AVERAGE_WEIGHT = 0.75f;

	UPROPERTY(SaveGame, EditDefaultsOnly, BlueprintReadOnly, Meta = (NoAutoJson))
	TArray<float> mOutputRates;

	UPROPERTY(SaveGame, EditDefaultsOnly, BlueprintReadOnly, Meta = (NoAutoJson))
	TArray<int32> mOutputStates;

	UPROPERTY(SaveGame, BlueprintReadOnly, Meta = (NoAutoJson))
	TArray<int32> mRemainingItems;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<int32> mItemsPerCycle;

	UPROPERTY(Transient, BlueprintReadOnly)
	int32 mLeftInCycle;

	UPROPERTY(Transient,BlueprintReadWrite)
	bool mDebug;

	UPROPERTY(Transient,BlueprintReadOnly)
	int32 mCycleLength;

	UFUNCTION(BlueprintCallable)
	bool SetOutputRate(int32 Output, float Rate);

	UFUNCTION(BlueprintCallable)
	bool SetOutputAutomatic(int32 Output, bool Automatic);

	UFUNCTION(BlueprintCallable)
	int32 BalanceNetwork(bool RootOnly = false);

	struct FConnections
	{
		AMFGBuildableAutoSplitter* Splitter;
		std::array<AMFGBuildableAutoSplitter*,3> Outputs;

		explicit FConnections(AMFGBuildableAutoSplitter* Splitter)
			: Splitter(Splitter)
		    , Outputs({nullptr})
		{}
	};

	UFUNCTION(BlueprintCallable,BlueprintPure)
	int32 GetInventorySize() const
	{
		return mCachedInventoryItemCount;
	}

	UFUNCTION(BlueprintCallable,BlueprintPure)
	float GetItemRate() const
	{
		return mItemRate;
	}
	

private:
	static AMFGBuildableAutoSplitter*
	FindAutoSplitterAfterBelt(UFGFactoryConnectionComponent* Connection, bool Forward);

	static void DiscoverHierarchy(TArray<TArray<FConnections>>& Splitters, AMFGBuildableAutoSplitter* Splitter,
	                              const int32 Level);

	std::array<int8,3> mJammedFor;
	std::array<int8,3> mAllowedItems;
	std::array<int8,3> mGrabbedItems;
	std::array<float,3> mPriorityStepSize;
	std::array<std::array<int32,MAX_ASSIGNMENTS_PER_OUTPUT>,3> mAssignedInventorySlots;

	bool mBalancingRequired;
	int32 mCachedInventoryItemCount;
	float mItemRate;
	float mCycleTime;
	int32 mReallyGrabbed;
};

