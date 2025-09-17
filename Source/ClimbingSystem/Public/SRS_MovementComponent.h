// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SRS_MovementComponent.generated.h"

class UAnimMontage;
class UAnimInstance;

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
	bool ShouldStopClimbing();
	bool CheckHasReachedGround();
	FQuat GetClimbRotation(float DeltaTime);
	void SnapToClimbableSurface(float DeltaTime);
	bool HasReachLedge();
	void PlayClimbMontage(UAnimMontage* MontageToPlay);

	UFUNCTION()
	void OnClimbMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	TArray<FHitResult> ClimbableSurfacesHits;

	FORCEINLINE FVector GetClimbableSurfaceLocation() const { return CurrentClimbableSurfaceLocation; }
	FORCEINLINE FVector GetClimbableSurfaceNormal() const { return CurrentClimbableSurfaceNormal; }
	FVector GetUnrotatedClimbVelocity() const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;
	virtual FVector ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity, const FVector& CurrentVelocity) const override;
	
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climbing", meta = (AllowPrivateAccess = "true"))
	float MaxClimbSpeed { 100.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climbing", meta = (AllowPrivateAccess = "true"))
	float MaxClimbAcceleration { 300.f };

	FVector CurrentClimbableSurfaceLocation { FVector::ZeroVector };
	FVector CurrentClimbableSurfaceNormal { FVector::ZeroVector };

	UPROPERTY()
	UAnimInstance* OwningPlayerAnimInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climbing", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* IdleToClimb;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climbing", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ClimbUpLedge;
};
