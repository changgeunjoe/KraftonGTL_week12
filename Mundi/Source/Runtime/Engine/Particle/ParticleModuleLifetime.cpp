#include "pch.h"
#include "ParticleModuleLifetime.h"

#include "ParticleEmitterInstances.h"
#include "ParticleHelper.h"

IMPLEMENT_CLASS(UParticleModuleLifetimeBase, UParticleModule)

IMPLEMENT_CLASS(UParticleModuleLifetime, UParticleModuleLifetimeBase)

UParticleModuleLifetime::UParticleModuleLifetime()
{
	bSpawnModule = true;
	bUpdateModule = false;

	InitializeDefaults();
}

void UParticleModuleLifetime::Spawn(const FSpawnContext& Context)
{
	SpawnEx(Context);
}

void UParticleModuleLifetime::InitializeDefaults()
{
	if (!Lifetime.IsCreated())
	{
		UDistributionFloatUniform* Dist = NewObject<UDistributionFloatUniform>();
		Dist->Min = 1.0f;
		Dist->Max = 1.0f;
		Lifetime.Distribution = Dist;
	}
}

float UParticleModuleLifetime::GetMaxLifetime()
{
	if (Lifetime.IsCreated())
	{
		float Min, Max;
		Lifetime.GetOutRange(Min, Max);
		return Max;
	}
	return 0.0f;
}

float UParticleModuleLifetime::GetLifetimeValue(const FSpawnContext& Context, float InTime, void* Data)
{
	return Lifetime.GetValue(InTime, Context.Owner.Component);
}

void UParticleModuleLifetime::SpawnEx(const FSpawnContext& Context)
{
	SPAWN_INIT;
	{
		float MaxLifetime = GetLifetimeValue(Context, Context.SpawnTime);
		if (Particle.OneOverMaxLifetime > 0.f)
		{
			Particle.OneOverMaxLifetime = 1.f / (MaxLifetime + 1.f / Particle.OneOverMaxLifetime);
		}
		else
		{
			Particle.OneOverMaxLifetime = MaxLifetime > 0.f ? 1.f / MaxLifetime : 0.f;
		}
		// 일부 파티클들은 RelativeTime이 1.0 보다 큰 경우 수명이 완료된 것으로 판정함, 따라서 해당 경우는 업데이트하지 않음
		Particle.RelativeTime = Particle.RelativeTime > 1.0f ? Particle.RelativeTime : Context.SpawnTime * Particle.OneOverMaxLifetime;
	}
}



