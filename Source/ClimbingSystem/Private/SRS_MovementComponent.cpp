// Fill out your copyright notice in the Description page of Project Settings.


#include "ClimbingSystem/Public/SRS_MovementComponent.h"

#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ClimbingSystem/Debugger/DebugHelper.h"
#include "Components/CapsuleComponent.h"

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
		StopMovementKeepPathing();
	}
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
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
		true,
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
		End,
		true,
		true
	);
}

void USRS_MovementComponent::ToggleClimbing(bool bEnableClimbing)
{
	if (bEnableClimbing)
	{
		if (CanClimb())
		{
			Debug::Print(TEXT("Can Climb!"));
			StartClimbing();
		}
		else
		{
			Debug::Print(TEXT("Can NOT Climb!"));
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

void USRS_MovementComponent::StopClimbing()
{
	SetMovementMode(MOVE_Falling);
}
