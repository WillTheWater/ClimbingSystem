// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SRS_MovementComponent.generated.h"

UENUM(BlueprintType)
namespace ECustomMovementMode
{
	enum Type
	{
		MOVE_Climb UMETA(DisplayName = "Climb Mode"),
	};
}

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CLIMBINGSYSTEM_API USRS_MovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	bool TraceClimbableSurfaces();
	FHitResult TraceFromEyeHeight(float TraceDistance, float StartOffset = 0.f);

	void ToggleClimbing(bool bEnableClimbing);
	bool IsClimbing() const;
	bool CanClimb();
	void StartClimbing();
	void StopClimbing();
	void PhysClimbing(float DeltaTime, int32 Iterations);
	void ProcessClimbableSurface();

	TArray<FHitResult> ClimbableSurfacesHits;

protected:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
	
private:
	TArray<FHitResult> DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End, bool bShowShape = false, bool bDrawPersistent = false);
	FHitResult DoLineTraceSingleByObject(const FVector& Start, const FVector& End, bool bShowShape = false, bool bDrawPersistent = false);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climbing", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery>> ClimbObjectTypes;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climbing", meta = (AllowPrivateAccess = "true"))
	float ClimbCapsuleRadius { 50.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climbing", meta = (AllowPrivateAccess = "true"))
	float ClimbCapsuleHeight { 72.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climbing", meta = (AllowPrivateAccess = "true"))
	float MaxBreakClimbDeceleration { 400.f };

	FVector CurrentClimbableSurfaceLocation { FVector::ZeroVector };
	FVector CurrentClimbableSurfaceNormal { FVector::ZeroVector };
};
