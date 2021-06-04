// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoSplittersGameWorldModule.h"
#include "AutoSplittersLog.h"

void UAutoSplittersGameWorldModule::DispatchLifecycleEvent(ELifecyclePhase Phase)
{
    Super::DispatchLifecycleEvent(Phase);

    switch(Phase)
    {
        case ELifecyclePhase::INITIALIZATION:
            {
                UE_LOG(LogAutoSplitters,Display,TEXT("Preloading mod configuration"));
                Config = FAutoSplitters_ConfigStruct::GetActiveConfig();
                break;
            }
        default:
            {}
    }
}
