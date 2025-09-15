// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/SRS_AnimInstance.h"

#include "ClimbingSystem/ClimbingSystemCharacter.h"
#include "ClimbingSystem/Public/SRS_MovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void USRS_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ClimbingSystemCharacter = Cast<AClimbingSystemCharacter>(TryGetPawnOwner());
	if (ClimbingSystemCharacter)
	{
		CustomMovementComponent = ClimbingSystemCharacter->GetCustomMovementComponent();
	}
}

void USRS_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CustomMovementComponent || !ClimbingSystemCharacter) { return; }
	GetGroundSpeed();
	GetAirSpeed();
	GetShouldMove();
	GetIsFalling();
	GetIsClimbing();
	GetClimbVelocity();
}

void USRS_AnimInstance::GetGroundSpeed()
{
	GroundSpeed = UKismetMathLibrary::VSizeXY(ClimbingSystemCharacter->GetVelocity());
}

void USRS_AnimInstance::GetAirSpeed()
{
	AirSpeed = ClimbingSystemCharacter->GetVelocity().Z;
}

void USRS_AnimInstance::GetShouldMove()
{
	bShouldMove =
		CustomMovementComponent->GetCurrentAcceleration().Size() > 0.f &&
		GroundSpeed > 5.f && !bIsFalling;
}

void USRS_AnimInstance::GetIsFalling()
{
	bIsFalling = CustomMovementComponent->IsFalling();
}

void USRS_AnimInstance::GetIsClimbing()
{
	bIsClimbing = CustomMovementComponent->IsClimbing();
}

void USRS_AnimInstance::GetClimbVelocity()
{
	ClimbVelocity = CustomMovementComponent->GetUnrotatedClimbVelocity();
}
