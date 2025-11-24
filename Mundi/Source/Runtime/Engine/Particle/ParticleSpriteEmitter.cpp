#include "pch.h"
#include "ParticleSpriteEmitter.h"

#include "ParticleEmitterInstances.h"
#include "ParticleLODLevel.h"
#include "ParticleModuleTypeDataBase.h"

IMPLEMENT_CLASS(UParticleSpriteEmitter, UParticleEmitter)

FParticleEmitterInstance* UParticleSpriteEmitter::CreateInstance(UParticleSystemComponent* InComponent)
{
	if (LODLevels.Num() == 0)
	{
		return nullptr;
	}

	FParticleEmitterInstance* Instance = nullptr;

	UParticleLODLevel* LODLevel = GetLODLevel(0);
	assert(LODLevel);

	if (LODLevel->TypeDataModule)
	{
		Instance = LODLevel->TypeDataModule->CreateInstance(this, InComponent);
	}
	else
	{
		Instance = new FParticleSpriteEmitterInstance(InComponent);
		assert(Instance);
		Instance->InitParameters(this);
	}

	if (Instance)
	{
		Instance->CurrentLODLevelIndex = 0;
		Instance->CurrentLODLevel = LODLevels[Instance->CurrentLODLevelIndex];
		Instance->Init();
	}

	return Instance;
}

void UParticleSpriteEmitter::SetToSensibleDefaults()
{
	UParticleLODLevel* LODLevel = LODLevels[0];

	// @todo
}
