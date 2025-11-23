#include "pch.h"

#include "ParticleEmitter.h"

#include "ParticleEmitterInstance.h"
#include "ParticleLODLevel.h"
#include "ParticleModuleTypeDataBase.h"

IMPLEMENT_CLASS(UParticleEmitter, UObject)

UParticleEmitter::UParticleEmitter()
	: PeakActiveParticles(0)
	, InitialAllocationCount(0)
	, ParticleSize(0)
	, ReqInstanceBytes(0)
{
}

void UParticleEmitter::UpdateModuleLists()
{
	for (int32 LODIndex = 0; LODIndex < LODLevels.Num(); LODIndex++)
	{
		UParticleLODLevel* LODLevel = LODLevels[LODIndex];
		if (LODLevel)
		{
			LODLevel->UpdateModuleLists();
		}
	}
	Build();
}

FParticleEmitterInstance* UParticleEmitter::CreateInstance(UParticleSystemComponent* InComponent)
{
	return nullptr;
}

void UParticleEmitter::SetEmitterName(FName Name)
{
	EmitterName = Name;
}

FName& UParticleEmitter::GetEmitterName()
{
	return EmitterName;
}

UParticleLODLevel* UParticleEmitter::GetCurrentLODLevel(FParticleEmitterInstance* Instance)
{
	if (Instance)
	{
		return Instance->CurrentLODLevel;
	}
	return nullptr;
}

UParticleLODLevel* UParticleEmitter::GetLODLevel(int32 LODLevel)
{
	if (LODLevel >= LODLevels.Num())
	{
		return nullptr;
	}
	return LODLevels[LODLevel];
}

bool UParticleEmitter::CalculateMaxActiveParticleCount()
{
	int32 CurrMaxAPC = 0;

	int32 MaxCount = 0;

	for (int32 LODIndex = 0; LODIndex < LODLevels.Num(); LODIndex++)
	{
		UParticleLODLevel* LODLevel = LODLevels[LODIndex];
		if (LODLevel && LODLevel->bEnabled)
		{
			bool bForceMaxCount = false;

			if ((LODLevel->Level == 0) && (LODLevel->TypeDataModule != nullptr))
			{
				// @todo Check for beams or trails
			}

			int32 LODMaxAPC = LODLevel->CalculateMaxActiveParticleCount();
			if (bForceMaxCount == true)
			{
				LODLevel->PeakActiveParticles = MaxCount;
				LODMaxAPC = MaxCount;
			}

			if (LODMaxAPC > CurrMaxAPC)
			{
				CurrMaxAPC = LODMaxAPC;
			}
		}
	}

	return true;
}

void UParticleEmitter::CacheEmitterModuleInfo()
{
	ModuleOffsetMap.Empty();
	ModuleInstanceOffsetMap.Empty();
	ModulesNeedingInstanceData.Empty();
	ParticleSize = 0;
	ReqInstanceBytes = 0;

	UParticleLODLevel* HighLODLevel = GetLODLevel(0);
	assert(HighLODLevel);

	UParticleModuleTypeDataBase* HighTypeData = HighLODLevel->TypeDataModule;
	if (HighTypeData)
	{
		// @todo
	}

	UParticleModuleRequired* RequiredModule = HighLODLevel->RequiredModule;
	assert(RequiredModule);

	for (int32 ModuleIdx = 0; ModuleIdx < HighLODLevel->Modules.Num(); ModuleIdx++)
	{
		UParticleModule* ParticleModule = HighLODLevel->Modules[ModuleIdx];
		assert(ParticleModule);

		if (ParticleModule->IsA(UParticleModuleTypeDataBase::StaticClass()) == false)
		{
			int32 ReqBytes = ParticleModule->RequiredBytes(HighTypeData);

			if (ReqBytes)
			{
				ModuleOffsetMap.Add(ParticleModule, ParticleSize);

				ParticleSize += ReqBytes;
			}
		}

		int32 TempInstanceBytes = ParticleModule->RequiredBytesPerInstance();
		if (TempInstanceBytes > 0)
		{
			ModuleInstanceOffsetMap.Add(ParticleModule, ReqInstanceBytes);
			ModulesNeedingInstanceData.Add(ParticleModule);

			for (int32 LODIdx = 1; LODIdx < LODLevels.Num(); LODIdx++)
			{
				UParticleLODLevel* CurLODLevel = LODLevels[LODIdx];
				ModuleInstanceOffsetMap.Add(CurLODLevel->Modules[ModuleIdx], ReqInstanceBytes);
			}
			ReqInstanceBytes += TempInstanceBytes;
		}
	}
}

void UParticleEmitter::Build()
{
	const int32 LODCount = LODLevels.Num();
	if (LODCount > 0)
	{
		UParticleLODLevel* HighLODLevel = LODLevels[0];
		assert(HighLODLevel);
		if (HighLODLevel->TypeDataModule != nullptr)
		{
			// @todo TypeDataModule
		}
		CacheEmitterModuleInfo();
	}
}
