#include "pch.h"
#include "ParticleModuleVelocity.h"

#include "ParticleEmitterInstances.h"
#include "ParticleHelper.h"
#include "ParticleLODLevel.h"
#include "ParticleModuleRequired.h"
#include "ParticleSystemComponent.h"

IMPLEMENT_CLASS(UParticleModuleVelocity, UParticleModule)

UParticleModuleVelocity::UParticleModuleVelocity()
	: bInWorldSpace(false)
	, bApplyOwnerScale(true)
	, StartVelocity(FVector::Zero())
	, StartVelocityRadial(0.0f)
{
	bSpawnModule = true;
	bUpdateModule = false;
}

void UParticleModuleVelocity::Spawn(const FSpawnContext& Context)
{
	FParticleEmitterInstance* Owner = &Context.Owner;

	SPAWN_INIT;
	{
		// @todo 초기 벡터를 특정 분포를 이용해 계산해야 한다.
		FVector Vel = StartVelocity;

		FVector FromOrigin = (Particle.Location - Owner->EmitterToSimulation.TransformPosition(FVector::Zero())).GetSafeNormal();

		FVector OwnerScale(1.0f);
		if (bApplyOwnerScale)
		{
			OwnerScale = Owner->Component->GetRelativeScale();
		}

		UParticleLODLevel* LODLevel = Owner->GetCurrentLODLevelChecked();

		if (LODLevel->RequiredModule->bUseLocalSpace)
		{
			if (bInWorldSpace)
			{
				FMatrix InvMat = Owner->SimulationToWorld.Inverse();
				Vel = InvMat.TransformVector(Vel);
			}
			else
			{
				Vel = Owner->EmitterToSimulation.TransformVector(Vel);
			}
		}
		else
		{
			if (!bInWorldSpace)
			{
				Vel = Owner->EmitterToSimulation.TransformVector(Vel);
			}
		}

		Vel = Vel * OwnerScale;

		Vel += FromOrigin * (StartVelocityRadial * OwnerScale.GetMaxValue());

		Particle.Velocity += Vel;
		Particle.BaseVelocity += Vel;
	}
}
