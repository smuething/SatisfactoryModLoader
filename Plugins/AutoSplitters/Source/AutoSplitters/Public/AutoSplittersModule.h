#pragma once

#include "Buildables/FGBuildableFactory.h"

// define to 1 to get more debug output to console when the debug flag is set in the splitter UI
#define AUTO_SPLITTERS_DEBUG 1
#define AUTO_SPLITTERS_DELAY_UNTIL_READY 1

class FAutoSplittersModule : public IModuleInterface
{
	friend class AMFGBuildableAutoSplitter;

	int32 mUpgradedSplitters = 0;
	int32 mDismantledConveyors = 0;

public:
	virtual void StartupModule() override;

	//static void onPowerCircuitChangedHook(class UFGPowerCircuit* powerCircuit);
	//static void setPendingPotentialCallback(class AFGBuildableFactory* buildable, float potential);

	//static std::map<FString, float> powerConsumptionMap;
};
