﻿#include "AutoSplittersModule.h"

void FAutoSplittersModule::StartupModule()
{
	// TSharedPtr<FJsonObject> powerMappingJson(new FJsonObject());
	// powerMappingJson->SetNumberField(TEXT("/Game/Teleporter/buildable/Build_Teleporteur.Build_Teleporteur_C"), -20);

	// if (powerMappingJson)
	// {
	// 	for (auto entry : powerMappingJson->Values)
	// 	{
	// 		auto key = entry.Key;
	// 		auto value = entry.Value->AsNumber();
	//
	// 		powerConsumptionMap[key] = value;
	//
	// 		PC_LOG_Display(TEXT(" PowerChecker:     - "), *key, TEXT(" = "), value);
	// 	}
	// }

	/*
#if UE_BUILD_SHIPPING
	{
		void* ObjectInstance = GetMutableDefault<UFGPowerCircuit>();

		SUBSCRIBE_METHOD_VIRTUAL_AFTER(UFGPowerCircuit::OnCircuitChanged, ObjectInstance, onPowerCircuitChangedHook)
	}

	{
		void* ObjectInstance = GetMutableDefault<AFGBuildableFactory>();

		SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableFactory::SetPendingPotential, ObjectInstance, setPendingPotentialCallback);
	}

	{
		void* ObjectInstance = GetMutableDefault<AFGBuildableFrackingActivator>();

		SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableFrackingActivator::SetPendingPotential, ObjectInstance, setPendingPotentialCallback);
	}

	{
		void* ObjectInstance = GetMutableDefault<AFGBuildableGeneratorFuel>();

		SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableGeneratorFuel::SetPendingPotential, ObjectInstance, setPendingPotentialCallback);
	}

	PC_LOG_Display(TEXT("==="));
#endif
}

void FPowerCheckerModule::onPowerCircuitChangedHook(UFGPowerCircuit* powerCircuit)
{
	if (!APowerCheckerLogic::singleton)
	{
		return;
	}

	auto circuitGroupId = powerCircuit->GetCircuitGroupID();

	FScopeLock ScopeLock(&APowerCheckerLogic::singleton->eclCritical);
	for (auto powerChecker : APowerCheckerLogic::singleton->allPowerCheckers)
	{
		if (circuitGroupId == powerChecker->getCircuitGroupId())
		{
			powerChecker->TriggerUpdateValues(true);
		}
	}
}

void FPowerCheckerModule::setPendingPotentialCallback(class AFGBuildableFactory* buildable, float potential)
{
	PC_LOG_Display(
        TEXT("SetPendingPotential of building "),
        *GetPathNameSafe(buildable),
        TEXT(" to "),
        potential
        );

	auto powerInfo = buildable->GetPowerInfo();
	if(!powerInfo)
	{
		return;
	}

	auto powerCircuit = powerInfo->GetPowerCircuit();
	if(!powerCircuit)
	{
		return;
	}

	auto circuitGroupId = powerCircuit->GetCircuitGroupID();

	// Update all EfficiencyCheckerBuildings that connects to this building
	FScopeLock ScopeLock(&APowerCheckerLogic::singleton->eclCritical);
	for (auto powerChecker : APowerCheckerLogic::singleton->allPowerCheckers)
	{
		if (circuitGroupId == powerChecker->getCircuitGroupId())
		{
			powerChecker->TriggerUpdateValues(true);
		}
	}
	*/
}

IMPLEMENT_GAME_MODULE(FAutoSplittersModule,AutoSplitters);