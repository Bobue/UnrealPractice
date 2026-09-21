//UI 서버 연결 작업

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TitleWidgetBase.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;

UCLASS()
class PRACTIVEUNREAL_API UTitleWidgetBase : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget", meta=(BindWidget))
	TObjectPtr<UButton> StartServerButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget", meta = (BindWidget))
	TObjectPtr<UButton> ConnectServerButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget", meta = (BindWidget))
	TObjectPtr<UEditableTextBox> UserID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget", meta = (BindWidget))
	TObjectPtr<UEditableTextBox> Password;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget", meta = (BindWidgetOptional))
	TObjectPtr<UEditableTextBox> ServerIP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StatusText;

	//델리게이트 작업
	UFUNCTION()
	void StartServer();

	UFUNCTION()
	void ConnectServer();

	UFUNCTION()
	void HandleServerDirectoryStatus(bool bSuccess, const FString& Message);
};
