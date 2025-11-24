#pragma once
#include "ParticleModuleLifetimeBase.h"
#include "UParticleModuleLifetime.generated.h"

UCLASS()
class UParticleModuleLifetime : public UParticleModuleLifetimeBase
{
	GENERATED_REFLECTION_BODY()

public:
	/** 파티클의 기본 수명 (초 단위) */
	UPROPERTY(EditAnywhere, Category="Lifetime")
	float Lifetime;

	/** 랜덤 범위 사용 시 최소 수명 */
	UPROPERTY(EditAnywhere, Category="Lifetime")
	float LifetimeMin;

	/** 랜덤 범위 사용 여부 */
	UPROPERTY(EditAnywhere, Category="Lifetime")
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
};
