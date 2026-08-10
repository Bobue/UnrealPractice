// Fill out your copyright notice in the Description page of Project Settings.


#include "TitlePC.h"
#include "TitleWidgetBase.h"
void ATitlePC::BeginPlay()
{
	Super::BeginPlay();
	// 위젯 생성 및 화면에 표시

	FSoftClassPath TitleWidgetClass(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/Title/UI/WBP_Title.WBP_Title'"));

	UClass* WidgetClass = TitleWidgetClass.TryLoadClass<UTitleWidgetBase>();
	if (WidgetClass)
	{
		TitleWidgetInstance = CreateWidget<UTitleWidgetBase>(this, WidgetClass);
		if(TitleWidgetInstance)
		{
			TitleWidgetInstance->AddToViewport();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create TitleWidgetInstance"));
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("TitlePC BeginPlay called"));
}