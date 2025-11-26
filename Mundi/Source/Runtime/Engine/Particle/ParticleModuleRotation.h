#pragma once
#include "ParticleModule.h"
#include "Source/Runtime/Engine/Distribution/Distributions.h"
#include "UParticleModuleRotation.generated.h"

UCLASS()
class UParticleModuleRotation : public UParticleModule
{
	GENERATED_REFLECTION_BODY()
public:
	/**
	 *	파티클의 초기 회전 (1 = 360 도)
	 *	EmitterTime으로 값을 가져온다.
	 */
	UPROPERTY(EditAnywhere, Category="ParticleModuleRotation")
	FRawDistributionFloat StartRotation;

public:
	UParticleModuleRotation();

	virtual ~UParticleModuleRotation() = default;

	void InitializeDefaults();

	//~Begin UParticleModule 인터페이스
	virtual void Spawn(const FSpawnContext& Context) override;
	//~End UParticleModule 인터페이스
};
