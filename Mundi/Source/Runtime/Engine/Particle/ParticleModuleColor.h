#pragma once
#include "ParticleModule.h"
#include "Source/Runtime/Engine/Distribution/Distributions.h"
#include "UParticleModuleColor.generated.h"

/**
 * 파티클의 초기 색상을 설정하는 모듈.
 * 스폰 시 파티클의 RGB 색상과 알파값을 정의한다.
 */
UCLASS(DisplayName="Initial Color", Description="파티클 초기 색상 설정")
class UParticleModuleColor : public UParticleModule
{
	GENERATED_REFLECTION_BODY()

public:
	/** 파티클의 초기 색상 (RGB) */
	UPROPERTY(EditAnywhere, Category="Color")
	FRawDistributionVector StartColor;

	/** 파티클의 초기 알파값 */
	UPROPERTY(EditAnywhere, Category="Color")
	FRawDistributionFloat StartAlpha;

public:
	UParticleModuleColor();

	virtual ~UParticleModuleColor() = default;

	void InitializeDefaults();

	//~Begin UParticleModule Interface
	virtual void Spawn(const FSpawnContext& Context) override;
	//~End UParticleModule Interface
};
