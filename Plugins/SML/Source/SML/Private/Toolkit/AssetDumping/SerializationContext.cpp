#include "Toolkit/AssetDumping/SerializationContext.h"
#include "Toolkit/ObjectHierarchySerializer.h"
#include "Toolkit/PropertySerializer.h"
#include "Toolkit/AssetDumping/AssetDumpProcessor.h"

UObject* FindObjectFastFailsafe(UPackage* Package, const FString& ObjectName) {
	//Make sure package is fully loading before trying to retrieve objects from it
	Package->FullyLoad();

	//Try normal FindObjectFast now that package is loaded
	UObject* FastResult = FindObjectFast<UObject>(Package, *ObjectName);
	if (FastResult) {
		return FastResult;
	}

	UE_LOG(LogAssetDumper, Error, TEXT("Failed to find Asset Object '%s' inside of the Package '%s'"), *ObjectName, *Package->GetPathName());
	UE_LOG(LogAssetDumper, Error, TEXT("Package Fully Loaded? %d"), Package->IsFullyLoaded());
	
	checkf(0, TEXT("Failed to resolve object '%s' inside of the package '%s'"), *ObjectName, *Package->GetPathName());
	return NULL;
}

UObject* ResolveBlueprintClassAsset(UPackage* Package, const FAssetData& AssetData) {
	FString GeneratedClassExportedPath;
	if (!AssetData.GetTagValue(FBlueprintTags::GeneratedClassPath, GeneratedClassExportedPath)) {
		checkf(0, TEXT("GetBlueprintAsset called on non-blueprint asset"));
		return NULL;
	}
		
	//Make sure export path represents a valid path and convert it to pure object path
	FString GeneratedClassPath;
	check(FPackageName::ParseExportTextPath(GeneratedClassExportedPath, NULL, &GeneratedClassPath));
	const FString BlueprintClassObjectName = FPackageName::ObjectPathToObjectName(GeneratedClassPath);

	//Load UBlueprintGeneratedClass for provided object and make sure it has been loaded
	UObject* ClassObject = FindObjectFastFailsafe(Package, BlueprintClassObjectName);
	checkf(ClassObject, TEXT("Failed to find Generated Class %s inside of the package %s"), *BlueprintClassObjectName, *Package->GetPathName());
	return ClassObject;
}

UObject* ResolveGenericAsset(UPackage* Package, const FAssetData& AssetData) {
	UObject* AssetObject = FindObjectFastFailsafe(Package, AssetData.AssetName.ToString());
    checkf(AssetObject, TEXT("Failed to find Asset %s inside of the package %s"), *AssetData.AssetName.ToString(), *Package->GetPathName());
    return AssetObject;
}

FSerializationContext::FSerializationContext(const FString& RootOutputDirectory, const FAssetData& AssetData, UPackage* Package) {
	this->AssetSerializedData = MakeShareable(new FJsonObject());
	this->PropertySerializer = NewObject<UPropertySerializer>();
	this->ObjectHierarchySerializer = NewObject<UObjectHierarchySerializer>();
	this->ObjectHierarchySerializer->SetPropertySerializer(PropertySerializer);

	this->PropertySerializer->AddToRoot();
	this->ObjectHierarchySerializer->AddToRoot();

	//Object hierarchy serializer will also root package object by referencing it
	this->ObjectHierarchySerializer->InitializeForSerialization(Package);
	this->Package = Package;
	this->AssetData = AssetData;

	this->RootOutputDirectory = RootOutputDirectory;
	this->PackageBaseDirectory = FPaths::Combine(RootOutputDirectory, AssetData.PackagePath.ToString());

	//Make sure package base directory exists
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*PackageBaseDirectory);

	//Check whenever asset represents a blueprint, then we want to work with blueprint asset specific methods
	if (AssetData.TagsAndValues.Contains(FBlueprintTags::GeneratedClassPath)) {
		this->AssetObject = ResolveBlueprintClassAsset(Package, AssetData);
	} else {
		//No GeneratedClassPath, it is an ordinary asset object
		this->AssetObject = ResolveGenericAsset(Package, AssetData);
	}
	
	//Set mark on the asset object so it can be referenced by other objects in hierarchy
	check(AssetObject);
	this->ObjectHierarchySerializer->SetObjectMark(AssetObject, TEXT("$AssetObject$"));
}

FSerializationContext::~FSerializationContext() {
	this->PropertySerializer->RemoveFromRoot();
	this->ObjectHierarchySerializer->RemoveFromRoot();

	this->PropertySerializer->MarkPendingKill();
	this->ObjectHierarchySerializer->MarkPendingKill();
}

void FSerializationContext::Finalize() const {
	TSharedRef<FJsonObject> RootObject = MakeShareable(new FJsonObject());
	RootObject->SetStringField(TEXT("AssetClass"), AssetData.AssetClass.ToString());
	RootObject->SetStringField(TEXT("AssetPackage"), Package->GetName());
	RootObject->SetStringField(TEXT("AssetName"), AssetData.AssetName.ToString());
	
	RootObject->SetObjectField(TEXT("AssetSerializedData"), AssetSerializedData);
	RootObject->SetArrayField(TEXT("ObjectHierarchy"), ObjectHierarchySerializer->FinalizeSerialization());

	FString ResultString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResultString);
	FJsonSerializer::Serialize(RootObject, Writer);

	const FString OutputFilename = GetDumpFilePath(TEXT(""), TEXT("json"));
	check(FFileHelper::SaveStringToFile(ResultString, *OutputFilename));
}
