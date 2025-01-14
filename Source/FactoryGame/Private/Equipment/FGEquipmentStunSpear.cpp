// This file has been automatically generated by the Unreal Header Implementation tool

#include "Equipment/FGEquipmentStunSpear.h"
#include "Components/SceneComponent.h"

AFGEquipmentStunSpear::AFGEquipmentStunSpear() : Super() {
	this->mCollisionComp = nullptr;
	this->mAttackNoise = nullptr;
	this->mSecondSwingMaxTime = 0.7;
	this->mSecondSwingCooldDownTime = 1.0;
	this->mAttackDistance = 100.0;
	this->mAttackSweepRadius = 10.0;
	this->mArmAnimation = EArmEquipment::AE_StunSpear;
	this->mOnlyVisibleToOwner = false;
	this->mDefaultEquipmentActions = 1;
	this->RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}
void AFGEquipmentStunSpear::DoAttack(){ }
void AFGEquipmentStunSpear::HandleDefaultEquipmentActionEvent(EDefaultEquipmentAction action, EDefaultEquipmentActionEvent actionEvent){ }
void AFGEquipmentStunSpear::Server_ShockEnemy_Implementation(const FVector& attackDirection){ }
void AFGEquipmentStunSpear::Server_PlayStunEffects_Implementation(bool secondSwing){ }
void AFGEquipmentStunSpear::Multicast_PlayHitEffects_Implementation(const TArray<FHitResult> &hitResults){ }
void AFGEquipmentStunSpear::Multicast_PlayStunEffects_Implementation(bool secondSwing){ }
