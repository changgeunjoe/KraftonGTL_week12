#include "pch.h"

#include "ParticleEmitter.h"

#include "ParticleEmitterInstances.h"
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

UParticleEmitter::~UParticleEmitter()
{
	for (UParticleLODLevel* Level : LODLevels)
	{
		if (Level)
		{
			DeleteObject(Level);
		}
	}
	LODLevels.Empty();
}

UParticleLODLevel* UParticleEmitter::AddLODLevel(UParticleLODLevel* LODLevel)
{
	UParticleLODLevel* NewLODLevel = LODLevel;
	if (!NewLODLevel)
	{
		NewLODLevel = NewObject<UParticleLODLevel>();
	}

	if (NewLODLevel)
	{
		NewLODLevel->OwnerEmitter = this;
		NewLODLevel->Level = LODLevels.Num();

		if (NewLODLevel->RequiredModule == nullptr)
		{
			NewLODLevel->AddRequiredModule();
		}

		if (NewLODLevel->SpawnModule == nullptr)
		{
			NewLODLevel->AddSpawnModule();
		}

		LODLevels.Add(NewLODLevel);

		UpdateModuleLists();
	}

	return NewLODLevel;
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

	for (int32 LODIndex = 0; LODIndex < LODLevels.Num(); LODIndex++)
	{
		UParticleLODLevel* LODLevel = LODLevels[LODIndex];
		if (LODLevel && LODLevel->bEnabled)
		{
			if ((LODLevel->Level == 0) && (LODLevel->TypeDataModule != nullptr))
			{
				// @todo Check for beams or trails
			}

			int32 LODMaxAPC = LODLevel->CalculateMaxActiveParticleCount();
			if (LODMaxAPC > CurrMaxAPC)
			{
				CurrMaxAPC = LODMaxAPC;
			}
		}
	}

	PeakActiveParticles = CurrMaxAPC;
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
