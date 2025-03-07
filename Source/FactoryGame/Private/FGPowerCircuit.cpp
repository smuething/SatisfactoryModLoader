// This file has been automatically generated by the Unreal Header Implementation tool

#include "FGPowerCircuit.h"

FPowerGraphPoint::FPowerGraphPoint(){ }
bool FPowerGraphPoint::NetSerialize(FArchive& ar,  UPackageMap* map, bool& out_success){ return bool(); }
FPowerCircuitStats::FPowerCircuitStats(){ }
bool FPowerCircuitStats::NetSerialize(FArchive& ar,  UPackageMap* map, bool& out_success){ return bool(); }
FPowerGraphPoint& FPowerCircuitStats::MakeAndAddGraphPoint(){ return *(new FPowerGraphPoint); }
FPowerGraphPoint& FPowerCircuitStats::AdvanceToNextGraphPoint(){ return *(new FPowerGraphPoint); }
void UFGPowerCircuit::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFGPowerCircuit, mMaximumPowerConsumption);
	DOREPLIFETIME(UFGPowerCircuit, mHasPower);
	DOREPLIFETIME(UFGPowerCircuit, mHasBatteries);
	DOREPLIFETIME(UFGPowerCircuit, mBatterySumPowerStore);
	DOREPLIFETIME(UFGPowerCircuit, mBatterySumPowerStoreCapacity);
	DOREPLIFETIME(UFGPowerCircuit, mBatterySumPowerInput);
	DOREPLIFETIME(UFGPowerCircuit, mTimeToBatteriesEmpty);
	DOREPLIFETIME(UFGPowerCircuit, mTimeToBatteriesFull);
	DOREPLIFETIME(UFGPowerCircuit, mIsFuseTriggered);
	DOREPLIFETIME(UFGPowerCircuit, mPowerStats);
}
void UFGPowerCircuit::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker){ }
UFGPowerCircuit::UFGPowerCircuit() : Super() {
	this->mPowerProductionCapacity = 0.0;
	this->mPowerProduced = 0.0;
	this->mPowerConsumed = 0.0;
	this->mMaximumPowerConsumption = 0.0;
	this->mHasPower = false;
	this->mHasBatteries = false;
	this->mBatterySumPowerStore = 0.0;
	this->mBatterySumPowerStoreCapacity = 0.0;
	this->mBatterySumPowerInput = 0.0;
	this->mTimeToBatteriesEmpty = 0.0;
	this->mTimeToBatteriesFull = 0.0;
	this->mIsFuseTriggered = false;
	this->mPowerStoreAtBatteryDepletionStart = 0.0;
	this->mTimeSinceLastWarning = 0.0;
}
void UFGPowerCircuit::ResetFuse(){ }
void UFGPowerCircuit::DisplayDebug( UCanvas* canvas, const  FDebugDisplayInfo& debugDisplay, float& YL, float& YPos, float indent){ }
bool UFGPowerCircuit::IsNoPowerCheatOn() const{ return bool(); }
void UFGPowerCircuit::OnCircuitChanged(){ }
bool UFGPowerCircuit::IsTrivial() const{ return bool(); }
void UFGPowerCircuit::OnRemoved(){ }
void UFGPowerCircuit::UpdateStatsGeneral(){ }
void UFGPowerCircuit::UpdateStatsGraph(){ }
void UFGPowerCircuit::PinStatsGraphPoint(){ }
void UFGPowerCircuit::StatFuseTriggered(){ }
UFGCircuit* UFGPowerCircuit::SplitCircuit(AFGCircuitSubsystem* subsystem) const{ return nullptr; }
UFGCircuitGroup* UFGPowerCircuit::CreateCircuitGroup(AFGCircuitSubsystem* subsystem) const{ return nullptr; }
void UFGPowerCircuit::SetHasPower(bool hasPower){ }
void UFGPowerCircuitGroup::ResetFuses(){ }
void UFGPowerCircuitGroup::RegisterPrioritySwitch( AFGBuildablePriorityPowerSwitch* circuitSwitch){ }
void UFGPowerCircuitGroup::PushCircuit(UFGCircuit* circuit){ }
bool UFGPowerCircuitGroup::PreTickCircuitGroup(float dt){ return bool(); }
void UFGPowerCircuitGroup::TickCircuitGroup(float dt){ }
void UFGPowerCircuitGroup::VisitCircuitBridge( AFGBuildableCircuitBridge* circuitBridge){ }
void UFGPowerCircuitGroup::TickPowerCircuitGroup(float deltaTime){ }
float UFGPowerCircuitGroup::TickBatteries(float deltaTime, const float netPowerProduction, bool isFuseTriggered){ return float(); }
bool UFGPowerCircuitGroup::TryTurnOffPrioritySwitch(){ return bool(); }
void UFGPowerCircuitGroup::OnFuseSet(){ }
void UFGPowerCircuitGroup::OnFuseReset(){ }
void UFGPowerCircuitGroup::OnPrioritySwitchesTurnedOff(int32 priority){ }
void UFGPowerCircuitGroup::DisplayDebug( UCanvas* canvas, const  FDebugDisplayInfo& debugDisplay, float& YL, float& YPos, float indent){ }
