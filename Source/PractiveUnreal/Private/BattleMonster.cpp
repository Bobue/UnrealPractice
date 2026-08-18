// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleMonster.h"

#include "Components/SkeletalMeshComponent.h" //마찬가지로 스켈레탈메시 인클루드
#include "Animation/AnimSequence.h"//애니메이션 시퀀스 추가

// Sets default values
ABattleMonster::ABattleMonster()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bCanEverTick = false;//매프레임 호출하는 tick을 할지말지 결정하는 부분.
	//기본적으로는 true로 되어있지만 턴제게임 특성상 매프레임 계산하는 것이 아니기에 false로 설정해두었음
	
	
	MonsterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MonsterMesh"));

	SetRootComponent(MonsterMesh);//스켈레탈메시를 루트컴포넌트로 지정

}

// Called when the game starts or when spawned
void ABattleMonster::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = MaxHealth; //생성될때 최대체력으로 설정

	PlayIdleAnimation();//대기모션 재생
}

// Called every frame
void ABattleMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABattleMonster::ReceiveDamage(int32 DamageAmount)
{
	if (DamageAmount <= 0)//데미지가 없으면
	{
		return;
	}

	//데미지 계산식
	//Clamp는 (계산, 최솟값, 최댓값)을 정해서 그 범위안에서만 계산되길 제어함
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0, MaxHealth);
}

void ABattleMonster::Attack(ABattleMonster* Target)
{
	if (!IsValid(Target))//유효한 개체인지 확인하는 함수
	{
		return;
	}

	Target->ReceiveDamage(AttackPower);//내 공격력으로 타겟의 피해함수를 호출한다.
}

EMonsterSide ABattleMonster::GetMonsterSide() const
{
	return MonsterSide;//어느편인지 반환
}

bool ABattleMonster::IsDead() const
{
	return CurrentHealth <= 0;//현재 체력이 0보다 적다면 true 반환
}

int32 ABattleMonster::GetCurrentHealth() const
{
	return CurrentHealth;
}

int32 ABattleMonster::GetMaxHealth() const
{
	return MaxHealth;
}

float ABattleMonster::GetHealthPercent() const
{
	if (MaxHealth <= 0)//최대체력이 0보다 작으면 그냥 0 리턴
	{
		UE_LOG(LogTemp, Error, TEXT("0보다 작다!"));
		return 0.0f;
	}

	//바로 계산해서 출력(정수끼리 나누면 소수가 사라질수도 있어서 float로 변환후 계산
	return static_cast<float>(CurrentHealth) / static_cast<float>(MaxHealth);
}

float ABattleMonster::PlayAttackAnimation()
{
	if (!IsValid(MonsterMesh) || !IsValid(AttackAnimation))//메쉬나 애님이 없다면 리턴
	{
		UE_LOG(LogTemp, Error, TEXT("메시나 애님이 읎다야"));
		return 0.0f;
	}
	//확인용 로그
	UE_LOG(
		LogTemp,
		Error,
		TEXT("Playing animation: %s, Length: %f"),
		*AttackAnimation->GetName(),
		AttackAnimation->GetPlayLength()
	);


	MonsterMesh->PlayAnimation(AttackAnimation, false);
	return AttackAnimation->GetPlayLength();
}

void ABattleMonster::PlayIdleAnimation()
{
	if (!IsValid(MonsterMesh) || !IsValid(IdleAnimation))//메쉬나 애님이 없다면 리턴
	{
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("대기모션 재생"));
	MonsterMesh->PlayAnimation(IdleAnimation, true);
}