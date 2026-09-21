// Fill out your copyright notice in the Description page of Project Settings.


#include "TitleWidgetBase.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "ServerDirectorySubsystem.h"

void UTitleWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartServerButton)
	{
		StartServerButton->OnClicked.AddDynamic(this, &UTitleWidgetBase::StartServer);
	}
	if (ConnectServerButton)
	{
		ConnectServerButton->OnClicked.AddDynamic(this, &UTitleWidgetBase::ConnectServer);
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UServerDirectorySubsystem* ServerDirectory = GameInstance->GetSubsystem<UServerDirectorySubsystem>())
		{
			ServerDirectory->OnStatusChanged.AddDynamic(this, &UTitleWidgetBase::HandleServerDirectoryStatus);
		}
	}
}

void UTitleWidgetBase::NativeDestruct()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UServerDirectorySubsystem* ServerDirectory = GameInstance->GetSubsystem<UServerDirectorySubsystem>())
		{
			ServerDirectory->OnStatusChanged.RemoveDynamic(this, &UTitleWidgetBase::HandleServerDirectoryStatus);
		}
	}
	Super::NativeDestruct();
}

void UTitleWidgetBase::StartServer()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UServerDirectorySubsystem* ServerDirectory = GameInstance->GetSubsystem<UServerDirectorySubsystem>())
		{
			ServerDirectory->StartListenServer();
			return;
		}
	}
	HandleServerDirectoryStatus(false, TEXT("서버 디렉터리 기능을 초기화하지 못했습니다."));
}

void UTitleWidgetBase::ConnectServer()
{
	if (!UserID || !Password)
	{
		HandleServerDirectoryStatus(false, TEXT("로그인 입력 위젯을 찾지 못했습니다."));
		return;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UServerDirectorySubsystem* ServerDirectory = GameInstance->GetSubsystem<UServerDirectorySubsystem>())
		{
			ServerDirectory->LoginAndConnect(UserID->GetText().ToString(), Password->GetText().ToString());
			return;
		}
	}
	HandleServerDirectoryStatus(false, TEXT("서버 디렉터리 기능을 초기화하지 못했습니다."));
}

void UTitleWidgetBase::HandleServerDirectoryStatus(const bool bSuccess, const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("Server directory status (%s): %s"), bSuccess ? TEXT("success") : TEXT("failure"), *Message);
	if (StatusText)
	{
		StatusText->SetText(FText::FromString(Message));
	}

	if (bSuccess && ServerIP)
	{
		if (const UGameInstance* GameInstance = GetGameInstance())
		{
			if (const UServerDirectorySubsystem* ServerDirectory = GameInstance->GetSubsystem<UServerDirectorySubsystem>())
			{
				if (!ServerDirectory->GetLastServerAddress().IsEmpty())
				{
					ServerIP->SetText(FText::FromString(ServerDirectory->GetLastServerAddress()));
				}
			}
		}
	}
}
