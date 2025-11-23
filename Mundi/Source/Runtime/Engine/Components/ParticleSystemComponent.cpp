#include "pch.h"
#include "ParticleSystemComponent.h"

#include "Source/Runtime/Engine/Particle/ParticleEmitter.h"
#include "Source/Runtime/Engine/Particle/ParticleEmitterInstances.h"
#include "Source/Runtime/Engine/Particle/ParticleHelper.h"
#include "Source/Runtime/Engine/Particle/ParticleLODLevel.h"
#include "Source/Runtime/Engine/Particle/ParticleSystem.h"

IMPLEMENT_CLASS(UFXSystemComponent, UPrimitiveComponent)

IMPLEMENT_CLASS(UParticleSystemComponent, UFXSystemComponent)

UParticleSystemComponent::UParticleSystemComponent()
	: Template(nullptr)
	, bSuppressSpawning(false)
	, bWasDeactivated(false)
	, bWasCompleted(true)
	, LODLevel(0)
	, TotalActiveParticles(0)
{
}

UParticleSystemComponent::~UParticleSystemComponent()
{
	ResetParticles(true);
	ClearDynamicData();
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
	EmitterInstances.SetNum(NumEmitters);

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

void UParticleSystemComponent::ClearDynamicData()
{
	for (auto* Data : EmitterRenderData)
	{
		delete Data;
	}
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
