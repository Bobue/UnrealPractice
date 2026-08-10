// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlanePawn.generated.h"

//추가 비행기에 붙힐 컴포넌트들
class USceneComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;

//스켈레톤 메시와 애니메이션 적용 위한 전방선언
class USkeletalMeshComponent;
class UAnimSequence;

//입력 에셋 선언
class UInputMappingContext;
class UInputAction;

//입력 밸류 전방선언
struct FInputActionValue;

UCLASS()
class PRACTIVEUNREAL_API APlanePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlanePawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


protected://컴포넌트 추가
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;//씬 루트

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PlaneMesh;//비행기 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;//스켈레톤 메쉬 -> 날개짓 하려고 본 깔아둔 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* PropellerMesh_1;//프로펠러 메쉬1
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* PropellerMesh_2;//프로펠러 메쉬2

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArm;//스프링 암

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;//카메라


protected://인풋시스템 추가
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> PlaneMappingContext;//입력 매핑 컨텍스트

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> PitchAction;//피치 액션

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> YawAction;//요 액션

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> BoostAction;//부스트 액션

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> TransformAction;//폼 변환을 위한 액션
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimSequence> PlaneFlapAnimation;//비행기 날개짓 애니메이션


protected:
	//인풋 핸들러
	void HandlePitch		 (const FInputActionValue& Value);
	void HandleYaw			 (const FInputActionValue& Value);
	void HandleBoostStarted  (const FInputActionValue& Value);
	void HandleBoostCompleted(const FInputActionValue& Value);
	void HandleTransformStarted(const FInputActionValue& Value);//폼 변환 시작키

	//현재 인풋 값
	float PitchInput    = 0.0f;
	float YawInput		= 0.0f;

	//비행 설정
	UPROPERTY(EditAnywhere, Category = "Flight|Speed")
	float NormalSpeed = 500.0f;
	UPROPERTY(EditAnywhere, Category = "Flight|Speed")
	float BoostSpeed = 1000.0f;
	UPROPERTY(EditAnywhere, Category = "Flight|Speed")
	float SpeedInterpRate = 2.0f;

	//비행 회전
	UPROPERTY(EditAnywhere, Category = "Flight|Rotation")
	float PitchSpeed = 60.0f;
	UPROPERTY(EditAnywhere, Category = "Flight|Rotation")
	float YawSpeed = 60.0f;
	UPROPERTY(EditAnywhere, Category = "Flight|Rotation")
	float MaxBankAngle = 30.0f;
	UPROPERTY(EditAnywhere, Category = "Flight|Rotation")
	float BankInterpRate = 5.0f;

private:
	float CurrentSpeed = 0.0f;
	float TargetSpeed = 0.0f;
	float CurrentBankAngle = 0.0f;
	bool bIsSkeletalMode = false;//폼변환 여부
};