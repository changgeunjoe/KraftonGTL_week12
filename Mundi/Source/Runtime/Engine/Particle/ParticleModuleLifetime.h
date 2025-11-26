#pragma once
#include "ParticleModuleLifetimeBase.h"
#include "UParticleModuleLifetime.generated.h"
#include "Source/Runtime/Engine/Distribution/Distributions.h"

/**
 * 파티클의 수명을 설정하는 모듈.
 * 파티클이 생성된 후 소멸될 때까지의 시간을 정의한다.
 */
UCLASS(DisplayName="Lifetime", Description="파티클 수명 설정")
class UParticleModuleLifetime : public UParticleModuleLifetimeBase
{
	GENERATED_REFLECTION_BODY()

public:
	/** 파티클의 기본 수명 (초 단위) */
	UPROPERTY(EditAnywhere, Category="Lifetime")
	FRawDistributionFloat Lifetime;

public:
	UParticleModuleLifetime();
	virtual ~UParticleModuleLifetime() = default;

	//~ Begin UParticleModule Interface
	virtual void Spawn(const FSpawnContext& Context) override;
	//~ End UParticleModule Interface

	void InitializeDefaults();

	//~ Begin UParticleModuleLifetimeBase Interface
	/** 최대 수명 반환 (메모리 추정용) */
	virtual float GetMaxLifetime() override;

	/** 실제 수명 값 계산 */
	virtual float GetLifetimeValue(const FSpawnContext& Context, float InTime, void* Data = nullptr) override;
	//~ End UParticleModuleLifetimeBase Interface

protected:
	void SpawnEx(const FSpawnContext& Context);
};
