// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Module/GameWorldModule.h"
#include "AutoSplitters_ConfigStruct.h"

#include "AutoSplittersGameWorldModule.generated.h"

/**
 *
 */
UCLASS()
class AUTOSPLITTERS_API UAutoSplittersGameWorldModule : public UGameWorldModule
{
    GENERATED_BODY()

public:

    UPROPERTY(Transient,BlueprintReadOnly)
    FAutoSplitters_ConfigStruct Config;

    virtual void DispatchLifecycleEvent(ELifecyclePhase Phase) override;
};
