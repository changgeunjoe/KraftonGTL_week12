#include "pch.h"
#include "ParticleModuleSpawn.h"

bool UParticleModuleSpawn::GetSpawnAmount(const FContext& Context, int32 Offset, float OldLeftover, float DeltaTime,
	int32& Number, float& Rate)
{
	return false;
}

float UParticleModuleSpawn::GetMaximumSpawnRate()
{
	return Rate * RateScale;
}

float UParticleModuleSpawn::GetEstimatedSpawnRate()
{
	return Rate * RateScale;
}
