#pragma once

#pragma once

#include <HookLoader.h>
#include <util/FString.h>

#pragma warning (push)
#pragma warning (disable: 4667)

class UGameInstance {
public:
	class UWorld* GetWorld();
};

DEFINE_METHOD(UGameInstance::GetWorld);

class UGameplayStatics {
public:
	static class APawn* GetPlayerPawn(class UObject* WorldContextObject, int PlayerIndex);
};

DEFINE_METHOD(UGameplayStatics::GetPlayerPawn);

class AFGPlayerController {
public:
	void BeginPlay();
	void EnterChatMessage(FString *inMessage);
	void OnAttentionPingPressed();
	void Suicide();
};

DEFINE_METHOD(AFGPlayerController::BeginPlay);
DEFINE_METHOD(AFGPlayerController::EnterChatMessage);
DEFINE_METHOD(AFGPlayerController::OnAttentionPingPressed);
DEFINE_METHOD(AFGPlayerController::Suicide);

class UFGInventoryComponent {
public:
	void SortInventory();
};

DEFINE_METHOD(UFGInventoryComponent::SortInventory);

#pragma warning (pop)