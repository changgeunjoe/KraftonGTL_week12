#include "pch.h"

#include "ParticleLODLevel.h"
#include "ParticleModule.h"
#include "ParticleModuleLifetime.h"
#include "ParticleModuleRequired.h"
#include "ParticleModuleSpawn.h"
#include "ParticleModuleTypeDataBase.h"

IMPLEMENT_CLASS(UParticleLODLevel, UObject)

UParticleLODLevel::UParticleLODLevel()
	: Level(0)
	, bEnabled(true)
	, RequiredModule(nullptr)
	, TypeDataModule(nullptr)
	, SpawnModule(nullptr)
	, PeakActiveParticles(0)
{
}

void UParticleLODLevel::UpdateModuleLists()
{
	SpawningModules.Empty();
	SpawnModules.Empty();
	UpdateModules.Empty();

	UParticleModule* Module;
	int32 TypeDataModuleIndex = -1;

	for (int32 i = 0; i < Modules.Num(); i++)
	{
		Module = Modules[i];
		if (!Module)
		{
			continue;
		}

		if (Module->bSpawnModule)
		{
			SpawnModules.Add(Module);
		}
		if (Module->bUpdateModule || Module->bFinalUpdateModule)
		{
			UpdateModules.Add(Module);
		}

		if (Module->IsA(UParticleModuleTypeDataBase::StaticClass()))
		{
			TypeDataModule = Cast<UParticleModuleTypeDataBase>(Module);
			if (!Module->bSpawnModule && !Module->bUpdateModule)
			{
				// TypeDataModule은 보통 실행 로직이 없으므로 리스트에서 제거하기 위해 인덱스 저장
				TypeDataModuleIndex = i;
			}
		}
		else if (Module->IsA(UParticleModuleSpawnBase::StaticClass()))
		{
			UParticleModuleSpawnBase* SpawnBase = Cast<UParticleModuleSpawnBase>(Module);
			SpawningModules.Add(SpawnBase);
		}
	}

	if (TypeDataModuleIndex != -1)
	{
		Modules.RemoveAt(TypeDataModuleIndex);
	}

	if (TypeDataModule)
	{
		// @todo 스프라이트 기반 파티클 우선 구현 이후에 구현(e.g., 메시, 빔, 리본 등)
		// UParticleModuleTypeDataMesh* MeshTD = Cast<UParticleModuleTypeDataMesh>(TypeDataModule);
		// if (MeshTD
		// 	&& MeshTD->Mesh
		// 	&& MeshTD->Mesh->HasValidRenderData(false))
		// {
		// 	UParticleSpriteEmitter* SpriteEmitter = Cast<UParticleSpriteEmitter>(GetOuter());
		// 	if (SpriteEmitter && (MeshTD->bOverrideMaterial == false))
		// 	{
		// 		FStaticMeshSection& Section = MeshTD->Mesh->GetRenderData()->LODResources[0].Sections[0];
		// 		UMaterialInterface* Material = MeshTD->Mesh->GetMaterial(Section.MaterialIndex);
		// 		if (Material)
		// 		{
		// 			RequiredModule->Material = Material;
		// 		}
		// 	}
		// }
	}
}

int32 UParticleLODLevel::CalculateMaxActiveParticleCount()
{
	assert(RequiredModule != nullptr);

	float ParticleLifetime = 0.0f;
	float MaxSpawnRate = SpawnModule->GetEstimatedSpawnRate();

	for (int32 ModuleIndex = 0; ModuleIndex < Modules.Num(); ModuleIndex++)
	{
		UParticleModuleLifetimeBase* LifetimeMod = Cast<UParticleModuleLifetimeBase>(Modules[ModuleIndex]);
		if (LifetimeMod != nullptr)
		{
			ParticleLifetime += LifetimeMod->GetMaxLifetime();
		}

		UParticleModuleSpawnBase* SpawnMod = Cast<UParticleModuleSpawnBase>(Modules[ModuleIndex]);
		if (SpawnMod != nullptr)
		{
			MaxSpawnRate += SpawnMod->GetEstimatedSpawnRate();
		}
	}

	float MaxDuration = 0.0f;
	float TotalDuration = 0.0f;
	int32 TotalLoops = 0;
	if (RequiredModule != nullptr)
	{
		MaxDuration = FMath::Max(RequiredModule->EmitterDuration, RequiredModule->EmitterDurationLow);
		TotalLoops = RequiredModule->EmitterLoops;
		TotalDuration = MaxDuration * TotalLoops;
	}

	int32 MaxAPC = 0;

	if (TotalDuration != 0.0f)
	{
		if (TotalLoops == 1)
		{
			if (ParticleLifetime < MaxDuration)
			{
				MaxAPC += std::ceil(ParticleLifetime * MaxSpawnRate);
			}
			else
			{
				MaxAPC += std::ceil(MaxDuration * MaxSpawnRate);
			}
			MaxAPC += 1; // 부동 소수점 오차 보정
		}
		else
		{
			if (ParticleLifetime < MaxDuration)
			{
				MaxAPC += std::ceil(ParticleLifetime * MaxSpawnRate);
			}
			else
			{
				MaxAPC += std::ceil(std::ceil(MaxDuration * MaxSpawnRate) * ParticleLifetime);
			}
			MaxAPC += 1; // 부동 소수점 오차 보정
		}
	}
	else // 무한 루프 (TotalLoops == 0)
	{
		if (ParticleLifetime < MaxDuration)
		{
			MaxAPC += std::ceil(ParticleLifetime * std::ceil(MaxSpawnRate));
		}
		else
		{
			if (ParticleLifetime != 0.0f)
			{
				if (ParticleLifetime <= MaxDuration)
				{
					MaxAPC += std::ceil(MaxDuration * MaxSpawnRate);
				}
				else
				{
					MaxAPC += std::ceil(MaxDuration * MaxSpawnRate) * ParticleLifetime;
				}
			}
			else
			{
				MaxAPC += std::ceil(MaxSpawnRate);
			}
		}
		// [안전 구역 (Safety Zone)] - 33ms 프레임 지연 보정
		// 프레임 드랍이 생겨서 DeltaTime이 0.032초(약 30fps)만큼 튀었을 때,
		// 한 번에 몰려서 생성될 파티클 개수를 미리 확보해둔다.
		MaxAPC += FMath::Max<int32>(std::ceil(MaxSpawnRate * 0.032f), 2);
	}

	PeakActiveParticles = MaxAPC;

	return MaxAPC;
}
