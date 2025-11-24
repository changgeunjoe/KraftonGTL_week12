#pragma once
#include "ParticleModuleSpawnBase.h"
#include "UParticleModuleSpawn.generated.h"

UCLASS()
class UParticleModuleSpawn : public UParticleModuleSpawnBase
{
	GENERATED_REFLECTION_BODY()

public:
	// @todo 현재는 float를 사용하지만, 언리얼엔진에서는 FRawDistributionFloat 사용한다. 이후 필요에따라 커브 데이터를 활용할 수 있다.
	UPROPERTY(EditAnywhere, Category="Spawn")
	float Rate;

	UPROPERTY(EditAnywhere, Category="Spawn")
	float RateScale;

public:
	UParticleModuleSpawn();

	virtual ~UParticleModuleSpawn() = default;

	//~Begin UParticleModuleSpawnBase Interface
	virtual bool GetSpawnAmount(const FContext& Context, int32 Offset, float OldLeftover,
		float DeltaTime, int32& Number, float& Rate) override;
	virtual float GetMaximumSpawnRate() override;
	virtual float GetEstimatedSpawnRate() override;
	//~End UParticleModuleSpawnBase Interface
};
