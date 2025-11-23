#include "pch.h"
#include "ParticleSystemComponent.h"

#include "Source/Runtime/Engine/Particle/ParticleEmitter.h"
#include "Source/Runtime/Engine/Particle/ParticleEmitterInstance.h"
#include "Source/Runtime/Engine/Particle/ParticleLODLevel.h"
#include "Source/Runtime/Engine/Particle/ParticleSystem.h"

void UParticleSystemComponent::InitParticles()
{
	if (Template != nullptr)
	{
		int32 NumInstances = EmitterInstances.Num();
		int32 NumEmitters = Template->Emitters.Num();
		const bool bIsFirstCreate = NumInstances == 0;
		/** @note 언리얼엔진에서는 SetNumZeroed를 활용한다. */
		EmitterInstances.SetNum(NumEmitters, nullptr);

		bWasComplete = bIsFirstCreate ? false : bWasComplete;

		int32 PreferredLODLevel = LODLevel;
		bool bSetLodLevels = LODLevel > 0; // 요청된 LOD가 0이 아닐 경우, 모든 이미터를 생성할 때 LOD 레벨을 설정해야 함

		for (int32 Idx = 0; Idx < NumEmitters; Idx++)
		{
			UParticleEmitter* Emitter = Template->Emitters[Idx];
			if (Emitter)
			{
				/** @todo NumInstances == 0의 이유 확인 */
				FParticleEmitterInstance* Instance = NumInstances == 0 ? nullptr : EmitterInstances[Idx];

				if (Instance)
				{
					Instance->SetHaltSpawning(false);
					Instance->SetHaltSpawningExternal(false);
				}
				else
				{
					Instance = Emitter->CreateInstance(this);
					EmitterInstances[Idx] = Instance;
				}

				if (Instance)
				{
					Instance->bEnabled = true;
					Instance->InitParameters(Emitter);
					Instance->Init();

					PreferredLODLevel = FMath::Min(PreferredLODLevel, Emitter->LODLevels.Num());
					bSetLodLevels |= !bIsFirstCreate; // LOD 레벨은 인스턴스를 초기화하고, 처음 생성 시점이 아닐때만 설정됨
				}
			}
		}

		if (bSetLodLevels)
		{
			if (PreferredLODLevel != LODLevel)
			{
				assert(PreferredLODLevel < LODLevel);
				LODLevel = PreferredLODLevel;
			}

			for (int32 Idx = 0; Idx < EmitterInstances.Num(); Idx++)
			{
				FParticleEmitterInstance* Instance = EmitterInstances[Idx];
				// 여기서 LOD 레벨을 설정함
				if (Instance)
				{
					Instance->CurrentLODLevelIndex = LODLevel;

					if (Instance->CurrentLODLevelIndex >= Instance->SpriteTemplate->LODLevels.Num())
					{
						Instance->CurrentLODLevelIndex = Instance->SpriteTemplate->LODLevels.Num() - 1;
					}
					Instance->CurrentLODLevel = Instance->SpriteTemplate->LODLevels[Instance->CurrentLODLevelIndex];
				}
			}
		}
	}
}

void UParticleSystemComponent::TickComponent(float DeltaTime)
{
	UFXSystemComponent::TickComponent(DeltaTime);

	TotalActiveParticles = 0;

	int32 EmitterIndex;
	for (EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); EmitterIndex++)
	{
		FParticleEmitterInstance* Instance = EmitterInstances[EmitterIndex];

		if (Instance && Instance->SpriteTemplate)
		{
			assert(Instance->SpriteTemplate->LODLevels.Num() > 0);

			UParticleLODLevel* SpriteLODLevel = Instance->SpriteTemplate->GetCurrentLODLevel(Instance);
			if (SpriteLODLevel && SPriteLODLevel->bEnabled)
			{
				Instance->Tick(DeltaTime/**, bSuppressSpawning*/);

				Instance->Tick_MaterialOverrides(EmitterIndex);
				TotalActiveParticles += Instance->ActiveParticles;
			}
		}
	}
}

void UParticleSystemComponent::ClearDynamicData()
{
}

void UParticleSystemComponent::UpdateDynamicData()
{
	// GPU에 데이터 전달
}
