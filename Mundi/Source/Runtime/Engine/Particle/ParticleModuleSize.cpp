#include "pch.h"
#include "ParticleModuleSize.h"

#include "ParticleHelper.h"

UParticleModuleSize::UParticleModuleSize()
{
	bSpawnModule = true;

	InitializeDefaults();
}

void UParticleModuleSize::InitializeDefaults()
{
	if (!StartSize.IsCreated())
	{
		UDistributionVectorUniform* DistributionStartSize = NewObject<UDistributionVectorUniform>();
		DistributionStartSize->Min = FVector(1.0f, 1.0f, 1.0f);
		DistributionStartSize->Max = FVector(1.0f, 1.0f, 1.0f);
		StartSize.Distribution = DistributionStartSize;
	}
}

void UParticleModuleSize::Spawn(const FSpawnContext& Context)
{
	FParticleEmitterInstance* Owner = &Context.Owner;

	SPAWN_INIT;
	FVector Size = StartSize.GetValue(Owner->EmitterTime);
	Particle.Size += Size;
	Particle.BaseSize += Size;
}
