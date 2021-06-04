// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "Buildables/MFGBuildableAutoSplitter.h"
#include "AutoSplitters_ConfigStruct.h"
#include "Subsystem/SubsystemActorManager.h"
#include "AutoSplittersLog.h"

#include "AutoSplittersSubsystem.generated.h"

/**
 *
 */
UCLASS()
class AUTOSPLITTERS_API AAutoSplittersSubsystem : public AModSubsystem
{
    GENERATED_BODY()

    friend class FAutoSplittersModule;

    static AAutoSplittersSubsystem* mCachedSubsystem;

public:

    UPROPERTY(SaveGame,BlueprintReadOnly)
    uint8 mMaxSplitterVersion = AMFGBuildableAutoSplitter::VERSION;

    UPROPERTY(SaveGame,BlueprintReadOnly)
    uint8 mMinSplitterVersion = 0;

    UPROPERTY(Transient,BlueprintReadOnly)
    FAutoSplitters_ConfigStruct mConfig;

protected:

    virtual void Init() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

    AAutoSplittersSubsystem();

    static AAutoSplittersSubsystem* Get(UObject* WorldContext)
    {
        if (mCachedSubsystem)
            return mCachedSubsystem;
        const auto World = WorldContext->GetWorld();
        auto SubsystemActorManager = World->GetSubsystem<USubsystemActorManager>();
        mCachedSubsystem = SubsystemActorManager->GetSubsystemActor<AAutoSplittersSubsystem>();
        check(mCachedSubsystem);
        return mCachedSubsystem;
    }

    void ReloadConfig();

};
