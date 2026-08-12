// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanePawn.h"

//어디에 있는 헤더파일인지 모르겠네 어쨌든 가져와서 인클루드 시켜줌
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"

//입력 시스템에 따른 헤더추가
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"

//애니메이션 및 스켈레톤 메쉬에 대한 헤더추가
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSingleNodeInstance.h"

// Sets default values
APlanePawn::APlanePawn()//생성자
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//헤더에서 생성했던 씬 루트에 신루트 컴포넌트 생성시키고 그대로 루트 컴포넌트로 설정
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	//마찬가지로 헤더에서 생성했던 비행기 메쉬에 스태틱 메쉬 컴포넌트 생성시키고 씬 루트에 붙임
	PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh"));
	PlaneMesh->SetupAttachment(SceneRoot);
	//프로펠러는 비행기 메쉬에 붙히기
	PropellerMesh_1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PropellerMesh_1"));
	PropellerMesh_1->SetupAttachment(PlaneMesh);
	PropellerMesh_2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PropellerMesh_2"));
	PropellerMesh_2->SetupAttachment(PlaneMesh);

	//마찬가지로 헤더에서 생성했던 스프링암에 스프링암 컴포넌트 생성시키고 씬 루트에 붙임
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(SceneRoot);

	//아래는 스프링암의 갖가지 설정들임. 아직 이해가 안되는 부분도 있고 에디터가 아니라 왜 여기서 하는지는 잘 모르겠음.
	//지금 생각으로는 에디터가 시각적으로 확인하는것도 그렇고 훨씬 편할것같은데 굳이 코드로 하는건 왜그런지 잘 모르겠음.
	SpringArm->TargetArmLength = 600.0f;
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 3.0f;

	//마찬가지로 헤더에서 생성했던 카메라에 카메라 컴포넌트 생성
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	//다만 다른 컴포넌트와는 다르게 루트가 아니라 스프링암에 붙임. SocketName은 뭔지 모르겠음.
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);


	//1. 스켈레톤 메쉬 컴포넌트 생성
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(SceneRoot);
	SkeletalMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	//게임 시작 시에는 기존 스테틱 메쉬만 표시
	SkeletalMesh->SetVisibility(false);
	SkeletalMesh->SetHiddenInGame(true);
	//충돌은 기존 플랜메쉬가 담당하기
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//2. 워킹 메쉬 컴포넌트 생성
	WalkingMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WalkingMesh"));
	WalkingMesh->SetupAttachment(SceneRoot);
	WalkingMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	//게임 시작 시에는 기존 스테틱 메쉬만 표시
	WalkingMesh->SetVisibility(false);
	WalkingMesh->SetHiddenInGame(true);
	//충돌은 기존 플랜메쉬가 담당하기
	WalkingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void APlanePawn::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("Plane BeginPlay"));//로그확인용
	
	//비행기 초기속도 설정
	CurrentSpeed = NormalSpeed;
	TargetSpeed = NormalSpeed;

	//이 밑으로 컨트롤러를 가져와서 입력 매핑 컨텍스트를 추가하는 코드임
	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController && PlaneMappingContext)
	{
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

		if ((InputSubsystem))
		{
			InputSubsystem->AddMappingContext(PlaneMappingContext, 0);
			UE_LOG(LogTemp, Warning, TEXT("Mapping Context Added"));
		}
	}
}

// Called every frame
void APlanePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//보간을 사용한 가속과 감속
	CurrentSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaTime, SpeedInterpRate);

	const FRotator RotationDelta(
		PitchInput * PitchSpeed * DeltaTime,
		YawInput * YawSpeed * DeltaTime,
		0.0f
	);
	AddActorLocalRotation(RotationDelta);

	const float TargetBankAngle = -YawInput * MaxBankAngle;
	CurrentBankAngle = FMath::FInterpTo(CurrentBankAngle, TargetBankAngle, DeltaTime, BankInterpRate);

	//메시가 나중에 또 추가될것같아서 하나로 합쳐주기
	const FRotator BankRotation(0.0f, 0.0f, CurrentBankAngle);
	PlaneMesh->SetRelativeRotation(BankRotation);
	SkeletalMesh->SetRelativeRotation(BankRotation);
	WalkingMesh->SetRelativeRotation(BankRotation);

	//if (bIsSkeletalMode)
	//{
	//	FlyingKeyMaping();
	//}
	//else
	//{
	//	DefaultKeyMaping(DeltaTime);
	//}

	//항상 앞으로 움직이는 방향
	const FVector Movement = GetActorForwardVector() * CurrentSpeed * DeltaTime;

	FHitResult HitResult;

	AddActorWorldOffset(Movement, true, &HitResult);

	//프로펠러 회전
	const float PropellerRotationSpeed = 1500.0f;
	const FRotator PropellerRotation(
		0.0f,
		0.0f,
		PropellerRotationSpeed * DeltaTime
	);
	PropellerMesh_1->AddLocalRotation(PropellerRotation);
	PropellerMesh_2->AddLocalRotation(PropellerRotation);
}

// Called to bind functionality to input
void APlanePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (!EnhancedInputComponent)
	{
		return;
	}
	//여기서 선언된 BindAction은 앞에 있던 액션이 감지되면 뒤에 있는 함수를 호출하는 역할임.
	EnhancedInputComponent->BindAction(
		PitchAction,
		ETriggerEvent::Triggered,
		this,
		&APlanePawn::HandlePitch);

	EnhancedInputComponent->BindAction(
		BoostAction,
		ETriggerEvent::Triggered,
		this,
		&APlanePawn::HandleBoostStarted);

	EnhancedInputComponent->BindAction(
		YawAction,
		ETriggerEvent::Triggered,
		this,
		&APlanePawn::HandleYaw);

	//손 땠을 때 입력값 초기화
	EnhancedInputComponent->BindAction(
		BoostAction,
		ETriggerEvent::Completed,
		this,
		&APlanePawn::HandleBoostCompleted);

	EnhancedInputComponent->BindAction(
		PitchAction,
		ETriggerEvent::Completed,
		this,
		&APlanePawn::HandlePitch);

	EnhancedInputComponent->BindAction(
		YawAction,
		ETriggerEvent::Completed,
		this,
		&APlanePawn::HandleYaw);
	//폼 체인지 액션 바인딩
	EnhancedInputComponent->BindAction(
		TransformAction,
		ETriggerEvent::Started,
		this,
		&APlanePawn::HandleTransformStarted);
}

//입력된 값에 맞게 값을 처리하는 함수들임.
void APlanePawn::HandlePitch(const FInputActionValue& Value)
{
	PitchInput = Value.Get<float>();
}

void APlanePawn::HandleYaw(const FInputActionValue& Value)
{
	YawInput = Value.Get<float>();
}

void APlanePawn::HandleBoostStarted(const FInputActionValue& Value)
{
	TargetSpeed = BoostSpeed;
}
void APlanePawn::HandleBoostCompleted(const FInputActionValue& Value)
{
	TargetSpeed = NormalSpeed;
}

void APlanePawn::HandleTransformStarted(const FInputActionValue& Value)
{
	if (!PlaneMesh || !SkeletalMesh || !WalkingMesh)//메시 둘 중 하나만이라도 없으면 끝
	{
		UE_LOG(LogTemp, Warning, TEXT("노 메시"));
		return;
	}

	//상태변화
	int32 NextState = static_cast<int32>(planeForm) + 1;
	if (NextState >= static_cast<int32>(PlaneForm::MAX))
	{
		NextState = 0;
	}
	planeForm = static_cast<PlaneForm>(NextState);

	switch (planeForm)
	{
	case PlaneForm::DefaultPlane:
		//SkeletalMesh->Stop();
		//if (UAnimSingleNodeInstance* AnimInstance = WalkingMesh->GetSingleNodeInstance())
		//{
		//	AnimInstance->SetPlaying(false);
		//	AnimInstance->SetPosition(0.0f, false);
		//}

		WalkingMesh->SetVisibility(false);
		WalkingMesh->SetHiddenInGame(true);

		PlaneMesh->SetVisibility(true, false);
		PlaneMesh->SetHiddenInGame(false, false);

		return;
	case PlaneForm::FlyingPlane:
		//아까 스켈레톤과는 다르게 후자의 파라미터의 경우 자식의 처리까지 지정해주는 것이다.
		PlaneMesh->SetVisibility(false, false);
		PlaneMesh->SetHiddenInGame(true, false);

		SkeletalMesh->SetVisibility(true);
		SkeletalMesh->SetHiddenInGame(false);
		if (PlaneFlapAnimation)
		{
			//그냥 플레이만하면 다시 재생할때 멈추는 문제가 생겨서 아예 초기화하고 다시 실행하는 코드작성
			SkeletalMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			SkeletalMesh->SetAnimation(PlaneFlapAnimation);

			if (UAnimSingleNodeInstance* AnimInstance = SkeletalMesh->GetSingleNodeInstance())
			{
				AnimInstance->SetLooping(true);
				AnimInstance->SetPosition(0.0f, false);
				AnimInstance->SetPlaying(true);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("엥 애니매이션 읎다"));
		}
		return;
	case PlaneForm::WalkingPlane:
		//아까 스켈레톤과는 다르게 후자의 파라미터의 경우 자식의 처리까지 지정해주는 것이다.
		SkeletalMesh->SetVisibility(false, false);
		SkeletalMesh->SetHiddenInGame(true, false);

		WalkingMesh->SetVisibility(true);
		WalkingMesh->SetHiddenInGame(false);
		if (PlaneWalkingAnimation)
		{
			//그냥 플레이만하면 다시 재생할때 멈추는 문제가 생겨서 아예 초기화하고 다시 실행하는 코드작성
			WalkingMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			WalkingMesh->SetAnimation(PlaneWalkingAnimation);

			if (UAnimSingleNodeInstance* AnimInstance = WalkingMesh->GetSingleNodeInstance())
			{
				AnimInstance->SetLooping(true);
				AnimInstance->SetPosition(0.0f, false);
				AnimInstance->SetPlaying(true);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("엥 애니매이션 읎다"));
		}
		return;
		return;
	}

}

void APlanePawn::DefaultKeyMaping(float DeltaTime)
{

}
void APlanePawn::FlyingKeyMaping()
{

}