#include "Toolkit/AssetTypeGenerator/Texture2DGenerator.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "AssetGeneration/AssetGeneratorSettings.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Toolkit/ObjectHierarchySerializer.h"

UPackage* UTexture2DGenerator::CreateAssetPackage() {
	UPackage* NewPackage = CreatePackage(NULL, *GetPackageName().ToString());
	UTexture2D* NewTexture = NewObject<UTexture2D>(NewPackage, GetAssetName(), RF_Public | RF_Standalone);
	
	RebuildTextureData(NewTexture);
	return NewPackage;
}

FString ComputeTextureHash(UTexture2D* Texture) {
	TArray64<uint8> OutSourceMipMapData;
	check(Texture->Source.GetMipData(OutSourceMipMapData, 0));

	FString TextureHash = FMD5::HashBytes(OutSourceMipMapData.GetData(), OutSourceMipMapData.Num() * sizeof(uint8));
	TextureHash.Append(FString::Printf(TEXT("%llx"), OutSourceMipMapData.Num()));
	return TextureHash;
}

void UTexture2DGenerator::OnExistingPackageLoaded() {
	UTexture2D* ExistingTexture = GetAsset<UTexture2D>();

	const FString SourceFileHash = GetAssetData()->GetStringField(TEXT("SourceImageHash"));
	const FString CurrentFileHash = ComputeTextureHash(ExistingTexture);

	if (SourceFileHash != CurrentFileHash) {
		UE_LOG(LogAssetGenerator, Log, TEXT("Refreshing source art data for Texture2D %s, OldSignature = %s, NewSignature = %s"), *ExistingTexture->GetPathName(), *CurrentFileHash, *SourceFileHash);
		RebuildTextureData(ExistingTexture);
	}
}

void UTexture2DGenerator::RebuildTextureData(UTexture2D* Texture) {
	const int32 TextureWidth = GetAssetData()->GetIntegerField(TEXT("TextureWidth"));
	const int32 TextureHeight = GetAssetData()->GetIntegerField(TEXT("TextureHeight"));

    //Reinitialize texture data with new dimensions and format
	Texture->Source.Init2DWithMipChain(TextureWidth, TextureHeight, ETextureSourceFormat::TSF_BGRA8);
	
	//Read contents of the PNG file provided with the dump and decompress it
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	const FString ImageFilePath = GetAdditionalDumpFilePath(TEXT(""), TEXT("png"));
	TArray<uint8> CompressedFileData;
	checkf(FFileHelper::LoadFileToArray(CompressedFileData, *ImageFilePath), TEXT("Failed to read dump image file %s"), *ImageFilePath);

	check(ImageWrapper->SetCompressed(CompressedFileData.GetData(), CompressedFileData.Num() * sizeof(uint8)));
	CompressedFileData.Empty();

	TArray64<uint8> ResultUncompressedData;
	check(ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, ResultUncompressedData));

	//Populate first texture mipmap with the decompressed data from the file
	uint8* LockedMipData = Texture->Source.LockMip(0);
	
	const int64 MipMapSize = Texture->Source.CalcMipSize(0);
	check(ResultUncompressedData.Num() == MipMapSize);
	FMemory::Memcpy(LockedMipData, ResultUncompressedData.GetData(), MipMapSize);

	Texture->Source.UnlockMip(0);

	//Apply settings from the serialized texture object
	const TSharedPtr<FJsonObject> TextureProperties = GetAssetData()->GetObjectField(TEXT("AssetObjectData"));
	GetObjectSerializer()->DeserializeObjectProperties(TextureProperties.ToSharedRef(), Texture);

	//Force no MipMaps policy if specified in the settings
	const UAssetGeneratorSettings* Settings = UAssetGeneratorSettings::Get();
	if (Settings->PackagesToForceNoMipMaps.Contains(GetPackageName().ToString())) {
		Texture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	}

	//Update texture resource, which will update existing resource, invalidate platform data and rebuild it
	Texture->UpdateResource();

	MarkAssetChanged();
}

FName UTexture2DGenerator::GetAssetClass() {
	return UTexture2D::StaticClass()->GetFName();
}
