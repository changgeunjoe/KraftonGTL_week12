#include "pch.h"
#include "ParticleModuleAccelerationConstant.h"

#include "ParticleEmitterInstances.h"
#include "ParticleHelper.h"

UParticleModuleAccelerationConstant::UParticleModuleAccelerationConstant()
{
	bSpawnModule = false;
	bUpdateModule = true;

	InitializeDefaults();
}

void UParticleModuleAccelerationConstant::InitializeDefaults()
{
	if (!Acceleration.IsCreated())
	{
		UDistributionVectorConstant* Dist = NewObject<UDistributionVectorConstant>();
		Dist->Constant = FVector(0.0f, 0.0f, -9.8f);
		Acceleration.Distribution = Dist;
	}
}

void UParticleModuleAccelerationConstant::Update(const FUpdateContext& Context)
{
	FParticleEmitterInstance* Owner = &Context.Owner;

	if (Owner == nullptr ||
		Owner->ActiveParticles <= 0 ||
		Owner->ParticleData == nullptr ||
		Owner->ParticleIndices == nullptr)
	{
		return;
	}

	FVector AccelValue = Acceleration.GetValue(0.0f, Context.GetDistributionData());

	FVector VelocityIncrement = AccelValue * Context.DeltaTime;

	BEGIN_UPDATE_LOOP;
	{
		Particle.BaseVelocity += VelocityIncrement;

		Particle.Velocity += VelocityIncrement;
	}
	END_UPDATE_LOOP;
}
