#include "pch.h"
#include "ParticleModuleRequired.h"
#include "ParticleSpriteEmitter.h"

IMPLEMENT_CLASS(UParticleModuleRequired, UParticleModule)

UParticleModuleRequired::UParticleModuleRequired()
	: Material(nullptr)
	, ScreenAlignment(PSA_Square)
	, bUseLocalSpace(false)
	, EmitterDuration(1.0f)
	, EmitterDurationLow(0.0f)
	, EmitterLoops(0)
	, EmitterDelay(0.0f)
{
	bSpawnModule = true;
	bUpdateModule = true;
}

void UParticleModuleRequired::InitializeDefaults()
{
	EmitterDuration = 1.0f;
	EmitterLoops = 0;
}

void UParticleModuleRequired::SetToSensibleDefaults(UParticleEmitter* Owner)
{
	Super::SetToSensibleDefaults(Owner);
}
