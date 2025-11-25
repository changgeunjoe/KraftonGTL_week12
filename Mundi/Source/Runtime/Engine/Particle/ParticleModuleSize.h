#pragma once
#include "ParticleModule.h"
#include "UParticleModuleSize.generated.h"
#include "Source/Runtime/Engine/Distribution/Distributions.h"

UCLASS()
class UParticleModuleSize : public UParticleModule
{
	GENERATED_REFLECTION_BODY()

public:
	/** 파티클의 초기 크기	*/
	/** @todo PROPERTY */
	FRawDistributionVector StartSize;

public:
	UParticleModuleSize();

	virtual ~UParticleModuleSize() = default;

	void InitializeDefaults();

	//~Begin UParticleModule 인터페이스
	virtual void Spawn(const FSpawnContext& Context) override;
	//~End UParticleModule 인터페이스
};
