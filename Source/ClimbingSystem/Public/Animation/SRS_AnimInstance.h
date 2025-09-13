// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SRS_AnimInstance.generated.h"


class USRS_MovementComponent;
class AClimbingSystemCharacter;

UCLASS()
class CLIMBINGSYSTEM_API USRS_AnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
protected:

private:
	UPROPERTY()
	AClimbingSystemCharacter* ClimbingSystemCharacter;

	UPROPERTY()
	USRS_MovementComponent* CustomMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climbing", meta = (AllowPrivateAccess = "true"))
	float GroundSpeed { 0.0f };

	void GetGroundSpeed();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climbing", meta = (AllowPrivateAccess = "true"))
	float AirSpeed { 0.0f };

	void GetAirSpeed();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climbing", meta = (AllowPrivateAccess = "true"))
	bool bShouldMove { false };

	void GetShouldMove();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climbing", meta = (AllowPrivateAccess = "true"))
	bool bIsFalling { false };

	void GetIsFalling();
};
