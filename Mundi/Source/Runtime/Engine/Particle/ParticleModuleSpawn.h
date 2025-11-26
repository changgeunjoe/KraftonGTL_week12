#pragma once
#include "ParticleModuleSpawnBase.h"
#include "UParticleModuleSpawn.generated.h"

/**
 * 파티클 스폰 속도를 제어하는 모듈.
 * 초당 생성되는 파티클 수를 정의한다.
 */
UCLASS(DisplayName="Spawn", Description="파티클 스폰 속도 설정")
class UParticleModuleSpawn : public UParticleModuleSpawnBase
{
	GENERATED_REFLECTION_BODY()

public:
	/** 초당 스폰되는 파티클 수 */
	UPROPERTY(EditAnywhere, Category="Rate")
	float Rate;

	/** 스폰 속도에 적용되는 배율 */
	UPROPERTY(EditAnywhere, Category="Rate")
	float RateScale;

public:
	UParticleModuleSpawn();

	virtual ~UParticleModuleSpawn() = default;

	//~Begin UParticleModuleSpawnBase Interface
	virtual bool GetSpawnAmount(const FContext& Context, int32 Offset, float OldLeftover,
		float DeltaTime, int32& Number, float& Rate) override;
	virtual float GetMaximumSpawnRate() override;
	virtual float GetEstimatedSpawnRate() override;
	//~End UParticleModuleSpawnBase Interface
};
