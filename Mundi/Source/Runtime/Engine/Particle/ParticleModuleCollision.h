#pragma once
#include "ParticleModule.h"

#include "UparticlemoduleCollision.generated.h"
#include "Source/Runtime/Engine/Distribution/Distributions.h"

struct FHitResult;
UCLASS()
class UParticleModuleCollision : public UParticleModule
{
	GENERATED_REFLECTION_BODY()
public:
	/** 충돌 후 속도가 감소하는 비율 (1.0 = 탄성 충돌, 0.0 = 완전 비탄성) */
	UPROPERTY(EditAnywhere, Category = "Collision")
	FRawDistributionVector DampingFactor;

	/** 최대 충돌 횟수 */
	UPROPERTY(EditAnywhere, Category = "Collision")
	FRawDistributionFloat MaxCollisions;

	/** 충돌 검사를 시작하기 전 대기 시간 */
	UPROPERTY(EditAnywhere, Category = "Collision")
	FRawDistributionFloat DelayAmount;

	/** 충돌 시 파티클 크기(반지름) 보정값 (값이 클수록 더 멀리서 충돌) */
	UPROPERTY(EditAnywhere, Category = "Collision")
	float DirScalar;

	/** 최대 충돌 횟수 도달 시 행동 (true: 삭제, false: 정지) */
	UPROPERTY(EditAnywhere, Category = "Collision")
	bool bKillOnMaxCollision;

public:
	UParticleModuleCollision();

	virtual ~UParticleModuleCollision() = default;

	void InitializeDefaults();

	//~Begin UParticleModule Interface
	virtual void Spawn(const FSpawnContext& Context) override;
	virtual void Update(const FUpdateContext& Context) override;
	virtual uint32 RequiredBytes(UParticleModuleTypeDataBase* TypeData) override;
	//~End UParticleModule Interface

protected:
	/** 실제 충돌 체크를 수행하는 헬퍼 함수 */
	bool PerformCollisionCheck(FParticleEmitterInstance* Owner, FBaseParticle* InParticle,
		FHitResult& Hit, AActor* SourceActor, const FVector& End, const FVector& Start, const FVector& Extent);
};
