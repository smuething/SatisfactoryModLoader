// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "FactoryGame.h"
#include "UObject/NoExportTypes.h"
#include "FGActorRepresentation.generated.h"

UENUM( BlueprintType )
enum class ERepresentationType : uint8
{
	RT_Default					UMETA( DisplayName = "Default" ),
	RT_Beacon					UMETA( DisplayName = "Beacon" ),
	RT_Crate					UMETA( DisplayName = "Crate" ),
	RT_Hub						UMETA( DisplayName = "Hub" ),
	RT_Ping						UMETA( DisplayName = "Ping" ),
	RT_Player					UMETA( DisplayName = "Player" ),
	RT_RadarTower				UMETA( DisplayName = "RadarTower" ),
	RT_Resource					UMETA( DisplayName = "Resource" ),
	RT_SpaceElevator			UMETA( DisplayName = "SpaceElevator" ),
	RT_StartingPod				UMETA( DisplayName = "StartingPod" ),
	RT_Train					UMETA( DisplayName = "Train" ),
	RT_TrainStation				UMETA( DisplayName = "TrainStation" ),
	RT_Vehicle					UMETA( DisplayName = "Vehicle" ),
	RT_VehicleDockingStation	UMETA( DisplayName = "VehicleDockingStation" ),
	RT_DronePort				UMETA( DisplayName = "DronePort" ),
	RT_Drone					UMETA( DisplayName = "Drone" ),
	RT_MapMarker				UMETA( DisplayName = "MapMarker" ),
	RT_Stamp					UMETA( DisplayName = "Stamp" )
};

UENUM( BlueprintType )
enum class EFogOfWarRevealType : uint8
{
	FOWRT_None					UMETA( DisplayName = "None" ),
	FOWRT_Static				UMETA( DisplayName = "Static" ),
	FOWRT_StaticNoGradient		UMETA( DisplayName = "Static No Gradient" ),
	FOWRT_Dynamic				UMETA( DisplayName = "Dynamic" )
};

UENUM( BlueprintType )
enum class ECompassViewDistance : uint8
{
	CVD_Off				UMETA( DisplayName = "Off" ),
	CVD_Near			UMETA( DisplayName = "Near" ),
	CVD_Mid				UMETA( DisplayName = "Mid" ),
	CVD_Far				UMETA( DisplayName = "Far" ),
	CVD_Always			UMETA( DisplayName = "Always" )
};

// Optimized struct for representation locations. Z Location is not needed nor is the higher precision of 32bit floats
// Net_QuantizedVector is also a nice way to do this but this is even smaller (44bits vs. 60bits)
USTRUCT()
struct FACTORYGAME_API FRepresentationVector2D
{
	GENERATED_BODY()

	// Default Construct No initialization
	FORCEINLINE FRepresentationVector2D() {}

	// Construct from 2 floats
	FRepresentationVector2D( float inX, float inY ) :
		X(inX), 
		Y(inY)
	{ }


	FORCEINLINE FRepresentationVector2D& operator=( const FRepresentationVector2D& other )
	{
		this->X = other.X;
		this->Y = other.Y;

		return *this;
	}

	FORCEINLINE FRepresentationVector2D& operator=( const FVector& other )
	{
		this->X = other.X;
		this->Y = other.Y;

		return *this;
	}

	bool NetSerialize( FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess )
	{
		bOutSuccess = true;
		if( Ar.IsSaving() )
		{
			WriteFixedCompressedFloat<1048576, 22>( X, Ar );
			WriteFixedCompressedFloat<1048576, 22>( Y, Ar );
		}
		else
		{
			ReadFixedCompressedFloat<1048576, 22>( X, Ar );
			ReadFixedCompressedFloat<1048576, 22>( Y, Ar );
		}

		return true;
	}

	// Components
	UPROPERTY()
	float X = 0.f;
	UPROPERTY()
	float Y = 0.f;
};

template<>
struct TStructOpsTypeTraits<FRepresentationVector2D> : public TStructOpsTypeTraitsBase2<FRepresentationVector2D>
{
	enum
	{
		WithNetSerializer = true,
	};
};

/**
 * This object represents an actor in the world. Used in the compass and the minimap.
 */
UCLASS( Blueprintable )
class FACTORYGAME_API UFGActorRepresentation : public UObject
{
	GENERATED_BODY()
	
public:
	/** Mark this class as supported for networking */
	virtual bool IsSupportedForNetworking() const override;
	
	/** Decide on what properties to replicate */
	void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override;

	virtual void SetupActorRepresentation( AActor* realActor, bool isLocal, float lifeSpan = 0.0f );

	virtual void TrySetupDestroyTimer( float lifeSpan );
	
	virtual void RemoveActorRepresentation();

	/** Get the Real actor we represent, might not be relevant on client */
	UFUNCTION( BlueprintPure, Category = "Representation" )
	FORCEINLINE AActor* GetRealActor() const { return mRealActor; }
	
	/** Is this the represented actor static or not */
	UFUNCTION( BlueprintPure, Category = "Representation" )
	FORCEINLINE bool IsActorStatic() const { return mIsStatic; }

	/** Get the location of the represented actor */
	UFUNCTION( BlueprintPure, Category = "Representation" )
	virtual FVector GetActorLocation() const;

	/** Get the rotation of the represented actor */
	UFUNCTION( BlueprintPure, Category = "Representation" )
	FRotator GetActorRotation() const;

	/** This is the image to render in the compass */
	UFUNCTION( BlueprintPure, Category = "Representation" )
	class UTexture2D* GetRepresentationTexture() const;

	/** This is the text to render in the compass */
	UFUNCTION( BlueprintPure, Category = "Representation" )
	FText GetRepresentationText() const;

	/** This is the color to render in the compass */
	UFUNCTION( BlueprintPure, Category = "Representation" )
	FLinearColor GetRepresentationColor() const;

	UFUNCTION( BlueprintPure, Category = "Representation" )
	ERepresentationType GetRepresentationType() const;

	/** If this should be shown in the compass or not*/
	UFUNCTION( BlueprintPure, Category = "Representation" )
	virtual bool GetShouldShowInCompass() const;

	/** If this should be shown on the map or not*/
	UFUNCTION( BlueprintPure, Category = "Representation" )
	bool GetShouldShowOnMap() const;

	UFUNCTION( BlueprintPure, Category = "Representation" )
	EFogOfWarRevealType GetFogOfWarRevealType() const;

	UFUNCTION( BlueprintPure, Category = "Representation" )
	float GetFogOfWarRevealRadius() const;

	void SetIsOnClient( bool onClient );

	UFUNCTION( BlueprintPure, Category = "Representation" )
	virtual ECompassViewDistance GetCompassViewDistance() const;

	UFUNCTION( BlueprintPure, Category = "Representation" )
	virtual bool GetScaleWithMap() const;

	UFUNCTION( BlueprintPure, Category = "Representation" )
	virtual float GetScaleOnMap() const;

	/** Sets the client representations compass view distance directly. It doesn't change the connected actors status so this is only for local updates to avoid waiting for replicated value */
	void SetLocalCompassViewDistance( ECompassViewDistance compassViewDistance );
	
	virtual bool CanBeHighlighted() const;
	
	virtual void SetHighlighted( bool highlighted );

	virtual bool IsHighlighted() const;
	virtual bool IsHighlighted( FLinearColor& out_highlightColor, bool& out_HighlightByLocalPlayer ) const;

	virtual class UFGHighlightedMarker* CreateHighlightedMarker( UObject* owner );

	UFUNCTION( BlueprintPure, Category = "Representation" )
	bool IsHidden() const { return mIsHidden; }
	void SetHidden( bool isHidden );

protected:

	/** Returns a cast of outer */
	class AFGActorRepresentationManager* GetActorRepresentationManager();

	/** This updates the location for this actor representation */
	void UpdateLocation();

	/** This updates the rotation for this actor representation */
	void UpdateRotation();

	/** Updates the representation text for this actor */
	void UpdateRepresentationText();

	/** Updates the representation texture for this actor */
	void UpdateRepresentationTexture();

	/** Updates the color of the representation for this actor */
	void UpdateRepresentationColor();

	/** Updates if this should be shown in the compass or not */
	void UpdateShouldShowInCompass();

	/** Updates if this should be shown on the map or not */
	void UpdateShouldShowOnMap();

	/** Updates the fog of war reveal type */
	void UpdateFogOfWarRevealType();

	/** Updates the fog of war reveal radius */
	void UpdateFogOfWarRevealRadius();

	/** Updates the view distance for this actor on the compass */
	void UpdateCompassViewDistance();

	/** Repnotifies */
	UFUNCTION()
	void OnRep_ShouldShowInCompass();

	UFUNCTION()
	void OnRep_ShouldShowOnMap();

	UFUNCTION()
	void OnRep_ActorRepresentationUpdated();
	
	friend AFGActorRepresentationManager;

	/** This actor representation is locally created */
	bool mIsLocal;

	/** This actor representation resides on a client, used to determine if we want the replicated property or get it from the actor itself. 
	This is used on properties that are replicated with notification */
	bool mIsOnClient;

	/** This is the real actor that this representation represents */
	UPROPERTY( Replicated )
	AActor* mRealActor;

	/** This is the actor location */
	UPROPERTY( Replicated )
	FRepresentationVector2D mActorLocation;

	/** This is the actor location for local representations which uses a regular FVector to get the Z value as well. Used for resource nodes. For Local Use Only */
	FVector mLocalActorLocation;

	/** This is the actor rotation */
	UPROPERTY( Replicated )
	FRotator mActorRotation;

	/** If the actor is static or can be moved */
	UPROPERTY( Replicated )
	bool mIsStatic;

	/** This is the texture to show for this actor representation */
	UPROPERTY( ReplicatedUsing = OnRep_ActorRepresentationUpdated )
	UTexture2D* mRepresentationTexture;

	/** This is the text to show for this actor representation */
	UPROPERTY( ReplicatedUsing = OnRep_ActorRepresentationUpdated )
	FText mRepresentationText;

	/** This is the color used for the representation of this actor */
	UPROPERTY( ReplicatedUsing = OnRep_ActorRepresentationUpdated )
	FLinearColor mRepresentationColor;	

	/** This helps define how this actor representation should be presented */
	UPROPERTY( Replicated )
	ERepresentationType mRepresentationType;

	UPROPERTY( ReplicatedUsing = OnRep_ActorRepresentationUpdated )
	EFogOfWarRevealType mFogOfWarRevealType;

	UPROPERTY( ReplicatedUsing = OnRep_ActorRepresentationUpdated )
	float mFogOfWarRevealRadius;
	
	/** If this should be shown in the compass or not*/
	UPROPERTY( ReplicatedUsing = OnRep_ShouldShowInCompass )
	bool mShouldShowInCompass;

	/** If this should be shown on the map or not*/
	UPROPERTY( ReplicatedUsing = OnRep_ShouldShowOnMap )
	bool mShouldShowOnMap;

	/** If this should be hidden in the map and compass. Still showned in object list in map. Used for pawns that are in a vehicle/train */
	UPROPERTY( ReplicatedUsing = OnRep_ActorRepresentationUpdated )
	bool mIsHidden;

	/** How far away this representation should be shown in the compass */
	UPROPERTY( ReplicatedUsing = OnRep_ActorRepresentationUpdated )
	ECompassViewDistance mCompassViewDistance;
};
