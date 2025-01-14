// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "FactoryGame.h"
#include "CoreMinimal.h"
#include "FGOptionInterface.h"
#include "FGOptionsSettings.h"
#include "Misc/Variant.h"
#include "Engine/DataAsset.h"
#include "FGUserSetting.generated.h"

DECLARE_LOG_CATEGORY_EXTERN( LogUserSetting, Log, All );

UENUM( BlueprintType )
enum class EUserSettingManagers : uint8
{
	USM_OptionsMenu					UMETA( DisplayName = "Options Menu" ),
	USM_Advanced					UMETA( DisplayName = "Advanced Game Settings" ),
	USM_PhotoMode					UMETA( DisplayName = "Photo Mode" )
};

// Disqualifiers if we should not show the setting at all
UENUM( BlueprintType, Meta = ( Bitflags, UseEnumValuesAsMaskValuesInEditor = "true" ) )
enum class ESettingVisiblityDisqualifier : uint8
{
	USAD_None			= 0			UMETA(Hidden),
	USAD_NotOnServer	= 1 << 0	UMETA( DisplayName = "Don't show on server" ),
	USAD_NotOnClient	= 1 << 1	UMETA( DisplayName = "Don't show on client" ),
	USAD_NotInMainMenu	= 1 << 2	UMETA( DisplayName = "Don't show in main menu" ),
	USAD_NotInGame		= 1 << 3	UMETA( DisplayName = "Don't show in game" ),
	USAD_NotForVulkan	= 1 << 4	UMETA( DisplayName = "Don't show when RHI is Vulkan" ),
};

// Disqualifiers if we should allow editing of the setting
UENUM( BlueprintType, Meta = ( Bitflags, UseEnumValuesAsMaskValuesInEditor = "true" ) )
enum class ESettingEditabilityDisqualifier : uint8
{
	USED_None			= 0			UMETA(Hidden),
	USED_NonReversible	= 1 << 0	UMETA( DisplayName = "This setting can't be reversed once changed" )
};

/**
 * 
 */
UCLASS( NotBlueprintable )
class FACTORYGAME_API UFGUserSetting : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFGUserSetting();
	
	// Used for legacy support until all settings are migrated
	FOptionRowData ToOptionRowData() const;

	// Should this option be showed in the current config. Takes into account various things like build config, graphics api, net mode and game mode
	bool ShouldShowInCurrentConfig( UWorld* world ) const;
	
	FVariant GetDefaultValue() const;

	class IFGOptionInterface* GetOptionInterface();

	TSubclassOf< class UFGOptionsValueController > GetValueSelectorWidgetClass() const;

#if WITH_EDITOR
	virtual bool SetupValueFunction( class UK2Node_CallFunction* callFunction, bool isGetterFunction ) const;
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif

	bool HasVisibilityDisqualifier( ESettingVisiblityDisqualifier disqualifier ) const;
	bool HasEditabilityDisqualifier( ESettingEditabilityDisqualifier disqualifier ) const;

private:
	// Should we show this setting in the current build
	bool ShouldShowInBuild() const;
	
public:
	// The identifier in the system. Not shown to player. Used to link this setting with a underlying value in the game
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Setting" )
	FString StrId;

	// Should we manage and if needed create a cvar for this setting based on StrId. Not needed for any functionality so leave it empty if you don't know you want it.
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Setting" )
	bool UseCVar;

	/** The name that is showed for this setting in the UI  */
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Setting" )
	FText DisplayName;

	/** The tooltip that is showed for this setting in the UI  */
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Setting" )
	FText ToolTip;

	/** The category this setting will be placed in when showed in the UI */
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Setting" )
	FText Category;
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Setting" )
	TSubclassOf<class UFGUserSettingCategory> CategoryClass;

	/** The sub category this setting will be placed in when showed in the UI */
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Setting" )
	FText SubCategory;
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Setting" )
	TSubclassOf<class UFGUserSettingCategory> SubCategoryClass;

	/** The order in menus is decided by this value. Lower values means earlier in menu. Negative values are allowed. [-N..0..N]*/
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category = "User Interface" )
	float MenuPriority;

	/** If true this setting affect the whole session and not only the local player */
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Setting" )
	bool IsSettingSessionWide;

	/** The class that we want to use to apply values of this setting. Leave at default if you want a "normal" setting */
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Apply Type" )
	TSoftClassPtr< class UFGUserSettingApplyType > ApplyType;

	/** What kind of value does this setting contain. The widget shown in the UI for this setting is decided by this unless a CustomValueSelectorWidget is defined */ 
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Instanced, Category = "Value Selector"  )
	class UFGUserSetting_ValueSelector* ValueSelector;

	/** Select a custom widget to override the value selctors default widget */ 
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category = "Value Selector"  )
	TSoftClassPtr< class UFGOptionsValueController > CustomValueSelectorWidget;
	
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Availability"  )
	EUserSettingManagers ManagerAvailability;

	// When any of these are true in the current menu we don't show the setting
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Availability", meta=(Bitmask,BitmaskEnum="/Script/FactoryGame.ESettingVisiblityDisqualifier") )
	uint8 VisibilityDisqualifiers;

	// When any of these are true in the current menu we don't allow the user to edit the setting
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Availability", meta=(Bitmask,BitmaskEnum="/Script/FactoryGame.ESettingEditabilityDisqualifier") )
	uint8 EditabilityDisqualifiers;

	// Not used for now but we want support for it later 
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Availability" )
	UFGUserSetting* SubOptionTo;

private:
	/** Slightly misleading name, as this doesn't only apply to builds. If set to Never, it won't show up in editor
	 * 	We could strip the settings from builds but I assumed added settings that is not meant to be in build might rely on the default values
	 * 	so felt safer to keep the setting in the build but hide it.
	 */
	UPROPERTY( EditDefaultsOnly, Category="Availability" )
	EIncludeInBuilds ShowInBuilds = EIncludeInBuilds::IIB_Development;
	
};

UCLASS( Blueprintable, abstract, editinlinenew )
class FACTORYGAME_API UFGUserSetting_ValueSelector : public UObject
{
	GENERATED_BODY()

public:
	virtual TSubclassOf< class UFGOptionsValueController > GetValueSelectorWidgetClass() const;

	// Legacy helper
	virtual EOptionType GetOptionType() const { checkNoEntry(); return EOptionType::OT_Checkbox; }

	virtual FVariant GetDefaultValue() const { return FVariant(); }

#if WITH_EDITOR
	virtual FName GetGraphSchemaName() const;
	virtual bool SetupValueFunction( class UK2Node_CallFunction* callFunction, bool isGetterFunction ) const { return false; }
#endif
	
};

UCLASS( Blueprintable, DefaultToInstanced, editinlinenew, meta = ( DisplayName = "CheckBox" ) )
class FACTORYGAME_API UFGUserSetting_CheckBox : public UFGUserSetting_ValueSelector
{
	GENERATED_BODY()
public:
	virtual TSubclassOf< class UFGOptionsValueController > GetValueSelectorWidgetClass() const override;

	virtual EOptionType GetOptionType() const override { return EOptionType::OT_Checkbox; }
	
	virtual FVariant GetDefaultValue() const override { return FVariant(DefaultCheckBoxValue); }

#if WITH_EDITOR
	virtual FName GetGraphSchemaName() const override;

	virtual bool SetupValueFunction( class UK2Node_CallFunction* callFunction, bool isGetterFunction ) const override;
#endif
	
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Value" )
	bool DefaultCheckBoxValue;

};

// @todok2 this needs more work and testing to make sure it works properly when changing enum and similar
// We need to add support for enum display names as FText so they can be translated.
// Maybe this should just be a dervied UFGUserSetting_IntSelector with some magic to populate values from a selected enum
UCLASS( Abstract, editinlinenew, meta = ( DisplayName = "Enum Selector" )  )
class FACTORYGAME_API UFGUserSetting_EnumSelector : public UFGUserSetting_ValueSelector
{
	GENERATED_BODY()

public:
	virtual TSubclassOf< class UFGOptionsValueController > GetValueSelectorWidgetClass() const override;

	virtual EOptionType GetOptionType() const override { return EOptionType::OT_IntegerSelection; }

	// @todok2 cache this?
	TArray<FIntegerSelection> GetIntegerSelectionValues( int32& out_DefaultSelectionIndex );

	UFUNCTION()
	TArray<FString> GetEnumOptions() const;

	// @todok2 Add support for GetDefaultValue

	// Can we replace or at least cache this with a enum*
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Value" )
	FString EnumAsString;

	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Value", meta=(GetOptions="GetEnumOptions") )
	FString DefaultEnumValue;
};

UCLASS( Blueprintable, DefaultToInstanced, editinlinenew, meta = ( DisplayName = "Integer Selector" ) )
class FACTORYGAME_API UFGUserSetting_IntSelector : public UFGUserSetting_ValueSelector
{
	GENERATED_BODY()

	UFGUserSetting_IntSelector();

public:
	virtual TSubclassOf< class UFGOptionsValueController > GetValueSelectorWidgetClass() const override;

	virtual EOptionType GetOptionType() const override { return EOptionType::OT_IntegerSelection; }

	virtual FVariant GetDefaultValue() const override { return FVariant(Defaultvalue); }

#if WITH_EDITOR
	virtual FName GetGraphSchemaName() const override;

	virtual bool SetupValueFunction( class UK2Node_CallFunction* callFunction, bool isGetterFunction ) const override;
#endif
	
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Value" )
	int32 Defaultvalue;

	/** The values that this setting can be set to. Notice that the index is only for the order. The FIntegerSelection::Value can be set to any int32 value */
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Value" )
	TArray<FIntegerSelection> IntegerSelectionValues;

	/** When his is true the user can't select the last index manually. It has to be set programmatically */
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Value" )
	bool BlockLastIndexFromManualSelection;

	/** When last index is selected from automatic selection the user can't change the setting. Only valid if BlockLastIndexFromManualSelection is enabled */
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Value", meta = (EditCondition = "BlockLastIndexFromManualSelection", EditConditionHides) )
	bool LockLastIndexWhenSelected;

	/** Should this integer selection be shown as a dropdown instead of a  */
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Value" )
	bool ShowAsDropdown;
};



UCLASS( Blueprintable, DefaultToInstanced, editinlinenew, meta = ( DisplayName = "Slider" ) )
class FACTORYGAME_API UFGUserSetting_Slider : public UFGUserSetting_ValueSelector
{
	GENERATED_BODY()
	
public:
	virtual TSubclassOf< class UFGOptionsValueController > GetValueSelectorWidgetClass() const override;

	virtual EOptionType GetOptionType() const override { return EOptionType::OT_Slider; }

	virtual FVariant GetDefaultValue() const override;

#if WITH_EDITOR
	virtual bool SetupValueFunction( class UK2Node_CallFunction* callFunction, bool isGetterFunction ) const override;
#endif
	
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Value" )
	float MinValue = 0.f;

	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Value" )
	float MaxValue = 1.f;

	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Value" )
	float MinDisplayValue = 0.f;

	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Value" )
	float MaxDisplayValue = 1.f;

	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Value" )
	int32 MaxFractionalDigits = 1;

	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Value" )
	bool ShowZeroAsOff;

	// The default value the slider will show. This should be in the MinDisplayValue - MaxDisplayValue range
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, Category="Value" )
	float DefaultSliderValue = 0.5f;
};

UCLASS( Blueprintable, DefaultToInstanced, editinlinenew, meta = ( DisplayName = "Custom" ) )
class FACTORYGAME_API UFGUserSetting_Custom : public UFGUserSetting_ValueSelector
{
	GENERATED_BODY()
public:
	virtual EOptionType GetOptionType() const override { return EOptionType::OT_Custom; }

	// @todok2 Add support for GetDefaultValue
};
