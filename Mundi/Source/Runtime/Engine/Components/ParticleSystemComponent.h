#pragma once

#include "PrimitiveComponent.h"

struct FDynamicEmitterDataBase;
class UParticleSystem;

class UFXSystemComponent : public UPrimitiveComponent
{

};

class UParticleSystemComponent : public UFXSystemComponent
{
public:
	UParticleSystemComponent();

	virtual ~UParticleSystemComponent();

	virtual void InitParticles();

	//~ Begin UActorComponent Interface //
	virtual void TickComponent(float DeltaTime) override;
	//~ End UActorComponent Interface //

	void ClearDynamicData();

	virtual void UpdateDynamicData();

private:
	UParticleSystem* Template;

	TArray<UMaterialInterface*> EmitterMaterials;

	TArray<struct FParticleEmitterInstance*> EmitterInstances;

	TArray<FDynamicEmitterDataBase*> EmitterRenderData;

	bool bWasComplete;

	int32 LODLevel;

	int32 TotalActiveParticles;
};
