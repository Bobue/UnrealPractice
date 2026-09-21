// Fill out your copyright notice in the Description page of Project Settings.


#include "TitlePC.h"

#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "ServerDirectorySubsystem.h"
#include "TitleWidgetBase.h"

void ATitlePC::BeginPlay()
{
	Super::BeginPlay();
	// 위젯 생성 및 화면에 표시

	UClass* WidgetClass = TitleWidgetClass.LoadSynchronous();
	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load title widget class: %s"), *TitleWidgetClass.ToString());
		return;
	}

	TitleWidgetInstance = CreateWidget<UTitleWidgetBase>(this, WidgetClass);
	if (!TitleWidgetInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create TitleWidgetInstance"));
		return;
	}

	TitleWidgetInstance->AddToViewport();

	// 타이틀 화면은 버튼 입력만 받으므로 마우스 커서와 UI 입력 모드를 켭니다.
	bShowMouseCursor = true;
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	UE_LOG(LogTemp, Warning, TEXT("TitlePC BeginPlay called"));
}

void ATitlePC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 서버 접속이나 맵 이동으로 타이틀을 떠날 때 위젯이 화면에 남지 않도록 정리합니다.
	if (TitleWidgetInstance)
	{
		TitleWidgetInstance->RemoveFromParent();
		TitleWidgetInstance = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void ATitlePC::TitleStartServer()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UServerDirectorySubsystem* ServerDirectory = GameInstance->GetSubsystem<UServerDirectorySubsystem>())
		{
			ServerDirectory->StartListenServer();
			return;
		}
	}
	UE_LOG(LogTemp, Error, TEXT("TitleStartServer: server directory subsystem is unavailable"));
}

void ATitlePC::TitleLogin(const FString& UserId, const FString& Password)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UServerDirectorySubsystem* ServerDirectory = GameInstance->GetSubsystem<UServerDirectorySubsystem>())
		{
			ServerDirectory->LoginAndConnect(UserId, Password);
			return;
		}
	}
	UE_LOG(LogTemp, Error, TEXT("TitleLogin: server directory subsystem is unavailable"));
}
