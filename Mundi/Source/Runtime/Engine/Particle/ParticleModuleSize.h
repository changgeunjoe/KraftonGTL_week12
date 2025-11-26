#pragma once
#include "ParticleModule.h"
#include "UParticleModuleSize.generated.h"
#include "Source/Runtime/Engine/Distribution/Distributions.h"

/**
 * 파티클의 초기 크기를 설정하는 모듈.
 * 스폰 시 파티클의 XYZ 크기를 정의한다.
 */
UCLASS(DisplayName="Initial Size", Description="파티클 초기 크기 설정")
class UParticleModuleSize : public UParticleModule
{
	GENERATED_REFLECTION_BODY()

public:
	/** 파티클의 초기 크기 */
	UPROPERTY(EditAnywhere, Category="Size")
	FRawDistributionVector StartSize;

public:
	UParticleModuleSize();

	virtual ~UParticleModuleSize() = default;

	void InitializeDefaults();

	//~Begin UParticleModule 인터페이스
	virtual void Spawn(const FSpawnContext& Context) override;
	//~End UParticleModule 인터페이스
};
