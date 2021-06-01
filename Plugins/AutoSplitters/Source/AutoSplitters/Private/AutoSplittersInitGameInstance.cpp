// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoSplittersInitGameInstance.h"

#include "Registry/RemoteCallObjectRegistry.h"
#include "AutoSplittersLog.h"
#include "AutoSplittersRCO.h"

UAutoSplittersInitGameInstance::UAutoSplittersInitGameInstance()
{
    bRootModule = true;
}

void UAutoSplittersInitGameInstance::DispatchLifecycleEvent(ELifecyclePhase Phase)
{
    switch(Phase)
    {
        case ELifecyclePhase::CONSTRUCTION:
            {
                UE_LOG(LogAutoSplitters,Display,TEXT("Registering AutoSplittersRCO object with RemoteCallObjectRegistry"));
                auto RCORegistry = GetGameInstance()->GetSubsystem<URemoteCallObjectRegistry>();
                RCORegistry->RegisterRemoteCallObject(UAutoSplittersRCO::StaticClass());
            }
        default:
            break;
    }
}
