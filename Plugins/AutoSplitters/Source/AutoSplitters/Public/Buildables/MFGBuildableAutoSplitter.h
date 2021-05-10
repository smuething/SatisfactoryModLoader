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

	virtual void FillDistributionTable(float dt) override;

private:

	void SetupDistribution(bool LoadingSave = false);
	void PrepareCycle();

public:

	UPROPERTY(SaveGame, EditDefaultsOnly, BlueprintReadOnly, Meta = (NoAutoJson))
	TArray<float> mOutputRates;

	UPROPERTY(SaveGame, EditDefaultsOnly, BlueprintReadOnly, Meta = (NoAutoJson))
	TArray<int32> mOutputStates;

	UPROPERTY(SaveGame, EditDefaultsOnly, BlueprintReadOnly, Meta = (NoAutoJson))
	TArray<float> mRemainingOutputPriority;

	UPROPERTY(SaveGame, EditDefaultsOnly, BlueprintReadOnly, Meta = (NoAutoJson))
	int32 mLeftInCycle;

	UPROPERTY(Transient,BlueprintReadWrite)
	bool mDebug;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<float> mPriorityStepSize;

	UPROPERTY(Transient,BlueprintReadOnly)
	int32 mCycleLength;

	UFUNCTION(BlueprintCallable)
	bool SetOutputRate(int32 Output, float Rate);

	UFUNCTION(BlueprintCallable)
	bool SetOutputAutomatic(int32 Output, bool automatic);

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

private:
	static AMFGBuildableAutoSplitter*
	FindAutoSplitterAfterBelt(UFGFactoryConnectionComponent* Connection, bool Forward);

	static void DiscoverHierarchy(TArray<TArray<FConnections>>& Splitters, AMFGBuildableAutoSplitter* Splitter,
	                              const int32 Level);

	std::array<int8,3> mJammedFor;

	float mEpsilon;

	bool mBalancingRequired;

};

