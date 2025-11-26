#pragma once
#include "ParticleModule.h"
#include "Source/Runtime/Engine/Distribution/Distributions.h"
#include "UParticleModuleColorOverLife.generated.h"

UCLASS()
class UParticleModuleColorOverLife : public UParticleModule
{
	GENERATED_REFLECTION_BODY()
public:
	UPROPERTY(EditAnywhere, Category=Color)
	FRawDistributionVector ColorOverLife;

	UPROPERTY(EditAnywhere, Category=Color)
	FRawDistributionFloat AlphaOverLife;

public:
	UParticleModuleColorOverLife();
	virtual ~UParticleModuleColorOverLife() = default;

	void InitializeDefaults();

	//~Begin UParticleModule Interface
	virtual void Spawn(const FSpawnContext& Context) override;
	virtual void Update(const FUpdateContext& Context) override;
	//~End UParticleModule Interface
};
