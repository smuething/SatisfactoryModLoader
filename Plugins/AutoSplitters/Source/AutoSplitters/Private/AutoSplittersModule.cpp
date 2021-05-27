#include "AutoSplittersModule.h"

#include "Patching/NativeHookManager.h"

//#include "FGDismantleInterface.h"
//#include "Buildables/FGBuildableAttachmentSplitter.h"
#include "Buildables/MFGBuildableAutoSplitter.h"

#include "AutoSplittersLog.h"

#pragma optimize( "", off )

void FAutoSplittersModule::StartupModule()
{

#if UE_BUILD_SHIPPING

	//void* Instance = static_cast<int*>(GetMutableDefault<AFGBuildableAttachmentSplitter>());


	auto PreUpgradeHook = [](auto& Call, UObject* self)
	{

		UE_LOG(LogAutoSplitters,Display,TEXT("Entered hook for IFGDismantleInterface::Execute_PreUpgrade()"));
	
		AFGBuildableAttachmentSplitter* Source = Cast<AFGBuildableAttachmentSplitter>(self);
		if (!Source)
		{
			UE_LOG(LogAutoSplitters,Display,TEXT("self is not an AFGBuildableAttachmentSplitter, bailing out"));
			return;
		}

		UE_LOG(LogAutoSplitters,Display,TEXT("Upgrading splitter to Auto Splitter"));
	
		// WARNING
		// this is a REALLY ugly hack, but should be safe as long as we ONLY access the otherwise off-limits
		// protected members mInputs and mOutputs of the source through this reference, and as long as there
		// is no funny virtual inheritance business going on, which there shouldn't be.
		AMFGBuildableAutoSplitter& DowncastedSource = *static_cast<AMFGBuildableAutoSplitter*>(Source);

		// let's get the lay of the land first
		/*
		for (int32 i = 0 ; i < DowncastedSource.mInputs.Num() ; ++i)
		{
			auto Input = DowncastedSource.mInputs[i];
			if (Input)
			{
				UE_LOG(LogAutoSplitters,Display,TEXT("Source input %d: %s"),i,Input->IsConnected() ? TEXT("Connected") : TEXT("Not connected"));
				if (Input->IsConnected())
				{
					UE_LOG(LogAutoSplitters,Display,TEXT("Connected source input %d : %d, connected: %s"),i,Input->GetConnection(),Input->GetConnection()->IsConnected());
					if (Input->GetConnection()->IsConnected())
					{
						UE_LOG(LogAutoSplitters,Display,TEXT("Connected source input %d : connected to: %s"),i,Input->GetConnection()->GetConnection());					
					}
				}
			}
		}

		for (int32 i = 0 ; i < DowncastedSource.mOutputs.Num() ; ++i)
		{
			auto Output = DowncastedSource.mOutputs[i];
			if (Output)
			{
				UE_LOG(LogAutoSplitters,Display,TEXT("Source output %d: %s"),i,Output->IsConnected() ? TEXT("Connected") : TEXT("Not connected"));
				if (Output->IsConnected())
				{
					UE_LOG(LogAutoSplitters,Display,TEXT("Connected source output %d : %d, connected: %s"),i,Output->GetConnection(),Output->GetConnection()->IsConnected());
					if (Output->GetConnection()->IsConnected())
					{
						UE_LOG(LogAutoSplitters,Display,TEXT("Connected source output %d : connected to: %s"),i,Output->GetConnection()->GetConnection());					
					}
				}
			}
		}
		*/
		Call.Cancel();

		/*
		UE_LOG(LogAutoSplitters,Display,TEXT("Entering original PreUpgrade() implementation"));
		Call(self);
		UE_LOG(LogAutoSplitters,Display,TEXT("Returned from original PreUpgrade() implementation"));

		for (int32 i = 0 ; i < DowncastedSource.mInputs.Num() ; ++i)
		{
			auto Input = DowncastedSource.mInputs[i];
			if (Input)
			{
				UE_LOG(LogAutoSplitters,Display,TEXT("Source input %d: %s"),i,Input->IsConnected() ? TEXT("Connected") : TEXT("Not connected"));
				if (Input->IsConnected())
				{
					UE_LOG(LogAutoSplitters,Display,TEXT("Connected source input %d : %d, connected: %s"),i,Input->GetConnection(),Input->GetConnection()->IsConnected());
					if (Input->GetConnection()->IsConnected())
					{
						UE_LOG(LogAutoSplitters,Display,TEXT("Connected source input %d : connected to: %s"),i,Input->GetConnection()->GetConnection());					
					}
				}
			}
		}

		for (int32 i = 0 ; i < DowncastedSource.mOutputs.Num() ; ++i)
		{
			auto Output = DowncastedSource.mOutputs[i];
			if (Output)
			{
				UE_LOG(LogAutoSplitters,Display,TEXT("Source output %d: %s"),i,Output->IsConnected() ? TEXT("Connected") : TEXT("Not connected"));
				if (Output->IsConnected())
				{
					UE_LOG(LogAutoSplitters,Display,TEXT("Connected source output %d : %d, connected: %s"),i,Output->GetConnection(),Output->GetConnection()->IsConnected());
					if (Output->GetConnection()->IsConnected())
					{
						UE_LOG(LogAutoSplitters,Display,TEXT("Connected source output %d : connected to: %s"),i,Output->GetConnection()->GetConnection());					
					}
				}
			}
		}
		*/

	};
	
	//SUBSCRIBE_METHOD(IFGDismantleInterface::Execute_PreUpgrade,PreUpgradeHook);


	
	
	auto UpgradeHook = [](auto& Call, UObject* self, AActor* newActor)
	{

		UE_LOG(LogAutoSplitters,Display,TEXT("Entered hook for IFGDismantleInterface::Execute_Upgrade()"));
	
		AMFGBuildableAutoSplitter* Target = Cast<AMFGBuildableAutoSplitter>(newActor);
		if (!Target)
		{
			UE_LOG(LogAutoSplitters,Display,TEXT("Target is not an AMFGBuildableAutoSplitter, bailing out"));
			return;
		}
		AFGBuildableAttachmentSplitter* Source = Cast<AFGBuildableAttachmentSplitter>(self);
		if (!Source)
		{
			UE_LOG(LogAutoSplitters,Display,TEXT("self is not an AFGBuildableAttachmentSplitter, bailing out"));
			return;
		}

		//UE_LOG(LogAutoSplitters,Display,TEXT("Calling UpgradeFromSplitter()"));
		//Target->UpgradeFromSplitter(*Source);

		UE_LOG(LogAutoSplitters,Display,TEXT("Cancelling original call"));
		Call.Cancel();
	};
	
	SUBSCRIBE_METHOD(IFGDismantleInterface::Execute_Upgrade,UpgradeHook);

#endif // UE_BUILD_SHIPPING

}

#pragma optimize( "", on )

IMPLEMENT_GAME_MODULE(FAutoSplittersModule,AutoSplitters);