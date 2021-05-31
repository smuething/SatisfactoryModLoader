// Fill out your copyright notice in the Description page of Project Settings.

#include "AutoSplittersRCO.h"

#include "Buildables/MFGBuildableAutoSplitter.h"

void UAutoSplittersRCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UAutoSplittersRCO,Dummy);
}

void UAutoSplittersRCO::SetOutputAutomatic_Implementation(AMFGBuildableAutoSplitter* Splitter, int32 Output,
                                                          bool Automatic) const
{
    Splitter->SetOutputAutomatic_Implementation(Output,Automatic);
}

void UAutoSplittersRCO::EnableReplication_Implementation(AMFGBuildableAutoSplitter* Splitter, float Duration) const
{
    Splitter->EnableReplication_Implementation(Duration);
}

void UAutoSplittersRCO::SetTargetInputRate_Implementation(AMFGBuildableAutoSplitter* Splitter, float Rate) const
{
    Splitter->SetTargetInputRate_Implementation(Rate);
}

void UAutoSplittersRCO::SetTargetRateAutomatic_Implementation(AMFGBuildableAutoSplitter* Splitter, bool Automatic) const
{
    Splitter->SetTargetRateAutomatic_Implementation(Automatic);
}

void UAutoSplittersRCO::SetOutputRate_Implementation(AMFGBuildableAutoSplitter* Splitter, int32 Output,
    float Rate) const
{
    Splitter->SetOutputRate_Implementation(Output,Rate);
}
