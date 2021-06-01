#include "AutoSplittersModule.h"

#include "Patching/NativeHookManager.h"

#include "FGWorldSettings.h"
#include "UI/FGPopupWidget.h"
#include "FGPlayerController.h"
#include "FGBlueprintFunctionLibrary.h"
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

	auto NotifyBeginPlayHook = [&](AFGWorldSettings* WorldSettings)
	{
		if (mUpgradedSplitters == 0)
			return;

		UE_LOG(LogAutoSplitters,Display,TEXT("Upgraded %d AutoSplitters while loading savegame"),mUpgradedSplitters);

		if (this->mDismantledConveyors > 0)
		{
			UE_LOG(LogAutoSplitters,Warning,TEXT("AutoSplitters Mod Upgrade: Dismantled %d Conveyors"),mDismantledConveyors);

			FString Str = FString::Printf(TEXT("Your savegame contained Auto Splitters created with versions of the mod older than 0.3.0,"\
				"which connect to the attached conveyors in a wrong way. The mod has upgraded these Auto Splitters, but some connections could"\
				"not be repaired.\n\n WARNING: Those conveyors have been dismantled to make it easy for you to find the broken splitters.\n\nA total "\
				"of %d conveyors have been removed."),mDismantledConveyors);

			AFGPlayerController* LocalController = UFGBlueprintFunctionLibrary::GetLocalPlayerController(WorldSettings->GetWorld());

			FPopupClosed CloseDelegate;

			UFGBlueprintFunctionLibrary::AddPopupWithCloseDelegate(LocalController,FText::FromString("Savegame upgraded to AutoSplitters 0.3.x"),FText::FromString(Str),CloseDelegate);
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
