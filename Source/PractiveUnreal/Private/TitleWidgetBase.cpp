// Fill out your copyright notice in the Description page of Project Settings.


#include "TitleWidgetBase.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"

#include "Kismet/GameplayStatics.h"

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
}

void UTitleWidgetBase::StartServer()
{
	// 서버 시작 로직 구현
	UE_LOG(LogTemp, Log, TEXT("Start Server button clicked"));

	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Lobby"), true, TEXT("Listen"));
}
void UTitleWidgetBase::ConnectServer()
{
	// 서버 연결 로직 구현
	UE_LOG(LogTemp, Log, TEXT("Connect Server button clicked"));
}