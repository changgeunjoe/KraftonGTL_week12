#pragma once
#include "ParticleLODLevel.h"
#include "ParticleModule.h"

UCLASS()
class UParticleModuleSpawnBase : public UParticleModule
{
	DECLARE_CLASS(UParticleModuleSpawnBase, UParticleModule)

public:
	/** * true이면, 이미터(emitter)의 SpawnModule에 있는 SpawnRate가 처리된다.
	 * 이미터에 여러 개의 Spawn 모듈이 '쌓여(stacked)' 있는 경우,
	 * 그중 하나라도 이 값이 false로 설정되어 있으면 SpawnModule의 SpawnRate는 처리되지 않는다.
	 */
	uint32 bProcessSpawnRate:1;

	/**
	 * 이 모듈이 기여하는 스폰(spawn) 수량을 가져온다.
	 * 여러 스폰 전용 모듈이 존재하는 경우, 그중 하나라도 SpawnRate 처리를
	 * 무시하면 전체가 무시된다는 점에 유의해야 한다.
	 *
	 * @param Owner			스폰을 수행 중인 파티클 이미터 인스턴스.
	 * @param Offset		모듈을 위한 파티클 페이로드(payload) 내 오프셋.
	 * @param OldLeftover	이전 프레임에서 남은 타임슬라이스(timeslice) 조각.
	 * @param DeltaTime		지난 프레임 이후 경과된 시간.
	 * @param Number		스폰할 파티클 수. (출력)
	 * @param Rate			모듈의 스폰 속도. (출력)
	 *
	 * @return    bool      SpawnRate를 무시해야 한다면 false.
	 * SpawnRate를 계속 처리해야 한다면 true.
	 */
	virtual bool GetSpawnAmount(const FContext& Context, int32 Offset, float OldLeftover,
	   float DeltaTime, int32& Number, float& Rate)
	{
		return bProcessSpawnRate;
	}

	/**
	 * 이 모듈의 최대 스폰 속도를 가져온다...
	 * 사용될 수 있는 파티클 수를 추정하는 데 사용된다.
	 *
	 * @return    float     최대 스폰 속도
	 */
	virtual float GetMaximumSpawnRate() { return 0.0f; }

	/**
	 * 이 모듈의 추정 스폰 속도를 가져온다...
	 * 사용될 수 있는 파티클 수를 추정하는 데 사용된다.
	 *
	 * @return    float		추정 스폰 속도
	 */
	virtual float GetEstimatedSpawnRate() { return 0.0f; }
};

UCLASS()
class UParticleModuleSpawn : public UParticleModuleSpawnBase
{
	DECLARE_CLASS(UParticleModuleSpawn, UParticleModuleSpawnBase)

public:
	// @todo 현재는 float를 사용하지만, 언리얼엔진에서는 FRawDistributionFloat 사용한다. 이후 필요에따라 커브 데이터를 활용할 수 있다.
	float Rate;

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
