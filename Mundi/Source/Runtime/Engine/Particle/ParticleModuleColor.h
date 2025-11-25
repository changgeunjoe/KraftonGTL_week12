#pragma once
#include "ParticleModule.h"
#include "Source/Runtime/Engine/Distribution/Distributions.h"
#include "UParticleModuleColor.generated.h"

UCLASS()
class UParticleModuleColor : public UParticleModule
{
	GENERATED_REFLECTION_BODY()

public:
	/** 이미터 시간에 대한 함수로써의 파티클에 대한 초기 색상		*/
	/** @todo PROPERTY */
	UPROPERTY(EditAnywhere, Category="ParticleModuleColor")
	FRawDistributionVector StartColor;

	/** 이미터 시간에 대한 함수로써의 파티클에 대한 초기 알파값		*/
	/** @todo PROPERTY */
	UPROPERTY(EditAnywhere, Category="ParticleModuleColor")
	FRawDistributionFloat StartAlpha;

public:
	UParticleModuleColor();

	virtual ~UParticleModuleColor() = default;

	void InitializeDefaults();

	//~Begin UParticleModule Interface
	virtual void Spawn(const FSpawnContext& Context) override;
	//~End UParticleModule Interface
};
