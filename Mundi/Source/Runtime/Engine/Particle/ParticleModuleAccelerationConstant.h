#pragma once
#include "ParticleModule.h"
#include "Source/Runtime/Engine/Distribution/Distributions.h"

#include "UparticleModuleAccelerationConstant.generated.h"

UCLASS()
class UParticleModuleAccelerationConstant : public UParticleModule
{
	GENERATED_REFLECTION_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Acceleration")
	FRawDistributionVector Acceleration;

public:
	UParticleModuleAccelerationConstant();
	virtual ~UParticleModuleAccelerationConstant() = default;

	virtual void InitializeDefaults();

	//~Begin UParticleModule Interface
	virtual void Update(const FUpdateContext& Context) override;
	//~End UParticleModule Interface
};
