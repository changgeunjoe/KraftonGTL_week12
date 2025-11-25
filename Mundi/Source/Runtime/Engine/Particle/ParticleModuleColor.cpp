#include "pch.h"
#include "ParticleModuleColor.h"
#include "ParticleHelper.h"

UParticleModuleColor::UParticleModuleColor()
{
	InitializeDefaults();
}

void UParticleModuleColor::InitializeDefaults()
{
	if (!StartColor.IsCreated())
	{
		StartColor.Distribution = NewObject<UDistributionVectorConstant>();
	}

	if (!StartAlpha.IsCreated())
	{
		UDistributionFloatConstant* DistributionStartAlpha = NewObject<UDistributionFloatConstant>();
		DistributionStartAlpha->Constant = 1.0f;
		StartAlpha.Distribution = DistributionStartAlpha;
	}
}

void UParticleModuleColor::Spawn(const FSpawnContext& Context)
{
	FParticleEmitterInstance* Owner = &Context.Owner;

	SPAWN_INIT
	{
		FVector ColorVec = (FVector)StartColor.GetValue(Owner->EmitterTime);
		float Alpha = StartAlpha.GetValue(Owner->EmitterTime);
		Particle.Color = ColorVec;
		Particle.Color.A = Alpha;
		Particle.BaseColor = Particle.Color;
	}
}
