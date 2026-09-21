// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TitlePC.generated.h"
class UTitleWidgetBase;
/**
 * 
 */
UCLASS()
class PRACTIVEUNREAL_API ATitlePC : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TObjectPtr<UTitleWidgetBase> TitleWidgetInstance;

	/** 타이틀 화면에 띄울 위젯 블루프린트입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSoftClassPtr<UTitleWidgetBase> TitleWidgetClass =
		TSoftClassPtr<UTitleWidgetBase>(FSoftObjectPath(TEXT("/Game/Title/UI/WBP_Title.WBP_Title_C")));

	/** 버튼 클릭 없이 서버 등록 흐름을 실행합니다. 시연과 자동화에 사용합니다. */
	UFUNCTION(Exec)
	void TitleStartServer();

	/** 버튼 클릭 없이 로그인 및 접속 흐름을 실행합니다. 시연과 자동화에 사용합니다. */
	UFUNCTION(Exec)
	void TitleLogin(const FString& UserId, const FString& Password);
};
