// ILikeBanas

#include "Buildables/MFGBuildableAutoSplitter.h"

#include <numeric>

#include "AutoSplittersLog.h"
#include "FGFactoryConnectionComponent.h"
#include "Buildables/FGBuildableConveyorBase.h"

AMFGBuildableAutoSplitter::AMFGBuildableAutoSplitter()
	: mOutputRates({1.0,1.0,1.0})
	, mOutputStates({Flag(EOutputState::Automatic),Flag(EOutputState::Automatic),Flag(EOutputState::Automatic)})
	, mRemainingItems({0,0,0})
	, mItemsPerCycle({0,0,0})
	, mLeftInCycle(0)
    , mDebug(false)
	, mCycleLength(0)
	, mJammedFor({0,0,0})
	, mAllowedItems({0,0,0})
	, mGrabbedItems({0,0,0})
	, mPriorityStepSize({0.0,0.0,0.0})
	, mBalancingRequired(true)
    , mCachedInventoryItemCount(0)
    , mItemRate(0.0)
	, mCycleTime(0.0)
	, mReallyGrabbed(0)
{}

template<typename T, std::size_t n>
constexpr auto make_array(T value) -> std::array<T,n>
{
	std::array<T,n> r{};
	for (auto& v : r)
		v = value;
	return r;
}

void AMFGBuildableAutoSplitter::Factory_Tick(float dt)
{
	Super::Factory_Tick(dt);

	for (int i = 0 ; i < 3 ; ++i)
	{
		mLeftInCycle -= mGrabbedItems[i];
		mGrabbedItems[i] = 0;
		mAllowedItems[i] = 0;
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
		mCycleTime += dt;
		return;
	}

	bool AllowCycleExtension = true;
	if (mLeftInCycle < -20)
	{
		UE_LOG(LogAutoSplitters,Warning,TEXT("mLeftInCycle too negative (%d), resetting"),mLeftInCycle);
		PrepareCycle(false,true);
		AllowCycleExtension = false;		
	}
	else if (mLeftInCycle <= 0)
	{
		PrepareCycle(AllowCycleExtension);
		AllowCycleExtension = false;
	}

	mCycleTime += dt;	

	mCachedInventoryItemCount = 0;
	auto PopulatedInventorySlots = make_array<int32,MAX_INVENTORY_SIZE>(-1);
	for (int32 i = 0 ; i < mInventorySizeX ; ++i)
	{
		if(mBufferInventory->IsSomethingOnIndex(i))
		{
			PopulatedInventorySlots[mCachedInventoryItemCount++] = i;
		}
	}

	if (mCachedInventoryItemCount == 0)
	{
		return;
	}

	for (int32 Slot = 0 ; Slot < mCachedInventoryItemCount ; ++Slot)
	{
		int32 Next = -1;
		float Priority = -INFINITY;
		std::array<float,3> Priorities = {0,0,0};
		for (int32 i = 0; i < 3; ++i)
		{
			const int32 AssignableItems = mRemainingItems[i] - mAllowedItems[i];
			const auto ItemPriority = Priorities[i] = AssignableItems * mPriorityStepSize[i];
			if (AssignableItems > 0 && ItemPriority > Priority)
			{
				Next = i;
				Priority = ItemPriority;
			}
		}

		if (Next < 0)
		{
			bool Stuck = true;
			for (int32 i = 0 ; i < 3 ; ++i)
			{
				Stuck = Stuck && mJammedFor[i] > 60;
			}
			if (Stuck)
			{
				if (mDebug)
				{
					UE_LOG(LogAutoSplitters,Display,TEXT("All outputs are jammed, splitter is stuck"));
					break;
				}
			}
			else
			{
				UE_LOG(LogAutoSplitters,Warning,TEXT("Splitter got stuck mLeftInCycle=%d,forcing reset"),mLeftInCycle);
				PrepareCycle(AllowCycleExtension);
				AllowCycleExtension = false;
				mCycleTime += dt;
				break;
			}
		}

		std::array<bool,3> mPenalized = {false,false,false};
		while (mJammedFor[Next] > 60)
		{
			if (mDebug)
			{
				UE_LOG(LogAutoSplitters,Display,TEXT("Output %d is stuck, reassigning item and penalizing output"),Next);
			}
			mPenalized[Next] = true;
			--mRemainingItems[Next];
			++mAllowedItems[Next];
			++mGrabbedItems[Next];
			Priority = -INFINITY;
			for (int32 i = 0 ; i < 3 ; ++i)
			{
				if (!mPenalized[i] && mRemainingItems[i] - mAllowedItems[i] < 0)
					continue;

				if (Priorities[i] > Priority)
				{
					Next = i;
					Priority = Priorities[i];
				}
			}
		}

		mAssignedInventorySlots[Next][mAllowedItems[Next]++] = Slot;
	}

	for (int32 i = 0 ; i < 3 ; ++i)
	{
		if (mAllowedItems[i] > 0 && mJammedFor[i] < 125)
		{
			++mJammedFor[i];
		}
	}

	if (mDebug)
	{
		UE_LOG(LogAutoSplitters,Display,TEXT("Assigned items: 0=%d 1=%d 2=%d"),mAllowedItems[0],mAllowedItems[1],mAllowedItems[2]);
	}
}

void AMFGBuildableAutoSplitter::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion)
{
	Super::PostLoadGame_Implementation(saveVersion,gameVersion);
	mLeftInCycle = std::accumulate(mRemainingItems.begin(),mRemainingItems.end(),0);
	mCycleLength = std::accumulate(mItemsPerCycle.begin(),mItemsPerCycle.end(),0);
	mCycleTime = -100000.0; // this delays item rate calculation to the first full cycle when loading the game
	SetupDistribution(true);
}

void AMFGBuildableAutoSplitter::BeginPlay()
{
	Super::BeginPlay();
	mBalancingRequired = true;
}


void AMFGBuildableAutoSplitter::FillDistributionTable(float dt)
{
	// we are doing our own distribution management, as we need to track
	// whether assigned items were actually picked up by the outputs
}

bool AMFGBuildableAutoSplitter::Factory_GrabOutput_Implementation(UFGFactoryConnectionComponent* connection,
	FInventoryItem& out_item, float& out_OffsetBeyond, TSubclassOf<UFGItemDescriptor> type)
{
	int32 Output = -1;
	for (int32 i = 0 ; i < 3 ; ++i)
	{
		if (connection == mOutputs[i])
		{
			Output = i;
			break;
		}
	}
	if (Output < 0)
	{
		UE_LOG(LogAutoSplitters,Error,TEXT("Could not find connection!"));
		return false;
	}

	mJammedFor[Output] = -1;	

	if (mAllowedItems[Output] <= mGrabbedItems[Output])
	{
		return false;
	}
	
	FInventoryStack Stack;
	const auto Slot = mAssignedInventorySlots[Output][mGrabbedItems[Output]];
	mBufferInventory->GetStackFromIndex(Slot,Stack);
	mBufferInventory->RemoveAllFromIndex(Slot);
	out_item = Stack.Item;
	out_OffsetBeyond = mGrabbedItems[Output] * AFGBuildableConveyorBase::ITEM_SPACING;
	++mGrabbedItems[Output];
	--mRemainingItems[Output];
	++mReallyGrabbed;

	if (mDebug)
	{
		UE_LOG(LogAutoSplitters,Display,TEXT("Sent item out of output %d"),Output);
	}
	
	return true;
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
	for (int32 i = 0 ; i < 3 ; ++i)
	{
		mItemsPerCycle[i] = static_cast<int32>(std::round(IsSet(mOutputStates[i], EOutputState::Connected) * mOutputRates[i] * 10000));
	}

	auto GCD = std::gcd(std::gcd(mItemsPerCycle[0],mItemsPerCycle[1]),mItemsPerCycle[2]);

	if (GCD == 0)
	{
		UE_LOG(LogAutoSplitters,Display,TEXT("Nothing connected, chilling"));
		return;
	}

	for (auto& Item : mItemsPerCycle)
		Item /= GCD;

	mCycleLength = 0;
    bool Changed = false;
	for (int32 i = 0 ; i < 3 ; ++i)
	{
		if (IsSet(mOutputStates[i],EOutputState::Connected))
		{
			mCycleLength += mItemsPerCycle[i];
			float StepSize = 0.0f;
			if (mItemsPerCycle[i] > 0)
			{
				StepSize = 1.0f/mItemsPerCycle[i];
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
		
	if (Changed && !LoadingSave)
	{
		mRemainingItems = {0,0,0};
		mLeftInCycle = 0;
		PrepareCycle(false);
	}
}

void AMFGBuildableAutoSplitter::PrepareCycle(const bool AllowCycleExtension, const bool Reset)
{

	if (!Reset && mCycleTime > 0.0 && mReallyGrabbed >= mCycleLength)
	{
		// update statistics
		if (mItemRate > 0.0)
		{
			mItemRate = EXPONENTIAL_AVERAGE_WEIGHT * 60 * mReallyGrabbed / mCycleTime + (1.0-EXPONENTIAL_AVERAGE_WEIGHT) * mItemRate;
		}
		else
		{
			// bootstrap
			mItemRate = 60.0 * mReallyGrabbed / mCycleTime;
		}
		
		if (AllowCycleExtension && mCycleTime < 2.0)
		{
			UE_LOG(LogAutoSplitters,Display,TEXT("Cycle time too short (%f), doubling cycle length to %d"),mCycleTime,2*mCycleLength);
			mCycleLength *= 2;
			for (int i = 0 ; i < 3 ; ++i)
				mItemsPerCycle[i] *= 2;
		}
		else if (mCycleTime > 10.0)
		{
			bool CanShortenCycle = !(mCycleLength & 1);
			for (int i = 0 ; i < 3 ; ++i)
				CanShortenCycle = CanShortenCycle && !(mItemsPerCycle[i] & 1);

			if (CanShortenCycle)
			{
				UE_LOG(LogAutoSplitters,Display,TEXT("Cycle time too long (%f), halving cycle length to %d"),mCycleTime,mCycleLength/2);
				mCycleLength /= 2;
				for (int i = 0 ; i < 3 ; ++i)
					mItemsPerCycle[i] /= 2;
			}
		}
	}

	mCycleTime = 0.0;
	mReallyGrabbed = 0;	

	if (Reset)
	{
		mLeftInCycle = mCycleLength;

		for (int i = 0; i < 3 ; ++i)
		{
			if (IsSet(mOutputStates[i],EOutputState::Connected) && mOutputRates[i] > 0)
				mRemainingItems[i] = mItemsPerCycle[i];
			else
				mRemainingItems[i] = 0;
		}
		
	}
	else
	{
		mLeftInCycle += mCycleLength;

		for (int i = 0; i < 3 ; ++i)
		{
			if (IsSet(mOutputStates[i],EOutputState::Connected) && mOutputRates[i] > 0)
				mRemainingItems[i] += mItemsPerCycle[i];
			else
				mRemainingItems[i] = 0;
		}
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

bool AMFGBuildableAutoSplitter::SetOutputAutomatic(int32 Output, bool Automatic)
{

	if (Output < 0 || Output > 2)
		return false;

	if (Automatic == IsSet(mOutputStates[Output],EOutputState::Automatic))
		return true;
	
	if (Automatic)
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
