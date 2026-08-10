// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPC.h"

void ALobbyPC::BeginPlay()
{
	Super::BeginPlay();
	// 위젯 생성 및 화면에 표시
	UE_LOG(LogTemp, Warning, TEXT("LobbyPC BeginPlay called"));
}