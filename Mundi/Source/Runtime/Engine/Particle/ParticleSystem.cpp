#include "pch.h"

#include "ParticleSystem.h"
#include "ParticleEmitter.h"
#include "ParticleLODLevel.h"
#include "ParticleModuleTypeDataBase.h"

IMPLEMENT_CLASS(UParticleSystem, UObject)

UParticleSystem::~UParticleSystem()
{
	for (UParticleEmitter* Emitter : Emitters)
	{
		if (Emitter)
		{
			DeleteObject(Emitter);
		}
	}
	Emitters.Empty();
}

UParticleEmitter* UParticleSystem::AddEmitter(UClass* EmitterClass)
{
	if (!EmitterClass || !EmitterClass->IsChildOf(UParticleEmitter::StaticClass()))
	{
		assert(false);
		return nullptr;
	}

	UParticleEmitter* NewEmitter = Cast<UParticleEmitter>(NewObject(EmitterClass));
	if (NewEmitter)
	{
		NewEmitter->SetEmitterName("Particle Emitter");

		NewEmitter->AddLODLevel();

		Emitters.Add(NewEmitter);

		UpdateAllModuleLists();
	}

	return NewEmitter;
}

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
			Emitter->UpdateModuleLists();

			if (Emitter->LODLevels.Num() > 0)
			{
				UParticleLODLevel* HighLODLevel = Emitter->LODLevels[0];
				if (HighLODLevel != nullptr && HighLODLevel->TypeDataModule != nullptr)
				{
					HighLODLevel->TypeDataModule->CacheModuleInfo(Emitter);
				}
			}
			Emitter->CacheEmitterModuleInfo();
		}
	}
}
