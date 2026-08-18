// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleMonster.generated.h"

class USkeletalMeshComponent; //스켈레탈 메시 선언
class UAnimSequence;//애니메이션시퀀스 선언

UENUM(BlueprintType)
enum class EMonsterSide : uint8 //배틀몬스터의 종류
{
	Player,
	Enemy
};

UCLASS()
class PRACTIVEUNREAL_API ABattleMonster : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABattleMonster();

public:
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void ReceiveDamage(int32 DamageAmout);//공격받는 함수
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void Attack(ABattleMonster* Target);//공격하는 함수
	UFUNCTION(BlueprintPure, Category = "Battle")
	EMonsterSide GetMonsterSide() const;//어느편인지 출력하는 함수
	UFUNCTION(BlueprintPure, Category = "Battle")
	bool IsDead() const;//죽음을 확인하는 함수
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetCurrentHealth() const;//현재체력을 확인하는 함수
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetMaxHealth() const;//최대체력을 확인하는 함수

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetHealthPercent() const;//UI랑 연동하려고 체력을 퍼센테이지로 변환하는 함수

	UFUNCTION(BlueprintCallable, Category = "Animation")
	float PlayAttackAnimation();//애니메이션 재생함수(공격모션)
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayIdleAnimation();//애니메이션 재생함수(대기모션)

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> MonsterMesh;//캐릭터 메쉬
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")//블루프린트 클래스의 기본값으로 설정
	int32 MaxHealth = 100;//최대체력
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stats")//현재상태를 확인만 가능
	int32 CurrentHealth = 100;//현재체력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	int32 AttackPower = 20;//공격력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Battle")
	EMonsterSide MonsterSide = EMonsterSide::Player;//어느편인지 선언
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimSequence> AttackAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimSequence> IdleAnimation;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
