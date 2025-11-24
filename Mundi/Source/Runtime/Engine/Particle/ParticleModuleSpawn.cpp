#include "pch.h"
#include "ParticleModuleSpawn.h"

IMPLEMENT_CLASS(UParticleModuleSpawnBase, UParticleModule)

IMPLEMENT_CLASS(UParticleModuleSpawn, UParticleModuleSpawnBase)

UParticleModuleSpawn::UParticleModuleSpawn()
	: Rate(10.0f)
	, RateScale(1.0f)
{
	bSpawnModule = false;
	bUpdateModule = false;
	bProcessSpawnRate = true;
}

bool UParticleModuleSpawn::GetSpawnAmount(const FContext& Context,
	int32 Offset, float OldLeftover, float DeltaTime, int32& Number, float& Rate)
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
