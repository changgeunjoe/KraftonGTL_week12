#pragma once
#include "ParticleModule.h"

/**
 * @note UParticleModuleVelocityBase를 편의를 위해 생략한다.
 */
UCLASS()
class UParticleModuleVelocity : public UParticleModule
{
	DECLARE_CLASS(UParticleModuleVelocity, UParticleModule)
public:
	//~ Begin UParticleModuleVelocityBase

	/** True일 경우, 속도를 월드 공간에서 정의된 것으로 간주한다. */
	uint32 bInWorldSpace;

	/** True일 경우, 파티클 시스템 컴포넌트의 스케일을 속도 값에 적용한다. */
	uint32 bApplyOwnerScale;

	//~ End UParticleModuleVelocityBase

	/**
	 * 파티클이 스폰될 때 적용할 기본 속도
	 * @note 언리얼엔진에서는 DistributionVector를 사용
	 */
	FVector StartVelocity;

	/**
	 * 이미터 원점으로부터 파티클 위치 방향으로 적용할 방사형 속도
	 */
	float StartVelocityRadial;

public:
	UParticleModuleVelocity();
	virtual ~UParticleModuleVelocity() = default;

	//~ Begin UParticleModule Interface
	virtual void Spawn(const FSpawnContext& Context) override;
	//~ End UParticleModule Interface
};
