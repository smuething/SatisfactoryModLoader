#pragma once
#include "Toolkit/AssetGeneration/AssetTypeGenerator.h"
#include "Texture2DGenerator.generated.h"

UCLASS(MinimalAPI)
class UTexture2DGenerator : public UAssetTypeGenerator {
	GENERATED_BODY()
protected:
	virtual UPackage* CreateAssetPackage() override;
	virtual void OnExistingPackageLoaded() override;
	void RebuildTextureData(class UTexture2D* Texture);
public:
	virtual FName GetAssetClass() override;
};