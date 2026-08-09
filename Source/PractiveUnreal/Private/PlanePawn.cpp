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
}

// Called when the game starts or when spawned
void APlanePawn::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("Plane BeginPlay"));//로그확인용
	
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
		ThrottleAction,
		ETriggerEvent::Triggered,
		this,
		&APlanePawn::HandleThrottle);

	EnhancedInputComponent->BindAction(
		PitchAction,
		ETriggerEvent::Triggered,
		this,
		&APlanePawn::HandlePitch);

	EnhancedInputComponent->BindAction(
		RollAction,
		ETriggerEvent::Triggered,
		this,
		&APlanePawn::HandleRoll);

	EnhancedInputComponent->BindAction(
		YawAction,
		ETriggerEvent::Triggered,
		this,
		&APlanePawn::HandleYaw);
}

//입력된 값에 맞게 값을 처리하는 함수들임. 지금은 그냥 로그로 찍는것만 함.
void APlanePawn::HandleThrottle(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Throttle: %f"), Value.Get<float>());
}

void APlanePawn::HandlePitch(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Pitch: %f"), Value.Get<float>());
}

void APlanePawn::HandleRoll(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Roll: %f"), Value.Get<float>());
}

void APlanePawn::HandleYaw(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Yaw: %f"), Value.Get<float>());
}