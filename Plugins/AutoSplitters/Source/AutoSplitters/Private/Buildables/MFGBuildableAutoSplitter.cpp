// ILikeBanas

#include <numeric>

#include "Buildables/MFGBuildableAutoSplitter.h"

#include "AutoSplittersLog.h"
#include "FGFactoryConnectionComponent.h"
#include "Buildables/FGBuildableConveyorBase.h"

AMFGBuildableAutoSplitter::AMFGBuildableAutoSplitter()
	: mOutputRates({1.0,1.0,1.0})
	, mOutputStates({Flag(EOutputState::Automatic),Flag(EOutputState::Automatic),Flag(EOutputState::Automatic)})
	, mRemainingOutputPriority({0.0,0.0,0.0})
	, mLeftInCycle(0)
	, mPriorityStepSize({0.0,0.0,0.0})
	, mCycleLength(0)
	, mJammedFor({0,0,0})
	, mEpsilon(1e-6)
	, mBalancingRequired(true)
{}

void AMFGBuildableAutoSplitter::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion)
{
	Super::PostLoadGame_Implementation(saveVersion,gameVersion);
	SetupDistribution(true);
}

void AMFGBuildableAutoSplitter::BeginPlay()
{
	Super::BeginPlay();
	mBalancingRequired = true;
}

void AMFGBuildableAutoSplitter::FillDistributionTable(float dt)
{	
	if (!HasAuthority())
	{
		UE_LOG(LogAutoSplitters,Display,TEXT("NO AUTHORITY, ABORTING"));
		return;
	}

	if (mBalancingRequired)
	{
		BalanceNetwork(true);
	}

	int32 Connections = 0;
	bool NeedsBalancing = false;
	for (int32 i = 0 ; i < 3 ; ++i)
	{
		const bool Connected = IsSet(mOutputStates[i],EOutputState::Connected);
		Connections += Connected;
		if (Connected != mOutputs[i]->IsConnected())
		{
			NeedsBalancing = true;
		}
	}

	if (NeedsBalancing)
		BalanceNetwork();
	
	if (Connections == 0 || mBufferInventory->IsEmpty())
	{
		return;
	}
	
	if (mLeftInCycle <= 0)
		PrepareCycle();
	else
	{
		for (int32 i = 0 ; i < 3 ; ++i)
		{
			if (IsSet(mOutputStates[i],EOutputState::Connected))
			{
				if (mRemainingOutputPriority[i] == -100000.0)
				{
					PrepareCycle();
				}
			}
			else
			{
				if (mRemainingOutputPriority[i] > -100000.0)
				{
					PrepareCycle();
				}
			}
		}
	}

	std::array<int32,3> PossibleGrabs = {0};
	std::array<float,3> Offsets = {0};
	int32 TotalPossibleGrabs = 0;
	for (int32 i = 0 ; i < mOutputs.Num(); ++i)
	{
		if (IsSet(mOutputStates[i],EOutputState::Connected))
		{
			const auto Output = mOutputs[i];
			//const auto AvailableSpace = Cast<AFGBuildableConveyorBase>(Output->GetConnection()->GetOuterBuildable())->GetCachedAvailableSpace_Threadsafe();
			//const int32 Possible = AvailableSpace / AFGBuildableConveyorBase::ITEM_SPACING;
			const int32 Possible = EstimatedMaxNumGrabFromConveyor(mOutputs[i],dt);
			Offsets[i] = 0.0;//AvailableSpace - Possible * AFGBuildableConveyorBase::ITEM_SPACING;
			PossibleGrabs[i] = Possible;
			TotalPossibleGrabs += Possible;
			mJammedFor[i] = Possible > 0 ? 0 : std::min(mJammedFor[i] + 1,125);
		}
	}

	if (mDebug)
	{
		UE_LOG(LogAutoSplitters,Display,TEXT("inventory=%d avail 0=%d 1=%d 2=%d"),mCachedInventorySize,PossibleGrabs[0],PossibleGrabs[1],PossibleGrabs[2]);
	}

	if (TotalPossibleGrabs == 0 || mBufferInventory->IsEmpty())
		return;

	auto MaxGrabs = PossibleGrabs;

	for (int32 Slot = 0; Slot < mInventorySizeX; ++Slot)
	{
		if (TotalPossibleGrabs == 0)
			break;
		
		if (mBufferInventory->IsIndexEmpty(Slot))
			continue;			

		int32 Next = -1;
		float Priority = -INFINITY;
		bool BlockOutput = false;
		for (int32 i = 0; i < 3; ++i)
		{
			if (PossibleGrabs[i] > 0)
			{
				if (mRemainingOutputPriority[i] + mEpsilon > Priority)
				{
					Next = i;
					Priority = mRemainingOutputPriority[i];
					BlockOutput = false;
				}
			}
			else
			{
				if (mRemainingOutputPriority[i] - mEpsilon > Priority)
				{
					//UE_LOG(LogAutoSplitters,Display,TEXT("Output %d is jammed for %d tries"),i,mJammedFor[i]);
					if (mJammedFor[i] >= 20)
					{
						// output has been jammed for 120 ticks (at least 2 seconds), skip
						//UE_LOG(LogAutoSplitters,Display,TEXT("Output %d is jammed, ignoring precedence"),i);
					}
					else
					{
						Next = i;
						Priority = mRemainingOutputPriority[i];
						BlockOutput = true;
					}
				}
			}
		}

		if (BlockOutput || Next < 0)
			break;
		
		FInventoryStack Stack;
		mBufferInventory->GetStackFromIndex(Slot,Stack);
		mDistributionTable.Emplace(
			mOutputs[Next],
			Stack.Item,
			(MaxGrabs[Next] - PossibleGrabs[Next]) * AFGBuildableConveyorBase::ITEM_SPACING + Offsets[Next],
			Slot
			);
		mRemainingOutputPriority[Next] -= mPriorityStepSize[Next];
		--PossibleGrabs[Next];
		--TotalPossibleGrabs;

		if (--mLeftInCycle <= 0)
			PrepareCycle();
	}

	if (mDebug)
	{
		auto componentIndex = [&](auto component) -> int32
		{
			for (int32 i = 0 ; i < 3 ; ++i)
			{
				if (mOutputs[i] == component)
					return i;
			}
			return -1;
		};
		for (auto& e : mDistributionTable)
		{
			UE_LOG(LogAutoSplitters,Display,TEXT("  output=%d inventory=%d offset=%f"),componentIndex(e.Connection),e.IndexInInventory,e.OffsetBeyond);
		}
	}
}

void AMFGBuildableAutoSplitter::SetupDistribution(bool LoadingSave)
{

	if (!LoadingSave)
	{
		for (int32 i = 0 ; i < 3 ; ++i)
		{
			mOutputStates[i] = SetFlag(mOutputStates[i],EOutputState::Connected,mOutputs[i]->IsConnected());
		}
	}
		
	if (!(
		IsSet(mOutputStates[0],EOutputState::Connected) ||
		IsSet(mOutputStates[1],EOutputState::Connected) ||
		IsSet(mOutputStates[2],EOutputState::Connected)))
	{
		return;		
	}
	
	// calculate item counts per cycle
	std::array<int32,3> Items = {
		IsSet(mOutputStates[0],EOutputState::Connected) * mOutputRates[0] * 10000,
		IsSet(mOutputStates[1],EOutputState::Connected) * mOutputRates[1] * 10000,
		IsSet(mOutputStates[2],EOutputState::Connected) * mOutputRates[2] * 10000,
	};

	auto GCD = std::gcd(std::gcd(Items[0],Items[1]),Items[2]);

	if (GCD == 0)
	{
		UE_LOG(LogAutoSplitters,Display,TEXT("Nothing connected, chilling"));
		return;
	}

	for (auto& Item : Items)
		Item /= GCD;

	mCycleLength = 0;
    bool Changed = false;
	mEpsilon = 5e-2;
	for (int32 i = 0 ; i < 3 ; ++i)
	{
		if (IsSet(mOutputStates[i],EOutputState::Connected))
		{
			mCycleLength += Items[i];
			float StepSize = 0.0;
			if (Items[i] > 0)
			{
				StepSize = 1.0/Items[i];
				mEpsilon = std::min(mEpsilon,mPriorityStepSize[i] * 5e-2f);
			}
			else
			{
				StepSize = 0;
			}
			if (mPriorityStepSize[i] != StepSize)
			{
				mPriorityStepSize[i] = StepSize;
				Changed = true;
			}
		}
		else
		{
			// disable output
			if (mPriorityStepSize[i] != 0)
			{
				mPriorityStepSize[i] = 0;
				Changed = true;
			}
		}
	}

	mEpsilon = 0;
	
	if (Changed && !LoadingSave)
		PrepareCycle();
}

void AMFGBuildableAutoSplitter::PrepareCycle()
{

	mLeftInCycle = mCycleLength;

	for (int i = 0; i < 3 ; ++i)
	{
		if (IsSet(mOutputStates[i],EOutputState::Connected) && mOutputRates[i] > 0)
			mRemainingOutputPriority[i] = 1.0;
		else
			mRemainingOutputPriority[i] = -100000.0;
	}
}

bool AMFGBuildableAutoSplitter::SetOutputRate(const int32 Output, const float Rate)
{
	if (Output < 0 || Output > 2)
		return false;
	
	if (Rate < 0.0 || Rate > 780.0)
		return false;

	if (mOutputRates[Output] == Rate)
		return true;

	mOutputRates[Output] = Rate;

    SetupDistribution();
	BalanceNetwork();

	return true;
}

bool AMFGBuildableAutoSplitter::SetOutputAutomatic(int32 Output, bool automatic)
{

	if (Output < 0 || Output > 2)
		return false;

	if (automatic == IsSet(mOutputStates[Output],EOutputState::Automatic))
		return true;
	
	if (automatic)
	{
		mOutputStates[Output] = SetFlag(mOutputStates[Output],EOutputState::Automatic);
		BalanceNetwork();
	}
	else
	{
		mOutputStates[Output] = ClearFlag(mOutputStates[Output],EOutputState::Automatic);
		if (IsSet(mOutputStates[Output],EOutputState::Connected))
		{
			if (mOutputRates[Output] != 1.0)
			{
				mOutputRates[Output] = 1.0;
				SetupDistribution();
				BalanceNetwork();
			}
		}
	}

	return true;
}

int32 AMFGBuildableAutoSplitter::BalanceNetwork(bool RootOnly)
{
	mBalancingRequired = false;
	TSet<AMFGBuildableAutoSplitter*> SplitterSet;
	// start by going upstream
	auto Root = this;
	for (auto Current = this ; Current ; Current = FindAutoSplitterAfterBelt(Current->mInputs[1],false))
	{
		if (SplitterSet.Contains(Current))
		{
			UE_LOG(LogAutoSplitters,Warning,TEXT("Cycle in auto splitter network detected, bailing out"));
			return -1;
		}
		SplitterSet.Add(Current);
		Root = Current;
	}

	if (RootOnly && this != Root)
	{
		Root->mBalancingRequired = true;
		return -1;
	}

	// Now walk the tree to discover the whole network
	TArray<TArray<FConnections>> SplitterHierarchy;
	DiscoverHierarchy(SplitterHierarchy,Root,0);


	// We have found all connected AutoSplitters, now let's re-balance
	TMap<AMFGBuildableAutoSplitter*,float> InputRates;

	int32 SplitterCount = 0;
	for (int32 Level = SplitterHierarchy.Num() - 1 ; Level >= 0 ; --Level)
	{
		for (auto& c : SplitterHierarchy[Level])
		{
			++SplitterCount;
			bool Changed = false;
			float InputRate = 0;
			for (int32 i = 0; i < 3; ++i)
			{
				auto& OutputState = c.Splitter->mOutputStates[i];
				const bool IsConnected = c.Splitter->mOutputs[i]->IsConnected();

				if (c.Outputs[i])
				{
					OutputState = SetFlag(OutputState,EOutputState::AutoSplitter);
					if (IsSet(OutputState,EOutputState::Automatic))
					{
						const auto OutputRate = InputRates[c.Outputs[i]];
						if (OutputRate != c.Splitter->mOutputRates[i])
						{
							c.Splitter->mOutputRates[i] = OutputRate;
							Changed = true;
						}
					}
				}
				else
				{
					OutputState = ClearFlag(OutputState,EOutputState::AutoSplitter);	
					if (IsSet(OutputState,EOutputState::Automatic))
					{
						if (c.Splitter->mOutputRates[i] != 1.0)
						{
							c.Splitter->mOutputRates[i] = 1.0;
							Changed = true;
						}
					}
					if (IsConnected != IsSet(OutputState,EOutputState::Connected))
					{
						OutputState = SetFlag(OutputState,EOutputState::Connected,IsConnected);
						Changed = true;
					}
				}

				// only count automatic outputs that have a belt connected
				if (!IsSet(OutputState,EOutputState::Automatic) || IsConnected)
					InputRate += c.Splitter->mOutputRates[i];
			}
			if (Changed)
				c.Splitter->SetupDistribution();

			InputRates.Add(c.Splitter,InputRate);
		}
	}
	return SplitterCount;
}

AMFGBuildableAutoSplitter* AMFGBuildableAutoSplitter::FindAutoSplitterAfterBelt(
	UFGFactoryConnectionComponent* Connection, bool Forward)
{
	while (Connection->IsConnected())
	{
		Connection = Connection->GetConnection();
		const auto Belt = Cast<AFGBuildableConveyorBase>(Connection->GetOuterBuildable());
		if (Belt)
		{			
			Connection = Forward ? Belt->GetConnection1() : Belt->GetConnection0();
			continue;
		}
		return Cast<AMFGBuildableAutoSplitter>(Connection->GetOuterBuildable());
	}
	return nullptr;
}

void AMFGBuildableAutoSplitter::DiscoverHierarchy(TArray<TArray<FConnections>>& Splitters,
                                                  AMFGBuildableAutoSplitter* Splitter, const int32 Level)
{
	if (!Splitters.IsValidIndex(Level))
	{
		Splitters.Emplace();
	}
	auto& Connections = Splitters[Level][Splitters[Level].Emplace(Splitter)];
	int32 i = 0;
	for (auto Connection : Splitter->mOutputs)
	{
		const auto Downstream = FindAutoSplitterAfterBelt(Connection, true);
		Connections.Outputs[i++] = Downstream;
		if (Downstream)
			DiscoverHierarchy(Splitters, Downstream, Level + 1);
	}
}
