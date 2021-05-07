#pragma once
#include "CoreMinimal.h"

class SMLEDITOR_API FAssetGenerationUtil {
public:
	static void ConvertPropertyObjectToGraphPinType(const TSharedPtr<FJsonObject> PropertyObject, FEdGraphPinType& OutPinType, class UObjectHierarchySerializer* ObjectSerializer);

	static void GetPropertyDependencies(const TSharedPtr<FJsonObject> PropertyObject, UObjectHierarchySerializer* ObjectSerializer, TArray<FString>& OutDependencies);

	static bool PopulateStructVariable(const TSharedPtr<FJsonObject>& PropertyObject, UObjectHierarchySerializer* ObjectSerializer, struct FStructVariableDescription& OutStructVariable);

	static bool AreStructDescriptionsEqual(const FStructVariableDescription& A, const FStructVariableDescription& B);
};