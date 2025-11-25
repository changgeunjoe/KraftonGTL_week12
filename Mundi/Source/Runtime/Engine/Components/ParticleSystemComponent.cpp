#include "pch.h"
#include "ParticleSystemComponent.h"

#include "ParticleModuleLifetime.h"
#include "ParticleModuleVelocity.h"
#include "Source/Runtime/Engine/Particle/ParticleEmitter.h"
#include "Source/Runtime/Engine/Particle/ParticleEmitterInstances.h"
#include "Source/Runtime/Engine/Particle/ParticleHelper.h"
#include "Source/Runtime/Engine/Particle/ParticleLODLevel.h"
#include "Source/Runtime/Engine/Particle/ParticleSystem.h"
#include "Source/Runtime/Engine/Particle/ParticleSpriteEmitter.h"
#include "Source/Runtime/Engine/Particle/ParticleModuleTypeDataMesh.h"
#include "SceneView.h"

class UParticleModuleVelocity;

UParticleSystemComponent::UParticleSystemComponent()
	: Template(nullptr)
	, bSuppressSpawning(false)
	, bWasDeactivated(false)
	, bWasCompleted(true)
	, LODLevel(0)
	, TotalActiveParticles(0)
{
	bCanEverTick = true;	// 에디터에서 tick 돌리기 위한

	// NOTE: UParticleSystem 에셋 하드코딩 (추후 에셋으로 할당 필요)
	Template = NewObject<UParticleSystem>();
	auto SpriteEmitter = Template->AddEmitter(UParticleSpriteEmitter::StaticClass());
	SpriteEmitter->LODLevels[0]->AddModule(UParticleModuleVelocity::StaticClass());
	SpriteEmitter->LODLevels[0]->AddModule(UParticleModuleLifetime::StaticClass());
	// SpriteEmitter->LODLevels[0]->AddModule(UParticleModuleTypeDataMesh::StaticClass());

	//InitParticles();	// tick에서 호출해줌
}

UParticleSystemComponent::~UParticleSystemComponent()
{
	ResetParticles(true);
	ClearDynamicData();

	// Template은 ResourceManager가 관리하므로 여기서 삭제하지 않음
	Template = nullptr;
}

void UParticleSystemComponent::InitParticles()
{
	if (Template == nullptr)
	{
		return;
	}

	if (EmitterInstances.Num() > 0)
	{
		ResetParticles(true);
	}

	const int32 NumEmitters = Template->Emitters.Num();

	for (int32 Idx = 0; Idx < NumEmitters; Idx++)
	{
		UParticleEmitter* Emitter = Template->Emitters[Idx];
		if (Emitter)
		{
			FParticleEmitterInstance* Instance = Emitter->CreateInstance(this);
			if (Instance)
			{
				EmitterInstances.Add(Instance);

				Instance->InitParameters(Emitter);
				Instance->Init();
				Instance->CurrentLODLevelIndex = LODLevel;

				if (Emitter->LODLevels.Num() > 0)
				{
					Instance->CurrentLODLevel = Emitter->LODLevels[0];
				}
			}
		}

		bWasCompleted = false;
		bWasDeactivated = false;
		bSuppressSpawning = false;
	}
}

void UParticleSystemComponent::ResetParticles(bool bEmptyInstances)
{
	for (int32 i = 0; i < EmitterInstances.Num(); i++)
	{
		if (EmitterInstances[i])
		{
			delete EmitterInstances[i];
			EmitterInstances[i] = nullptr;
		}
	}

	if (bEmptyInstances)
	{
		EmitterInstances.Empty();
	}

	TotalActiveParticles = 0;
	bWasCompleted = true;
}

void UParticleSystemComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);

	if (bWasCompleted && bWasDeactivated)
	{
		return;
	}

	if (Template && EmitterInstances.Num() == 0 && !bWasDeactivated)
	{
		InitParticles();
	}

	TotalActiveParticles = 0;

	for (int32 EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); EmitterIndex++)
	{
		FParticleEmitterInstance* Instance = EmitterInstances[EmitterIndex];

		if (Instance && Instance->SpriteTemplate)
		{
			assert(Instance->SpriteTemplate->LODLevels.Num() > 0);

			UParticleLODLevel* SpriteLODLevel = Instance->SpriteTemplate->GetCurrentLODLevel(Instance);
			if (SpriteLODLevel && SpriteLODLevel->bEnabled)
			{
				Instance->Tick(DeltaTime, bSuppressSpawning);

				TotalActiveParticles += Instance->ActiveParticles;
			}
		}
	}

	if (bWasDeactivated && TotalActiveParticles == 0)
	{
		bWasCompleted = true;
	}

	UpdateDynamicData();
}

void UParticleSystemComponent::CollectMeshBatches(TArray<FMeshBatchElement>& OutMeshBatchElements, const FSceneView* View)
{
	for (FDynamicEmitterDataBase* EmitterData : EmitterRenderData)
	{
		if (!EmitterData)
		{
			continue;
		}

		// 렌더링 배치를 수집 (Collect)
		EmitterData->GetDynamicMeshElementsEmitter(OutMeshBatchElements, View);
	}
}


void UParticleSystemComponent::ClearDynamicData()
{
	EmitterRenderData.Empty();
}

void UParticleSystemComponent::UpdateDynamicData()
{
	ClearDynamicData();

	for (FParticleEmitterInstance* Instance : EmitterInstances)
	{
		if (Instance)
		{
			FDynamicEmitterDataBase* NewData = Instance->GetDynamicData(false);
			if (NewData)
			{
				EmitterRenderData.Add(NewData);
			}
		}
	}
}

void UParticleSystemComponent::Activate(bool bReset)
{
	if (Template == nullptr)
	{
		return;
	}

	if (bReset)
	{
		ResetParticles(bReset);
	}

	bWasDeactivated = false;
	bSuppressSpawning = false;
	bWasCompleted = false;

	Template->UpdateAllModuleLists();

	if (EmitterInstances.Num() == 0)
	{
		InitParticles();
	}
}

void UParticleSystemComponent::Deactivate()
{
	bSuppressSpawning = true;
	bWasDeactivated = true;
}

void UParticleSystemComponent::SetTemplate(UParticleSystem* NewTemplate, bool bAutoActivate)
{
	// 기존 템플릿과 동일하면 아무것도 하지 않음
	if (Template == NewTemplate)
	{
		return;
	}

	// 기존 파티클 인스턴스 정리
	ResetParticles(true);
	ClearDynamicData();

	// 새로운 템플릿 설정
	Template = NewTemplate;

	// 템플릿이 유효하고 자동 활성화가 켜져있으면 활성화
	if (Template && bAutoActivate)
	{
		Activate(true);
	}
}

int32 UParticleSystemComponent::GetTotalActiveParticles() const
{
	int32 TotalCount = 0;
	for (int32 i = 0; i < EmitterInstances.Num(); i++)
	{
		TotalCount += GetActiveParticleCount(i);
	}
	return TotalCount;
}

int32 UParticleSystemComponent::GetActiveParticleCount(int32 EmitterIndex) const
{
	if ((0 <= EmitterIndex && EmitterIndex < EmitterInstances.Num()) && EmitterInstances[EmitterIndex])
	{
		return EmitterInstances[EmitterIndex]->ActiveParticles;
	}
	return 0;
}
