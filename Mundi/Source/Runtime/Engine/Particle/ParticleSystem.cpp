#include "pch.h"

#include "ParticleSystem.h"

#include "ParticleEmitter.h"
#include "ParticleLODLevel.h"

IMPLEMENT_CLASS(UParticleSystem, UObject)

bool UParticleSystem::CalculateMaxActiveParticleCounts()
{
	bool bSuccess = true;

	for (int32 EmitterIndex = 0; EmitterIndex < Emitters.Num(); EmitterIndex++)
	{
		UParticleEmitter* Emitter = Emitters[EmitterIndex];
		if (Emitter)
		{
			if (Emitter->CalculateMaxActiveParticleCount() == false)
			{
				bSuccess = false;
			}
		}
	}

	return bSuccess;
}

void UParticleSystem::UpdateAllModuleLists()
{
	for (int32 EmitterIdx = 0; EmitterIdx < Emitters.Num(); EmitterIdx++)
	{
		UParticleEmitter* Emitter = Emitters[EmitterIdx];
		if (Emitter != nullptr)
		{
			for (int32 LODIdx = 0; LODIdx < Emitter->LODLevels.Num(); LODIdx++)
			{
				UParticleLODLevel* LODLevel = Emitter->LODLevels[LODIdx];
				if (LODLevel != nullptr)
				{
					LODLevel->UpdateModuleLists();
				}
			}

			if (Emitter->LODLevels.Num() > 0)
			{
				UParticleLODLevel* HighLODLevel = Emitter->LODLevels[0];
				if (HighLODLevel != nullptr && HighLODLevel->TypeDataModule != nullptr)
				{
					// @todo
					// HighLODLevel->TypeDataModule->CacheModuleInfo(Emitter);
				}
			}
			Emitter->CacheEmitterModuleInfo();
		}
	}
}
