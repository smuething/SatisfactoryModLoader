// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoSplittersRCO.h"

void UAutoSplittersRCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UAutoSplittersRCO,Dummy);
}

void UAutoSplittersRCO::SetOutputAutomatic_Implementation(AMFGBuildableAutoSplitter* Splitter, int32 Output,
    bool Automatic) const
{
}

void UAutoSplittersRCO::EnableReplication_Implementation(AMFGBuildableAutoSplitter* Splitter, float Duration) const
{
}

void UAutoSplittersRCO::SetTargetInputRate_Implementation(AMFGBuildableAutoSplitter* Splitter, float Rate) const
{
}

void UAutoSplittersRCO::SetTargetRateAutomatic_Implementation(AMFGBuildableAutoSplitter* Splitter, bool Automatic) const
{
}

void UAutoSplittersRCO::SetOutputRate_Implementation(AMFGBuildableAutoSplitter* Splitter, int32 Output,
    float Rate) const
{
}
