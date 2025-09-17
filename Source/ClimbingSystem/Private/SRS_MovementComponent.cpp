// Fill out your copyright notice in the Description page of Project Settings.


#include "ClimbingSystem/Public/SRS_MovementComponent.h"

#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ClimbingSystem/Debugger/DebugHelper.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"

TArray<FHitResult> USRS_MovementComponent::DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End,
                                                                       bool bShowShape, bool bDrawPersistent)
{
	TArray<FHitResult> Hits;
	EDrawDebugTrace::Type DrawDebugType = EDrawDebugTrace::None;
	if (bShowShape)
	{
		DrawDebugType = bDrawPersistent ? EDrawDebugTrace::Persistent : EDrawDebugTrace::ForOneFrame;
	}
	UKismetSystemLibrary::CapsuleTraceMultiForObjects
	(
		this,
		Start,
		End,
		ClimbCapsuleRadius,
		ClimbCapsuleHeight,
		ClimbObjectTypes,
		false,
		TArray<AActor*>(),
		DrawDebugType,
		Hits,
		false
	);
	return Hits;
}

FHitResult USRS_MovementComponent::DoLineTraceSingleByObject(const FVector& Start, const FVector& End, bool bShowShape,
	bool bDrawPersistent)
{
	FHitResult Hit;
	EDrawDebugTrace::Type DrawDebugType = EDrawDebugTrace::None;
	if (bShowShape)
	{
		DrawDebugType = bDrawPersistent ? EDrawDebugTrace::Persistent : EDrawDebugTrace::ForOneFrame;
	}
	UKismetSystemLibrary::LineTraceSingleForObjects
	(
		this,
		Start,
		End,
		ClimbObjectTypes,
		false,
		TArray<AActor*>(),
		DrawDebugType,
		Hit,
		false
	);
	return Hit;
}

FVector USRS_MovementComponent::GetUnrotatedClimbVelocity() const
{
	return UKismetMathLibrary::Quat_UnrotateVector(UpdatedComponent->GetComponentQuat(), Velocity);
}

void USRS_MovementComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
	if (OwningPlayerAnimInstance)
	{
		OwningPlayerAnimInstance->OnMontageEnded.AddDynamic(this, &ThisClass::OnClimbMontageEnded);
		OwningPlayerAnimInstance->OnMontageBlendingOut.AddDynamic(this, &ThisClass::OnClimbMontageEnded);
	}
}

void USRS_MovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void USRS_MovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	if (IsClimbing())
	{
		bOrientRotationToMovement = false;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(48.f);
	}
	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == ECustomMovementMode::MOVE_Climb)
	{
		bOrientRotationToMovement = true;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(96.f);
		const FRotator DirtyRotation = UpdatedComponent->GetComponentRotation();
		const FRotator CleanRotation = FRotator(0.f, DirtyRotation.Yaw, 0.f);
		UpdatedComponent->SetRelativeRotation(CleanRotation);
		StopMovementKeepPathing();
	}
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

void USRS_MovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	if (IsClimbing())
	{
		PhysClimbing(DeltaTime, Iterations);		
	}
	Super::PhysCustom(DeltaTime, Iterations);
}

float USRS_MovementComponent::GetMaxSpeed() const
{
	if (IsClimbing())
	{
		return MaxClimbSpeed;
	}
	return Super::GetMaxSpeed();
}

float USRS_MovementComponent::GetMaxAcceleration() const
{
	if (IsClimbing())
	{
		return MaxClimbAcceleration;
	}
	return Super::GetMaxAcceleration();
}

FVector USRS_MovementComponent::ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity,
	const FVector& CurrentVelocity) const
{
	const bool bIsPlayingClimbMontage = IsFalling() && OwningPlayerAnimInstance && OwningPlayerAnimInstance->IsAnyMontagePlaying();
	if (bIsPlayingClimbMontage)
	{
		return RootMotionVelocity;
	}
	return Super::ConstrainAnimRootMotionVelocity(RootMotionVelocity, CurrentVelocity);
	
}

bool USRS_MovementComponent::TraceClimbableSurfaces()
{
	const FVector StartOffset = UpdatedComponent->GetForwardVector() * 30.f;
	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
	const FVector End = Start + UpdatedComponent->GetForwardVector();
	ClimbableSurfacesHits = DoCapsuleTraceMultiByObject
	(
		Start,
		End,
		true
	);
	return !ClimbableSurfacesHits.IsEmpty();
}

FHitResult USRS_MovementComponent::TraceFromEyeHeight(float TraceDistance, float StartOffset)
{
	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
	const FVector EyeHeightOffset = UpdatedComponent->GetUpVector() * (CharacterOwner->BaseEyeHeight + StartOffset);
	const FVector Start = ComponentLocation + EyeHeightOffset;
	const FVector End = Start + UpdatedComponent->GetForwardVector() * TraceDistance;
	return DoLineTraceSingleByObject
	(
		Start,
		End
	);
}

void USRS_MovementComponent::ToggleClimbing(bool bEnableClimbing)
{
	if (bEnableClimbing)
	{
		if (CanClimb())
		{
			PlayClimbMontage(IdleToClimb);
		}
		else if (CanClimbDown())
		{
			PlayClimbMontage(ClimbDownLedge);
		}
	}
	else
	{
		StopClimbing();
	}
}

bool USRS_MovementComponent::IsClimbing() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == ECustomMovementMode::MOVE_Climb;
}

bool USRS_MovementComponent::CanClimb()
{
	if (IsFalling()) { return false; }
	if (!TraceClimbableSurfaces()) { return false; }
	if (!TraceFromEyeHeight(100.f, 0.f).bBlockingHit) { return false; }
	return true;
}

void USRS_MovementComponent::StartClimbing()
{
	SetMovementMode(MOVE_Custom, ECustomMovementMode::MOVE_Climb);
}

bool USRS_MovementComponent::CanClimbDown()
{
	if (IsFalling()) { return false; }
	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
	const FVector ComponentForward = UpdatedComponent->GetForwardVector();
	const FVector ComponentDown = -UpdatedComponent->GetUpVector();
	const FVector Start = ComponentLocation + ComponentForward * ClimbDownTraceDistance;
	const FVector End = Start + ComponentDown * 100.f;
	FHitResult Hit = DoLineTraceSingleByObject(Start, End);
	const FVector LedgeStart = Hit.TraceStart + ComponentForward * LedgeTraceDistance;
	const FVector LedgeEnd = LedgeStart + ComponentDown * 200.f;
	FHitResult LedgeHit = DoLineTraceSingleByObject(LedgeStart, LedgeEnd);
	if (Hit.bBlockingHit && !LedgeHit.bBlockingHit)
	{
		return true;
	}
	return false;
}

void USRS_MovementComponent::StopClimbing()
{
	SetMovementMode(MOVE_Falling);
}

void USRS_MovementComponent::PhysClimbing(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	TraceClimbableSurfaces();
	ProcessClimbableSurface();

	if (ShouldStopClimbing() || CheckHasReachedGround())
	{
		StopClimbing();
		return;
	}
	
	RestorePreAdditiveRootMotionVelocity();

	if( !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() )
	{
		CalcVelocity(DeltaTime, 0.f, true, MaxBreakClimbDeceleration);
	}

	ApplyRootMotionToVelocity(DeltaTime);

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Adjusted = Velocity * DeltaTime;
	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Adjusted, GetClimbRotation(DeltaTime), true, Hit);

	if (Hit.Time < 1.f)
	{
		HandleImpact(Hit, DeltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}

	if(!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() )
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / DeltaTime;
	}
	SnapToClimbableSurface(DeltaTime);
	if (HasReachLedge())
	{
		PlayClimbMontage(ClimbUpLedge);
	}
}

void USRS_MovementComponent::ProcessClimbableSurface()
{
	CurrentClimbableSurfaceLocation = FVector::ZeroVector;
	CurrentClimbableSurfaceNormal = FVector::ZeroVector;

	if (ClimbableSurfacesHits.IsEmpty()) { return; }

	for (const FHitResult& Hit : ClimbableSurfacesHits)
	{
		CurrentClimbableSurfaceLocation += Hit.ImpactPoint;
		CurrentClimbableSurfaceNormal += Hit.ImpactNormal;
	}

	CurrentClimbableSurfaceLocation /= ClimbableSurfacesHits.Num();
	CurrentClimbableSurfaceNormal = CurrentClimbableSurfaceNormal.GetSafeNormal();
}

bool USRS_MovementComponent::ShouldStopClimbing()
{
	if (ClimbableSurfacesHits.IsEmpty()) { return true; }

	const float DotResult = FVector::DotProduct(CurrentClimbableSurfaceNormal, FVector::UpVector);
	const float DegreeDiff = FMath::RadiansToDegrees(FMath::Acos(DotResult));
	return DegreeDiff <= 60.f;
}

bool USRS_MovementComponent::CheckHasReachedGround()
{
	const FVector DownVector = -UpdatedComponent->GetUpVector();
	const FVector StartOffset = DownVector * 120.f;
	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
	const FVector End = Start + DownVector;
	TArray<FHitResult> Hits = DoCapsuleTraceMultiByObject(Start, End);
	if (Hits.IsEmpty()) { return false; }
	for (const FHitResult& Hit : Hits)
	{
		const bool bFloorReached = FVector::Parallel(-Hit.ImpactNormal, FVector::UpVector)
		&& GetUnrotatedClimbVelocity().Z < -10.f;
		if (bFloorReached) { return true; }
	}
	return false;
}

FQuat USRS_MovementComponent::GetClimbRotation(float DeltaTime)
{
	const FQuat CurrentRotation = UpdatedComponent->GetComponentQuat();
	if (HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity())
	{
		return CurrentRotation;
	}
	const FQuat TargetRotation = FRotationMatrix::MakeFromX(-CurrentClimbableSurfaceNormal).ToQuat();
	return FMath::QInterpTo(CurrentRotation, TargetRotation, DeltaTime, 5.f);
}

void USRS_MovementComponent::SnapToClimbableSurface(float DeltaTime)
{
	const FVector CompomentForward = UpdatedComponent->GetForwardVector();
	const FVector CompomentLocation = UpdatedComponent->GetComponentLocation();
	const FVector ProjectedVector = (CurrentClimbableSurfaceLocation - CompomentLocation).ProjectOnTo(CompomentForward);
	const FVector SnapLocation = -CurrentClimbableSurfaceNormal * ProjectedVector.Length();

	UpdatedComponent->MoveComponent
	(
		SnapLocation * DeltaTime * MaxClimbSpeed,
		UpdatedComponent->GetComponentQuat(),
		true
	);
}

bool USRS_MovementComponent::HasReachLedge()
{
	FHitResult LedgeHit = TraceFromEyeHeight(100.f, 50.f);
	if (!LedgeHit.bBlockingHit)
	{
		const FVector WalkableSurface = LedgeHit.TraceEnd;
		const FVector DownVector = -UpdatedComponent->GetUpVector();
		const FVector WalkableSurfaceEnd = WalkableSurface + DownVector * 100.f;
		FHitResult WalkableSurfaceHit = DoLineTraceSingleByObject(WalkableSurface, WalkableSurfaceEnd, true);
		if (WalkableSurfaceHit.bBlockingHit && GetUnrotatedClimbVelocity().Z > 10.f)
		{
			return true;
		}
	}
	return false;
}

void USRS_MovementComponent::PlayClimbMontage(UAnimMontage* MontageToPlay)
{
	if (!MontageToPlay) { return; }
	if (!OwningPlayerAnimInstance) { return; }
	if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) { return; }
	OwningPlayerAnimInstance->Montage_Play(MontageToPlay);
}

void USRS_MovementComponent::OnClimbMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == IdleToClimb || Montage == ClimbDownLedge)
	{
		StartClimbing();
		StopMovementImmediately();
	}
	if (Montage == ClimbUpLedge)
	{
		SetMovementMode(MOVE_Walking);
	}
}
