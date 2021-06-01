#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "AutoSplitters_ConfigStruct.generated.h"

struct FAutoSplitters_ConfigStruct_Upgrade;

USTRUCT(BlueprintType)
struct FAutoSplitters_ConfigStruct_Upgrade {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    bool RemoveAllConveyors;

    UPROPERTY(BlueprintReadWrite)
    bool ShowWarningMessage;
};

/* Struct generated from Mod Configuration Asset '/AutoSplitters/AutoSplitters_Config' */
USTRUCT(BlueprintType)
struct FAutoSplitters_ConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    FAutoSplitters_ConfigStruct_Upgrade Upgrade;

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FAutoSplitters_ConfigStruct GetActiveConfig() {
        FAutoSplitters_ConfigStruct ConfigStruct{};
        FConfigId ConfigId{"AutoSplitters", ""};
        UConfigManager* ConfigManager = GEngine->GetEngineSubsystem<UConfigManager>();
        ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FAutoSplitters_ConfigStruct::StaticStruct(), &ConfigStruct});
        return ConfigStruct;
    }
};

