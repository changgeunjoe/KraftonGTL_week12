#include "pch.h"
#include "ParticleModuleLifetime.h"

#include "ParticleEmitterInstances.h"
#include "ParticleHelper.h"

IMPLEMENT_CLASS(UParticleModuleLifetimeBase, UParticleModule)

IMPLEMENT_CLASS(UParticleModuleLifetime, UParticleModuleLifetimeBase)

UParticleModuleLifetime::UParticleModuleLifetime()
	: Lifetime(1.0f)
	, LifetimeMin(1.0f)
	, bUseLifetimeRange(false)
{
	bSpawnModule = true;
	bUpdateModule = false;
}

void UParticleModuleLifetime::Spawn(const FSpawnContext& Context)
{
	SpawnEx(Context);
}

float UParticleModuleLifetime::GetMaxLifetime()
{
	if (bUseLifetimeRange)
	{
		return FMath::Max(Lifetime, LifetimeMin);
	}
	return Lifetime;
}

float UParticleModuleLifetime::GetLifetimeValue(const FSpawnContext& Context, float InTime, void* Data)
{
	if (bUseLifetimeRange)
	{
		// @todo 메르센 트위스터 난수 생성기 멤버변수로 사용
	}
	return Lifetime;
}

void UParticleModuleLifetime::SpawnEx(const FSpawnContext& Context)
{
	SPAWN_INIT;
	{
		FParticleEmitterInstance* Owner = &Context.Owner;

		// float MaxLifetime = Lifetime.GetValue(Owner->EmitterTime, Context.GetDistributionData(), InRandomStream);
		float MaxLifetime = GetMaxLifetime();
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



