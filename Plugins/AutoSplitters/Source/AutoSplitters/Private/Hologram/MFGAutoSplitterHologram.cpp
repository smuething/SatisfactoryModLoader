// ILikeBanas

#include "Hologram/MFGAutoSplitterHologram.h"
#include "Buildables/MFGBuildableAutoSplitter.h"
#include "Buildables/FGBuildable.h"

#include "AutoSplittersLog.h"

static constexpr std::array<int32,4> NewToOldComponentsMap = {0,1,2,3};

void AMFGAutoSplitterHologram::ConfigureComponents(AFGBuildable* inBuildable) const
{
	UE_LOG(LogAutoSplitters,Display,TEXT("Entering AMFGAutoSplitterHologram::ConfigureComponents"));
	UE_LOG(LogAutoSplitters,Display,TEXT("First input: %s First output: %s"),*AMFGAutoSplitterHologram::mInputConnection1.ToString(),*AMFGAutoSplitterHologram::mOutputConnection1.ToString());

	AMFGBuildableAutoSplitter* Splitter = Cast<AMFGBuildableAutoSplitter>(inBuildable);

	if (IsUpgrade())
	{
		UE_LOG(LogAutoSplitters,Display,TEXT("Upgrading %p from %p"),Splitter,this->mUpgradedSplitter);
	}

	for (int i = 0 ; i < mSnappedConnectionComponents.Num() ; ++i)
	{
		auto c = mSnappedConnectionComponents[i];
		UE_LOG(LogAutoSplitters,Display,TEXT("Snapped component %d : %p - %s"),i,c,c ? TEXT("exists") : TEXT("missing"));
		if (c)
		{
			UE_LOG(LogAutoSplitters,Display,TEXT("Snapped component %d : buildable = %p connected = %s"),i,c->GetOuterBuildable(),c->IsConnected() ? TEXT("true") : TEXT("false"));
			UE_LOG(LogAutoSplitters,Display,TEXT("Snapped component %d : direction = %s"),i,c->GetDirection() == EFactoryConnectionDirection::FCD_INPUT ? TEXT("input") : TEXT("output"));
			if (c->IsConnected())
			{
				UE_LOG(LogAutoSplitters,Display,TEXT("Snapped component %d connection : buildable = %p connected = %s"),i,c->GetConnection()->GetOuterBuildable(),c->GetConnection()->IsConnected() ? TEXT("true") : TEXT("false"));
			}
			c->ClearConnection();
		}
	}

	if (IsUpgrade())
	{
		UE_LOG(LogAutoSplitters,Display,TEXT("Upgrading, working around broken connection handling of upstream hologram"));
		std::array<UFGFactoryConnectionComponent*,4> SnappedConnections;
		for (int i = 0 ; i < 4 ; ++i)
		{
			SnappedConnections[i] = mSnappedConnectionComponents[i];
			if (mSnappedConnectionComponents[i])
			{
				mSnappedConnectionComponents[i]->ClearConnection();
			}
			const_cast<AMFGAutoSplitterHologram*>(this)->mSnappedConnectionComponents[i] = nullptr;
		}

		UE_LOG(LogAutoSplitters,Display,TEXT("Calling original implementation"));
		//Super::ConfigureComponents(inBuildable);
		AFGFactoryHologram::ConfigureComponents(inBuildable);
		// UE_LOG(LogAutoSplitters,Display,TEXT("Original implementation done, fixing up connections"));

		TInlineComponentArray<UFGFactoryConnectionComponent*,4> Connections;
		Splitter->GetComponents(Connections);

		if (Connections.Num() != 4)
		{
			UE_LOG(LogAutoSplitters,Error,TEXT("Unexpected number of connections: %d"),Connections.Num());
			return;
		}

		for (int i = 0 ; i < 4 ; ++i)
		{
			UE_LOG(LogAutoSplitters,Display,TEXT("Component %d, name = %s,  direction = %s"),i,*Connections[i]->GetName(),Connections[i]->GetDirection() == EFactoryConnectionDirection::FCD_INPUT ? TEXT("input") : TEXT("output"));
			if (Connections[i]->IsConnected())
			{
				UE_LOG(LogAutoSplitters,Warning,TEXT("Connection %d is connected but should not be"),i);
				Connections[i]->ClearConnection();
			}

			if (SnappedConnections[i])
			{
				UE_LOG(LogAutoSplitters,Display,TEXT("Hooking up connection %d to old connection %d, direction: %s"),i,i,SnappedConnections[i]->GetDirection() == EFactoryConnectionDirection::FCD_INPUT ? TEXT("input") : TEXT("output"));
				Connections[i]->SetConnection(SnappedConnections[i]);
				//SnappedConnections[i] = nullptr;
			}
			/*
			auto Pos = Connections[i]->GetComponentLocation();
			for (int j = 0 ; j < 4 ; ++j)
			{
				if (SnappedConnections[j])
				{
					float Dist = FVector::DistSquared(SnappedConnections[j]->GetComponentLocation(),Pos);
					bool CanConnect = Connections[i]->CanConnectTo(SnappedConnections[j]);
					UE_LOG(LogAutoSplitters,Display,TEXT("Distance %d - %d : %f, can connect: %s"),i,j,Dist,CanConnect ? TEXT("true") : TEXT("false"));
					if (Dist < 400)
					{
						UE_LOG(LogAutoSplitters,Display,TEXT("Hooking up connection %d to old connection %d, direction: %s"),i,j,SnappedConnections[j]->GetDirection() == EFactoryConnectionDirection::FCD_INPUT ? TEXT("input") : TEXT("output"));
						Connections[i]->SetConnection(SnappedConnections[j]);
						SnappedConnections[j] = nullptr;
					}
				}
			}
			*/
		}
		
		UE_LOG(LogAutoSplitters,Display,TEXT("ConfigureComponents() completed"));
		
	}
	else
	{
		UE_LOG(LogAutoSplitters,Display,TEXT("Calling original implementation"));
		Super::ConfigureComponents(inBuildable);
	}
}
