// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/AutoSplittersSubsystem.h"

AAutoSplittersSubsystem* AAutoSplittersSubsystem::mCachedSubsystem = nullptr;

AAutoSplittersSubsystem::AAutoSplittersSubsystem()
{
    ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnServer;
}

void AAutoSplittersSubsystem::Init()
{
    Super::Init();
    ReloadConfig();
}

void AAutoSplittersSubsystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    if (this == mCachedSubsystem)
        mCachedSubsystem = nullptr;
}

void AAutoSplittersSubsystem::ReloadConfig()
{
    mConfig = FAutoSplitters_ConfigStruct::GetActiveConfig();
}

/*
void AAutoSplittersSubsystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AAutoSplittersSubsystem,mMaxSplitterVersion);
    DOREPLIFETIME(AAutoSplittersSubsystem,mMinSplitterVersion);
    DOREPLIFETIME(AAutoSplittersSubsystem,mConfig);
}
*/
