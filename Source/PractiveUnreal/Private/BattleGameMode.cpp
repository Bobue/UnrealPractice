// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleGameMode.h"

#include "BattleMonster.h"
#include "EngineUtils.h"//애는 뭐지
#include "Blueprint/UserWidget.h"//위젯추가
#include "TimerManager.h"//타이머 추가


void ABattleGameMode::BeginPlay()
{
	Super::BeginPlay();

	//게임모드가 비긴플레이되면 어떤 몬스터인지 판단한 후 변수에 저장
	for (TActorIterator<ABattleMonster> It(GetWorld()); It; ++It)
	{
		ABattleMonster* Monster = *It;

		if (Monster->GetMonsterSide() == EMonsterSide::Player)
		{
			PlayerMonster = Monster;
		}
		else if (Monster->GetMonsterSide() == EMonsterSide::Enemy)
		{
			EnemyMonster = Monster;
		}
	}

	//전투상태 초기화
	if (!IsValid(PlayerMonster) || !IsValid(EnemyMonster))//적아군 둘중 하나라도 없으면 리턴
	{
		UE_LOG(LogTemp, Error, TEXT("아군 또는 적군 둘 중 하나는 없네"));
		return;
	}

	BattleState = EBattleState::PlayerTurn;//한번더 확실하게 플레이어턴으로 선언
	UE_LOG(LogTemp, Error, TEXT("배틀 시작 : 플레이어 턴"));

	if (IsValid(BattleWidgetClass))//위젯이 정상적으로 존재한다면
	{
		BattleWidget = CreateWidget<UUserWidget>(GetWorld(), BattleWidgetClass);//위젯객체를 생성하여 넣어준다

		if (IsValid(BattleWidget))//위젯객체가 생성됐다면 뷰포트에 보이게한다.
		{
			BattleWidget->AddToViewport();
		}
	}
}

void ABattleGameMode::PlayerAttack()
{
	if (BattleState != EBattleState::PlayerTurn)//플레이어턴이 아니면 리턴
	{
		return;
	}
	if (!IsValid(PlayerMonster) || !IsValid(EnemyMonster))//둘중하나만 없어도 리턴
	{
		return;
	}

	BattleState = EBattleState::PlayerAttacking;//위에거 확인됐으면 상태변환

	const float AnimationDuration = PlayerMonster->PlayAttackAnimation();//애님시간 받아서

	if (AnimationDuration <= 0.0f)
	{
		CompletePlayerAttack();
		return;
	}


	//EnemyAttack();//적군 자동 공격 ->타이머로 대체
	GetWorldTimerManager().SetTimer(
		PlayerAttackTimerHandle,
		this,
		&ABattleGameMode::CompletePlayerAttack,
		AnimationDuration,
		false
	);//파라미터가 엄청 많은데 무슨 함수인지 알아봐야겠다.
}

void ABattleGameMode::EnemyAttack()
{
	if (BattleState != EBattleState::EnemyTurn)//적턴이 아니면 리턴
	{
		return;
	}
	if (!IsValid(PlayerMonster) || !IsValid(EnemyMonster))//둘 중 하나만 없어도 리턴
	{
		return;
	}

	BattleState = EBattleState::EnemyAttacking;//상태바꿔주고

	EnemyMonster->Attack(PlayerMonster);//적의 공격
	UE_LOG(LogTemp, Log, TEXT("적 공격, 아군 HP: %d / %d"), PlayerMonster->GetCurrentHealth(), PlayerMonster->GetMaxHealth());

	if (PlayerMonster->IsDead())//플레이어가 죽었으면 패배상태로 변경
	{
		BattleState = EBattleState::Defeat;
		UE_LOG(LogTemp, Log, TEXT("패배..."));
		return;
	}

	BattleState = EBattleState::PlayerTurn;//안 죽었으면 다시 플레이어 턴
	UE_LOG(LogTemp, Log, TEXT("플레이어 턴"));

}

ABattleMonster* ABattleGameMode::GetPlayerMonster() const
{
	return PlayerMonster;
}
ABattleMonster* ABattleGameMode::GetEnemyMonster() const
{
	return EnemyMonster;
}
void ABattleGameMode::CompletePlayerAttack()
{
	PlayerMonster->PlayIdleAnimation();
	PlayerMonster->Attack(EnemyMonster);//공격

	UE_LOG(LogTemp, Log, TEXT("플레이어 공격. 적 HP : %d / %d"), EnemyMonster->GetCurrentHealth(), EnemyMonster->GetMaxHealth());

	if (EnemyMonster->IsDead())//방금 공격으로 적 죽었는지 확인
	{
		BattleState = EBattleState::Victory;
		UE_LOG(LogTemp, Log, TEXT("플레이어 승리!"));
		return;
	}

	//죽은게 아니라면 적군의 턴으로 변환
	BattleState = EBattleState::EnemyTurn;
	UE_LOG(LogTemp, Log, TEXT("적군 턴"));

	GetWorldTimerManager().SetTimer(
		EnemyAttackTimerHandle,
		this,
		&ABattleGameMode::EnemyAttack,
		1.0f,
		false
	);
}