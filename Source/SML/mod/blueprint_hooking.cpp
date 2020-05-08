#include "blueprint_hooking.h"
#include "toolkit/BPCodeDumper.h"
#include "BPHookHelper.h"
#include "util/Logging.h"

struct FHookKey {
	int64 HookFunctionAddress;
	int32 HookOffset;
	
	bool operator==(const FHookKey& Other) const {
		return HookFunctionAddress == Other.HookFunctionAddress && HookOffset == Other.HookOffset;
	}
};

FORCEINLINE uint32 GetTypeHash(const FHookKey& Key) {
	return FCrc::MemCrc32(&Key, sizeof(FHookKey));
}

struct FHookEntry {
	TArray<std::function<HookSignature>> Hooks;
};

static TMap<FHookKey, FHookEntry> RegisteredHooks;

FHookEntry& GetOrAddHookEntry(const FHookKey& SearchKey, bool& EntryAdded) {
	FHookEntry* ExistingEntry = RegisteredHooks.Find(SearchKey);
	if (ExistingEntry)
		return *ExistingEntry;
	EntryAdded = true;
	return RegisteredHooks.Add(SearchKey);
}

void HandleHookedFunctionCall(FFrame& Stack, int64 HookedFunctionAddress, int32 HookOffset) {
	const FHookKey SearchKey{ HookedFunctionAddress, HookOffset };
	FHookEntry* HookEntry = RegisteredHooks.Find(SearchKey);
	checkf(HookEntry, TEXT("HandleHookedFunctionCall on unhooked function"));
	FBlueprintHookHelper HookHelper{ Stack };
	//SML::Logging::info(TEXT("HandleHookedFunctionCall: Hooked Function Address: "), HookedFunctionAddress, TEXT(", Hook Offset: "), HookOffset);
	for (const std::function<HookSignature>& Hook : HookEntry->Hooks) {
		Hook(HookHelper);
	}
}

#define WRITE_UNALIGNED(Arr, Type, Value) \
	Arr.AddUninitialized(sizeof(Type)); \
	FPlatformMemory::WriteUnaligned<Type>(&AppendedCode[Arr.Num() - sizeof(Type)], (Type) Value);

void InstallBlueprintHook(UFunction* Function, const FHookKey& HookKey) {
	TArray<uint8>& OriginalCode = Function->Script;
	//basically EX_Jump + CodeSkipSizeType;
	const int32 MinBytesRequired = 1 + sizeof(CodeSkipSizeType);
	const int32 JumpDestination = SML::ComputeStatementReplaceOffset(Function, MinBytesRequired, HookKey.HookOffset);
	checkf(JumpDestination != -1, TEXT("Cannot install hook on that function"));
	const int32 BytesToMove = JumpDestination - HookKey.HookOffset;
	//SML::Logging::info(TEXT("InstallBlueprintHook: Address: "), reinterpret_cast<int64>(Function));
	//SML::Logging::info(TEXT("InstallBlueprintHook: Min Bytes: "), MinBytesRequired, TEXT(", Hook Offset: "), HookKey.HookOffset, TEXT(", Jump Destination: "), JumpDestination);

	//Generate code to call function & code we stripped by jump
	TArray<uint8> AppendedCode;
	UFunction* HookCallFunction = UBPHookHelper::StaticClass()->FindFunctionByName(TEXT("ExecuteBPHook"));
	check(HookCallFunction);
	//EX_CallMath opcode to call static function & write it's address
	AppendedCode.Add(EX_CallMath);
	WRITE_UNALIGNED(AppendedCode, ScriptPointerType, HookCallFunction);
	//Pass hook function address & hook offset
	AppendedCode.Add(EX_Int64Const);
	WRITE_UNALIGNED(AppendedCode, int64, HookKey.HookFunctionAddress);
	AppendedCode.Add(EX_IntConst);
	WRITE_UNALIGNED(AppendedCode, int32, HookKey.HookOffset);
	//Indicate end of function parameters
	AppendedCode.Add(EX_EndFunctionParms);
	//Append original code we stripped
	AppendedCode.AddUninitialized(BytesToMove);
	FPlatformMemory::Memcpy(&AppendedCode[AppendedCode.Num() - BytesToMove], &OriginalCode[HookKey.HookOffset], BytesToMove);
	//And then jump to the original location
	AppendedCode.Add(EX_Jump);
	WRITE_UNALIGNED(AppendedCode, CodeSkipSizeType, JumpDestination);
	//Also EX_EndOfScript in the end for safety
	AppendedCode.Add(EX_EndOfScript);

	//Now, append our code to the end of the function
	const int32 StartOfAppendedCode = OriginalCode.Num();
	OriginalCode.Append(AppendedCode);
	//And actually replace start code with jump
	//But first, fill it all with EX_EndOfScript (for safety, again)
	FPlatformMemory::Memset(&OriginalCode[HookKey.HookOffset], EX_EndOfScript, BytesToMove);
	OriginalCode[HookKey.HookOffset] = EX_Jump;
	FPlatformMemory::WriteUnaligned<CodeSkipSizeType>(&OriginalCode[HookKey.HookOffset + 1], StartOfAppendedCode);
	//SML::Logging::info(TEXT("Inserted EX_Jump at "), HookKey.HookOffset, TEXT(" to "), StartOfAppendedCode, TEXT(" (Appended Code Size: "), AppendedCode.Num(), TEXT(")"));
}

int32 PreProcessHookOffset(UFunction* Function, int32 HookOffset) {
	if (HookOffset == EPredefinedHookOffset::Return) {
		//For now Kismet Compiler will always generate only one Return node, so all
		//execution paths will end up either with executing it directly or jumping to it
		//So we need to hook only in one place to handle all possible execution paths
		int32 ReturnOffset = SML::FindReturnStatementOffset(Function);
		checkf(ReturnOffset != -1, TEXT("EX_Return not found for function"));
		return ReturnOffset;
	}
	return HookOffset;
}

SML_API void HookBlueprintFunction(UFunction* Function, std::function<HookSignature> Hook, int32 HookOffset) {
#if !WITH_EDITOR
	checkf(Function->Script.Num(), TEXT("HookBPFunction: Function provided is not implemented in BP"));
	//Make sure to add outer UClass to root set to avoid it being Garbage Collected
	//Because otherwise after GC script byte code will be reloaded, without our hooks applied
	Function->GetTypedOuter<UClass>()->AddToRoot();
	
	SML::Logging::info(TEXT("Hooking blueprint implemented function "), *Function->GetPathName());
	HookOffset = PreProcessHookOffset(Function, HookOffset);
	const FHookKey SearchKey{ reinterpret_cast<int64>(Function), HookOffset };
	bool HookEntryAdded = false;
	FHookEntry& HookEntry = GetOrAddHookEntry(SearchKey, HookEntryAdded);
	if (HookEntryAdded) {
		//Entry was just added, we need to install hook now
		InstallBlueprintHook(Function, SearchKey);
	}
	//Register our provided hook now
	HookEntry.Hooks.Add(Hook);
#endif
}