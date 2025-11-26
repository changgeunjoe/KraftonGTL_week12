#pragma once
#include "ParticleModule.h"
#include "Source/Runtime/Engine/Distribution/Distributions.h"
#include "UParticleModuleVelocity.generated.h"

/**
 * 파티클의 초기 속도를 설정하는 모듈.
 * 스폰 시 파티클에 적용할 방향 속도와 방사형 속도를 정의한다.
 */
UCLASS(DisplayName="Initial Velocity", Description="파티클 초기 속도 설정")
class UParticleModuleVelocity : public UParticleModule
{
	GENERATED_REFLECTION_BODY()
public:
	/** True일 경우, 속도를 월드 공간에서 정의된 것으로 간주한다. */
	UPROPERTY(EditAnywhere, Category="Options")
	bool bInWorldSpace;

	/** True일 경우, 파티클 시스템 컴포넌트의 스케일을 속도 값에 적용한다. */
	UPROPERTY(EditAnywhere, Category="Options")
	bool bApplyOwnerScale;

	/** 파티클이 스폰될 때 적용할 기본 속도 */
	UPROPERTY(EditAnywhere, Category="Velocity")
	FRawDistributionVector StartVelocity;

	/** 이미터 원점으로부터 파티클 위치 방향으로 적용할 방사형 속도 */
	UPROPERTY(EditAnywhere, Category="Velocity")
	FRawDistributionFloat StartVelocityRadial;

public:
	UParticleModuleVelocity();
	virtual ~UParticleModuleVelocity() = default;

	void InitializeDefaults();

	//~ Begin UParticleModule Interface
	virtual void Spawn(const FSpawnContext& Context) override;
	//~ End UParticleModule Interface
};
