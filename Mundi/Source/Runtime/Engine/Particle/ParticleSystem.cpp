#include "pch.h"

#include "ParticleSystem.h"
#include "ParticleEmitter.h"
#include "ParticleLODLevel.h"
#include "ParticleModuleTypeDataBase.h"
#include "ParticleSpriteEmitter.h"
#include "Source/Runtime/Core/Misc/JsonSerializer.h"

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

void UParticleSystem::Serialize(const bool bInIsLoading, JSON& InOutHandle)
{
	if (bInIsLoading)
	{
		// 기존 이미터 제거
		for (UParticleEmitter* Emitter : Emitters)
		{
			if (Emitter)
				DeleteObject(Emitter);
		}
		Emitters.Empty();

		// 이미터 배열 로드
		JSON EmittersJson;
		if (FJsonSerializer::ReadArray(InOutHandle, "Emitters", EmittersJson))
		{
			for (uint32 i = 0; i < EmittersJson.size(); ++i)
			{
				JSON EmitterJson = EmittersJson.at(i);

				// 이미터 생성
				UParticleEmitter* NewEmitter = NewObject<UParticleSpriteEmitter>();
				if (NewEmitter)
				{
					NewEmitter->Serialize(bInIsLoading, EmitterJson);
					Emitters.Add(NewEmitter);
				}
			}
		}

		// 시스템 업데이트
		UpdateAllModuleLists();
		CalculateMaxActiveParticleCounts();
		OnParticleChanged.Broadcast();
	}
	else
	{
		// 파티클 시스템 타입 저장
		InOutHandle["Type"] = "UParticleSystem";

		// 이미터 배열 저장
		JSON EmittersJson = JSON::Make(JSON::Class::Array);
		for (UParticleEmitter* Emitter : Emitters)
		{
			if (!Emitter)
				continue;

			JSON EmitterJson;
			Emitter->Serialize(bInIsLoading, EmitterJson);
			EmittersJson.append(EmitterJson);
		}
		InOutHandle["Emitters"] = EmittersJson;
	}
}

bool UParticleSystem::LoadFromFile(const FWideString& FilePath)
{
	JSON ParticleSystemJson;

	if (!FJsonSerializer::LoadJsonFromFile(ParticleSystemJson, FilePath))
	{
		UE_LOG("Failed to load particle system from file: %s", WideToUTF8(FilePath).c_str());
		return false;
	}

	Serialize(true, ParticleSystemJson);
	UE_LOG("Loaded particle system from: %s", WideToUTF8(FilePath).c_str());
	return true;
}

bool UParticleSystem::SaveToFile(const FWideString& FilePath)
{
	JSON ParticleSystemJson;
	Serialize(false, ParticleSystemJson);

	if (!FJsonSerializer::SaveJsonToFile(ParticleSystemJson, FilePath))
	{
		UE_LOG("Failed to save particle system to file: %s", WideToUTF8(FilePath).c_str());
		return false;
	}

	FString FinalPathStr = ResolveAssetRelativePath(WideToUTF8(FilePath), "");
	UParticleSystem* ParticleSystem = UResourceManager::GetInstance().Get<UParticleSystem>(FinalPathStr);
	if (ParticleSystem)
	{
		ParticleSystem->Serialize(true, ParticleSystemJson);
	}
	else
	{
		UResourceManager::GetInstance().Load<UParticleSystem>(FinalPathStr);
	}

	UE_LOG("Saved particle system to: %s", WideToUTF8(FilePath).c_str());
	return true;
}

void UParticleSystem::Load(const FString& FilePath, ID3D11Device* Device)
{
	// FString을 FWideString으로 변환
	FWideString WideFilePath = UTF8ToWide(FilePath);
	LoadFromFile(WideFilePath);
}
