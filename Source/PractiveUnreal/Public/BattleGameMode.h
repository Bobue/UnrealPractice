// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BattleGameMode.generated.h"

class ABattleMonster;//몬스터를 게임모드가 알 수 있게 전방선언
class UUserWidget;//위젯 클래스 추가


UENUM(BlueprintType)
enum class EBattleState : uint8 //게임상태를 구분
{
	PlayerTurn,//내턴
	PlayerAttacking,//내가 공격중
	EnemyTurn,//상대턴
	EnemyAttacking,//상대가 공격중
	Victory,//승리
	Defeat//패배
};

UCLASS()
class PRACTIVEUNREAL_API ABattleGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void PlayerAttack();//플레이어 공격

	//체력 가져오려고 객체 자체를 가져오는 함수들
	UFUNCTION(BlueprintPure, Category = "Battle")
	ABattleMonster* GetPlayerMonster() const;
	UFUNCTION(BlueprintPure, Category = "Battle")
	ABattleMonster* GetEnemyMonster() const;



protected:
	virtual void BeginPlay() override;

	//두편의 몬스터를 각각 구분하여 선언
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Battle")
	TObjectPtr<ABattleMonster> PlayerMonster;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Battle")
	TObjectPtr<ABattleMonster> EnemyMonster;

	//전투상태 선언
	UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly, Category = "Battle")
	EBattleState BattleState = EBattleState::PlayerTurn;//시작은 플레이어턴으로

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> BattleWidgetClass;//생성할 위젯 종류
	UPROPERTY()//왜 아무것도 안 하는지 아직 잘 모르겠음.
	TObjectPtr<UUserWidget> BattleWidget;//실제로 생성된 위젯 '객체'


	void EnemyAttack();//적 자동 공격
	FTimerHandle EnemyAttackTimerHandle;//타이머 추가
	FTimerHandle PlayerAttackTimerHandle;
	void CompletePlayerAttack();//공격완료 함수 추가
};
