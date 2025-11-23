#include "pch.h"

#include "ParticleModule.h"

IMPLEMENT_CLASS(UParticleModule, UObject)

void UParticleModule::Spawn(const FSpawnContext& Context)
{
}

void UParticleModule::Update(const FUpdateContext& Context)
{
}

void UParticleModule::FinalUpdate(const FUpdateContext& Context)
{
}

uint32 UParticleModule::RequiredBytes(UParticleModuleTypeDataBase* TypeData)
{
	return 0;
}

uint32 UParticleModule::RequiredBytesPerInstance()
{
	return 0;
}

uint32 UParticleModule::PrepPerInstanceBlock(FParticleEmitterInstance* Owner, void* InstData)
{
	return 0xffffffff;
}
