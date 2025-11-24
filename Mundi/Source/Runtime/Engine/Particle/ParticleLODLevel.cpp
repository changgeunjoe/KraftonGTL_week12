#include "pch.h"

#include "ParticleLODLevel.h"
#include "ParticleModule.h"
#include "ParticleModuleLifetime.h"
#include "ParticleModuleRequired.h"
#include "ParticleModuleSpawn.h"
#include "ParticleModuleTypeDataBase.h"
#include "Source/Runtime/Core/Misc/JsonSerializer.h"

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

UParticleLODLevel::~UParticleLODLevel()
{
	for (UParticleModule* Module : Modules)
	{
		if (Module)
		{
			DeleteObject(Module);
			Module = nullptr;
		}
	}
	Modules.Empty();

	if (SpawnModule)
	{
		DeleteObject(SpawnModule);
		SpawnModule = nullptr;
	}

	if (TypeDataModule)
	{
		DeleteObject(TypeDataModule);
		TypeDataModule = nullptr;
	}

	if (RequiredModule)
	{
		DeleteObject(RequiredModule);
		RequiredModule = nullptr;
	}

	// 다른 배열들 정리 (포인터만 제거, 객체는 이미 위에서 삭제됨)
	SpawningModules.Empty();
	SpawnModules.Empty();
	UpdateModules.Empty();
}

UParticleModuleSpawn* UParticleLODLevel::AddSpawnModule()
{
	if (SpawnModule)
	{
		DeleteObject(SpawnModule);
		SpawnModule = nullptr;
	}

	UParticleModuleSpawn* NewSpawnModule = NewObject<UParticleModuleSpawn>();
	if (NewSpawnModule)
	{
		NewSpawnModule->OwnerEmitter = OwnerEmitter;
		NewSpawnModule->SetToSensibleDefaults(OwnerEmitter);
		SpawnModule = NewSpawnModule;

		UpdateModuleLists();
	}

	return SpawnModule;
}

UParticleModuleRequired* UParticleLODLevel::AddRequiredModule()
{
	if (RequiredModule)
	{
		DeleteObject(RequiredModule);
		RequiredModule = nullptr;
	}

	UParticleModuleRequired* NewModule = NewObject<UParticleModuleRequired>();
	if (NewModule)
	{
		NewModule->OwnerEmitter = OwnerEmitter;
		NewModule->SetToSensibleDefaults(OwnerEmitter);
		RequiredModule = NewModule;

		UpdateModuleLists();
	}

	return NewModule;
}

UParticleModule* UParticleLODLevel::AddModule(UClass* ModuleClass)
{
	if (!ModuleClass || !ModuleClass->IsChildOf(UParticleModule::StaticClass()))
	{
		return nullptr;
	}

	UParticleModule* NewModule = NewObject<UParticleModule>();
	if (NewModule)
	{
		NewModule->OwnerEmitter = OwnerEmitter;
		NewModule->SetToSensibleDefaults(OwnerEmitter);

		if (NewModule->IsA(UParticleModuleTypeDataBase::StaticClass()))
		{
			if (TypeDataModule)
			{
				DeleteObject(TypeDataModule);
			}
			TypeDataModule = Cast<UParticleModuleTypeDataBase>(NewModule);
		}
		else
		{
			Modules.Add(NewModule);
		}

		UpdateModuleLists();
	}

	return NewModule;
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

void UParticleLODLevel::Serialize(const bool bInIsLoading, JSON& InOutHandle)
{
	if (bInIsLoading)
	{
		// 기존 모듈 제거
		for (UParticleModule* Module : Modules)
		{
			if (Module)
				DeleteObject(Module);
		}
		Modules.Empty();

		if (RequiredModule)
		{
			DeleteObject(RequiredModule);
			RequiredModule = nullptr;
		}

		if (SpawnModule)
		{
			DeleteObject(SpawnModule);
			SpawnModule = nullptr;
		}

		if (TypeDataModule)
		{
			DeleteObject(TypeDataModule);
			TypeDataModule = nullptr;
		}

		// 다른 배열들 정리 (포인터만 제거)
		SpawningModules.Empty();
		SpawnModules.Empty();
		UpdateModules.Empty();

		// LOD 레벨 속성 로드
		FJsonSerializer::ReadInt32(InOutHandle, "Level", Level);

		// bitfield는 참조를 취할 수 없으므로 임시 변수 사용
		bool bEnabledTemp = false;
		FJsonSerializer::ReadBool(InOutHandle, "bEnabled", bEnabledTemp);
		bEnabled = bEnabledTemp ? 1 : 0;

		// 모듈 배열 로드
		JSON ModulesJson;
		if (FJsonSerializer::ReadArray(InOutHandle, "Modules", ModulesJson))
		{
			for (uint32 i = 0; i < ModulesJson.size(); ++i)
			{
				JSON ModuleJson = ModulesJson.at(i);

				// 모듈 타입 읽기
				FString TypeString;
				FJsonSerializer::ReadString(ModuleJson, "Type", TypeString);

				// 클래스 찾기
				UClass* ModuleClass = UClass::FindClass(TypeString);
				if (!ModuleClass)
					continue;

				// 모듈 생성
				UParticleModule* NewModule = (UParticleModule*)ObjectFactory::NewObject(ModuleClass);
				if (!NewModule)
					continue;

				// 모듈 데이터 역직렬화
				NewModule->Serialize(bInIsLoading, ModuleJson);

				// 특수 모듈 처리
				if (UParticleModuleRequired* RequiredMod = Cast<UParticleModuleRequired>(NewModule))
				{
					RequiredModule = RequiredMod;
					// Required 모듈도 Modules 배열에 추가 (UpdateModuleLists에서 필요)
					Modules.Add(NewModule);
				}
				else if (UParticleModuleSpawn* SpawnMod = Cast<UParticleModuleSpawn>(NewModule))
				{
					SpawnModule = SpawnMod;
					// Spawn 모듈도 Modules 배열에 추가 (UpdateModuleLists에서 필요)
					Modules.Add(NewModule);
				}
				else if (UParticleModuleTypeDataBase* TypeDataMod = Cast<UParticleModuleTypeDataBase>(NewModule))
				{
					TypeDataModule = TypeDataMod;
					// TypeData 모듈도 Modules 배열에 추가 (UpdateModuleLists에서 필요)
					Modules.Add(NewModule);
				}
				else
				{
					// 일반 모듈은 Modules 배열에 추가
					Modules.Add(NewModule);
				}
			}
		}
	}
	else
	{
		// LOD 레벨 속성 저장
		InOutHandle["Level"] = Level;
		InOutHandle["bEnabled"] = (bEnabled != 0);

		// 모듈 배열 저장
		JSON ModulesJson = JSON::Make(JSON::Class::Array);

		// Required 모듈 먼저 저장
		if (RequiredModule)
		{
			JSON ModuleJson;
			RequiredModule->Serialize(bInIsLoading, ModuleJson);
			ModulesJson.append(ModuleJson);
		}

		// Spawn 모듈 저장
		if (SpawnModule)
		{
			JSON ModuleJson;
			SpawnModule->Serialize(bInIsLoading, ModuleJson);
			ModulesJson.append(ModuleJson);
		}

		// TypeData 모듈 저장
		if (TypeDataModule)
		{
			JSON ModuleJson;
			TypeDataModule->Serialize(bInIsLoading, ModuleJson);
			ModulesJson.append(ModuleJson);
		}

		// 일반 모듈들 저장
		for (UParticleModule* Module : Modules)
		{
			if (!Module)
				continue;

			JSON ModuleJson;
			Module->Serialize(bInIsLoading, ModuleJson);
			ModulesJson.append(ModuleJson);
		}

		InOutHandle["Modules"] = ModulesJson;
	}
}
