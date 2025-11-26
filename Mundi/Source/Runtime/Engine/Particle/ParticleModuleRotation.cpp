#include "pch.h"
#include "ParticleModuleRotation.h"

#include "ParticleHelper.h"

UParticleModuleRotation::UParticleModuleRotation()
{
	bSpawnModule = true;

	InitializeDefaults();
}

void UParticleModuleRotation::InitializeDefaults()
{
	if (!StartRotation.IsCreated())
	{
		UDistributionFloatUniform* DistributionStartRotation = NewObject<UDistributionFloatUniform>();
		DistributionStartRotation->Min = 0.0f;
		DistributionStartRotation->Max = 1.0f;
		StartRotation.Distribution = DistributionStartRotation;
	}
}

void UParticleModuleRotation::Spawn(const FSpawnContext& Context)
{
	FParticleEmitterInstance* Owner = &Context.Owner;

	SPAWN_INIT
	{
		Particle.Rotation += (PI/180.0f) * 360.0f * StartRotation.GetValue(Owner->EmitterTime);
	}
}
