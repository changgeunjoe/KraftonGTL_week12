#pragma once
#include "ParticleModule.h"
#include "UParticleModuleLifetimeBase.generated.h"

UCLASS()
class UParticleModuleLifetimeBase : public UParticleModule
{
	GENERATED_REFLECTION_BODY()

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
