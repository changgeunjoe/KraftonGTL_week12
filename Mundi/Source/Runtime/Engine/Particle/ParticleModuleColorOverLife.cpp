#include "pch.h"
#include "ParticleModuleColorOverLife.h"

#include "ParticleHelper.h"

UParticleModuleColorOverLife::UParticleModuleColorOverLife()
{
	bSpawnModule = true;
	bUpdateModule = true;

	InitializeDefaults();
}

void UParticleModuleColorOverLife::InitializeDefaults()
{
	if (!ColorOverLife.IsCreated())
	{
		ColorOverLife.Distribution = NewObject<UDistributionVectorConstant>();
	}

	if (!AlphaOverLife.IsCreated())
	{
		UDistributionFloatConstant* DistAlpha = NewObject<UDistributionFloatConstant>();
		DistAlpha->Constant = 1.0f;
		AlphaOverLife.Distribution = DistAlpha;
	}
}

void UParticleModuleColorOverLife::Spawn(const FSpawnContext& Context)
{
	SPAWN_INIT
	{
		float RelTime = Particle.RelativeTime;

		FVector ColorScale = ColorOverLife.GetValue(RelTime, Context.GetDistributionData());
		float AlphaScale = AlphaOverLife.GetValue(RelTime, Context.GetDistributionData());

		Particle.Color.R = Particle.BaseColor.R * ColorScale.X;
		Particle.Color.G = Particle.BaseColor.G * ColorScale.Y;
		Particle.Color.B = Particle.BaseColor.B * ColorScale.Z;
		Particle.Color.A = Particle.BaseColor.A * AlphaScale;
	}
}

void UParticleModuleColorOverLife::Update(const FUpdateContext& Context)
{
	FParticleEmitterInstance* Owner = &Context.Owner;

	if (Owner == nullptr ||
		Owner->ActiveParticles <= 0 ||
		Owner->ParticleData == nullptr ||
		Owner->ParticleIndices == nullptr)
	{
		return;
	}

	BEGIN_UPDATE_LOOP;
	{
		float RelTime = Particle.RelativeTime;

		FVector ColorScale = ColorOverLife.GetValue(RelTime, Context.GetDistributionData());
		float AlphaScale = AlphaOverLife.GetValue(RelTime, Context.GetDistributionData());

		Particle.Color.R = Particle.BaseColor.R * ColorScale.X;
		Particle.Color.G = Particle.BaseColor.G * ColorScale.Y;
		Particle.Color.B = Particle.BaseColor.B * ColorScale.Z;
		Particle.Color.A = Particle.BaseColor.A * AlphaScale;
	}
	END_UPDATE_LOOP;
}
