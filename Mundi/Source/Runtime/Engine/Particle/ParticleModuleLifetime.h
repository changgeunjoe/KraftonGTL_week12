#pragma once
#include <random>

#include "ParticleModule.h"

UCLASS()
class UParticleModuleLifetimeBase : public UParticleModule
{
	DECLARE_CLASS(UParticleModuleLifetimeBase, UParticleModule)
public:
	UParticleModuleLifetimeBase()
	{
	}

	virtual ~UParticleModuleLifetimeBase() {}

	/**
	 * 이 모듈이 반환할 수 있는 최대 수명 반환
	 * (메모리 프리사이징 계산 시 사용됨)
	 */
	virtual float GetMaxLifetime()
	{
		return 0.0f;
	}

	/**
	 * 주어진 시간에서의 수명 값 반환
	 * (파티클 스폰 시 호출됨)
	 */
	virtual float GetLifetimeValue(const FSpawnContext& Context, float InTime, void* Data = nullptr)
	{
		return 0.0f;
	}
};

UCLASS()
class UParticleModuleLifetime : public UParticleModuleLifetimeBase
{
	DECLARE_CLASS(UParticleModuleLifetime, UParticleModuleLifetimeBase)

public:
	/** 파티클의 기본 수명 (초 단위) */
	float Lifetime;

	/** 랜덤 범위 사용 시 최소 수명 */
	float LifetimeMin;

	/** 랜덤 범위 사용 여부 */
	bool bUseLifetimeRange;

public:
	UParticleModuleLifetime();
	virtual ~UParticleModuleLifetime() = default;

	//~ Begin UParticleModule Interface
	virtual void Spawn(const FSpawnContext& Context) override;
	//~ End UParticleModule Interface

	//~ Begin UParticleModuleLifetimeBase Interface
	/** 최대 수명 반환 (메모리 추정용) */
	virtual float GetMaxLifetime() override;

	/** 실제 수명 값 계산 */
	virtual float GetLifetimeValue(const FSpawnContext& Context, float InTime, void* Data = nullptr) override;
	//~ End UParticleModuleLifetimeBase Interface

protected:
	void SpawnEx(const FSpawnContext& Context);

private:
	/** 메르센 트위스터 난수 생성기 */
	std::mt19937 RandomStream;
};
