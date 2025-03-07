// This file has been automatically generated by the Unreal Header Implementation tool

#include "FGGenericBuildableHologram.h"

AFGGenericBuildableHologram::AFGGenericBuildableHologram() : Super() {
	this->mIsWallRotationAllowed = true;
	this->mAllowGranularRotation = false;
	this->mCanSnapToFoundationFloor = true;
	this->mCanSnapToFoundationCeiling = false;
	this->mCanSnapToFoundationSide = false;
	this->mCanSnapToWalls = false;
	this->mCanSnapToAngularWalls = true;
	this->mBeamSnappingMode = EBeamSnappingMode::BSM_None;
	this->mPillarSnappingMode = EPillarSnappingMode::PSM_None;
	this->mWallSnapOffset = FVector2D::ZeroVector;
	this->mSnapAxis = EAxis::Z;
	this->mFoundationSnappingInset = 0.0;
}
void AFGGenericBuildableHologram::BeginPlay(){ }
bool AFGGenericBuildableHologram::TrySnapToActor(const FHitResult& hitResult){ return bool(); }
int32 AFGGenericBuildableHologram::GetRotationStep() const{ return int32(); }
ENudgeFailReason AFGGenericBuildableHologram::NudgeHologram(const FVector& NudgeInput, const FHitResult& HitResult){ return ENudgeFailReason(); }
void AFGGenericBuildableHologram::UpdateRotationValuesFromTransform(){ }
bool AFGGenericBuildableHologram::IsHologramIdenticalToActor(AActor* actor, const FVector& hologramLocationOffset) const{ return bool(); }
