// This file has been automatically generated by the Unreal Header Implementation tool

#include "Replication/FGReplicationDetailActor_ResourceSink.h"

AFGReplicationDetailActor_ResourceSink::AFGReplicationDetailActor_ResourceSink() : Super() {
	this->mCouponInventory = nullptr;
}
void AFGReplicationDetailActor_ResourceSink::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFGReplicationDetailActor_ResourceSink, mCouponInventory);
}
void AFGReplicationDetailActor_ResourceSink::InitReplicationDetailActor( AFGBuildable* owningActor){ }
void AFGReplicationDetailActor_ResourceSink::FlushReplicationActorStateToOwner(){ }
bool AFGReplicationDetailActor_ResourceSink::HasCompletedInitialReplication() const{ return bool(); }
