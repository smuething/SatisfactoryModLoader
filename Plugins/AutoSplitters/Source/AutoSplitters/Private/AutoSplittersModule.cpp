#include "AutoSplittersModule.h"

#include "Patching/NativeHookManager.h"

#include "FGWorldSettings.h"
#include "Buildables/MFGBuildableAutoSplitter.h"

#include "AutoSplittersLog.h"

#pragma optimize( "", off )

void FAutoSplittersModule::StartupModule()
{

#if UE_BUILD_SHIPPING

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

	auto NotifyBeginPlayHook = [&](auto WorldSettings)
	{
		if (mUpgradedSplitters == 0)
			return;

		UE_LOG(LogAutoSplitters,Display,TEXT("Upgraded %d AutoSplitters while loading savegame"),mUpgradedSplitters);

		if (this->mDismantledConveyors > 0)
		{
			UE_LOG(LogAutoSplitters,Warning,TEXT("Dismantled %d conveyors during AutoSplitter upgrade"),mDismantledConveyors);
		}

		mUpgradedSplitters = 0;
		mDismantledConveyors = 0;
	};

	void* SampleInstance = GetMutableDefault<AFGWorldSettings>();

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGWorldSettings::NotifyBeginPlay,SampleInstance,NotifyBeginPlayHook);

#endif // UE_BUILD_SHIPPING

}

#pragma optimize( "", on )

IMPLEMENT_GAME_MODULE(FAutoSplittersModule,AutoSplitters);
