#include "pch.h"
#include "ParticleEmitterInstances.h"

#include "ParticleEmitter.h"
#include "ParticleHelper.h"
#include "ParticleLODLevel.h"
#include "ParticleModule.h"
#include "ParticleModuleRequired.h"
#include "ParticleModuleSpawn.h"
#include "ParticleModuleTypeDataBase.h"
#include "ParticleSystemComponent.h"

/*-----------------------------------------------------------------------------
	FParticleEmitterInstance
-----------------------------------------------------------------------------*/
const float FParticleEmitterInstance::PeakActiveParticleUpdateDelta = 0.05f;

FParticleEmitterInstance::FParticleEmitterInstance(UParticleSystemComponent* InComponent)
	: SpriteTemplate(nullptr)
	, Component(InComponent)
	, CurrentLODLevel(nullptr)
	, CurrentLODLevelIndex(0)
	, ParticleData(nullptr)
	, ParticleIndices(nullptr)
	, InstanceData(nullptr)
	, InstancePayloadSize(0)
	, PayloadOffset(0)
	, ParticleSize(0)
	, ParticleStride(0)
	, ActiveParticles(0)
	, MaxActiveParticles(0)
	, SpawnFraction(0.0f)
	, SecondsSinceCreation(0.0f)
{
}

FParticleEmitterInstance::~FParticleEmitterInstance()
{
	if (ParticleData)
	{
		std::free(ParticleData);
		ParticleData = nullptr;
	}

	if (ParticleIndices)
	{
		std::free(ParticleIndices);
		ParticleIndices = nullptr;
	}

	if (InstanceData)
	{
		std::free(InstanceData);
		InstanceData = nullptr;
	}
}

void FParticleEmitterInstance::InitParameters(UParticleEmitter* InTemplate)
{
	SpriteTemplate = InTemplate;
	SetupEmitterDuration();
}

void FParticleEmitterInstance::Init()
{
	assert(SpriteTemplate != nullptr);

	// 데이터 초기화를 위해서 모든 모듈 타입을 포함하고 있는 가장 높은 LOD 레벨을 사용한다.
	UParticleLODLevel* HighLODLevel = SpriteTemplate->LODLevels[0];

	// 현재 머티리얼을 설정한다.
	assert(HighLODLevel->RequiredModule);
	CurrentMaterial = HighLODLevel->RequiredModule->Material;

	// non-zero 파티클 크기를 이미 가지고 있다면, 대부분의 할당 작업을 다시 할 필요 없음
	bool bNeedsInit = (ParticleSize == 0);

	if (bNeedsInit)
	{
		// 사전 계산된 값 복사
		ParticleSize = SpriteTemplate->ParticleSize;
		PayloadOffset = ParticleSize;

		if ((InstanceData == nullptr || (SpriteTemplate->ReqInstanceBytes > InstancePayloadSize)))
		{
			InstanceData = (uint8*)std::realloc(InstanceData, SpriteTemplate->ReqInstanceBytes);
			InstancePayloadSize = SpriteTemplate->ReqInstanceBytes;
		}

		std::memset(InstanceData, 0, InstancePayloadSize);

		for (UParticleModule* ParticleModule : SpriteTemplate->ModulesNeedingInstanceData)
		{
			assert(ParticleModule);
			uint8* PrepInstData = GetModuleInstanceData(ParticleModule);
			assert(PrepInstData != nullptr);
			ParticleModule->PrepPerInstanceBlock(this, (void*)PrepInstData);
		}

		/*
		for (UParticleModule* ParticleModule : SpriteTemplate->ModulesNeedingRandomSeedInstanceData)
		{

		}
		*/

		// 이미터 특화 페이로드 오프셋 (e.g., TrailComponent는 추가 바이트를 요구)
		PayloadOffset = ParticleSize;

		// 이미터 특화 요구사항을 위한 크기 업데이트
		ParticleSize += RequiredBytes();

		// SIMD(SSE) 연산을 위해 16 바이트 정렬
		// ParticleSize = Align(ParticleSize, 16);

		// E.g. trail emitters store trailing particles directly after leading one.
		ParticleStride = CalculateParticleStride(ParticleSize);
	}

	SetMeshMaterials(SpriteTemplate->MeshMaterials);

	// 초기값 설정
	SpawnFraction			= 0;
	SecondsSinceCreation	= 0;
	ParticleCounter			= 0;

	UpdateTransforms();
	Location				= Component->GetWorldLocation();
	OldLocation				= Location;

	if (ParticleData == NULL)
	{
		MaxActiveParticles	= 0;
		ActiveParticles		= 0;
	}

	// 합리적인 디폴트 값으로 초기화
	if (bNeedsInit)
	{
		if ((HighLODLevel->PeakActiveParticles > 0) || (SpriteTemplate->InitialAllocationCount > 0))
		{
			if (SpriteTemplate->InitialAllocationCount > 0)
			{
				Resize(FMath::Min(SpriteTemplate->InitialAllocationCount, 100));
			}
			else
			{
				Resize(FMath::Min(HighLODLevel->PeakActiveParticles, 100));
			}
		}
		else
		{
			Resize(10);
		}
	}
}

bool FParticleEmitterInstance::Resize(int32 NewMaxActiveParticles, bool bSetMaxActiveCount)
{
	if (NewMaxActiveParticles < 0)
	{
		return false;
	}

	ParticleData = (uint8*) std::realloc(ParticleData, ParticleStride * NewMaxActiveParticles);
	assert(ParticleData);

	if (ParticleIndices == nullptr)
	{
		MaxActiveParticles = 0;
	}
	ParticleIndices = (uint16*) std::realloc(ParticleIndices, sizeof(uint16) * (NewMaxActiveParticles + 1));

	for (int32 i = MaxActiveParticles; i < NewMaxActiveParticles; i++)
	{
		ParticleIndices[i] = i;
	}

	MaxActiveParticles = NewMaxActiveParticles;

	return true;
}

void FParticleEmitterInstance::Tick(float DeltaTime, bool bSuppressSpawning)
{
	assert(SpriteTemplate);
	assert(SpriteTemplate->LODLevels.Num() > 0);

	bool bFirstTime = (SecondsSinceCreation > 0.0f) ? false : true;

	UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();

	float EmitterDelay = Tick_EmitterTimeSetup(DeltaTime, LODLevel);

	if (bEnabled)
	{
		KillParticles();

		ResetParticleParameters(DeltaTime);

		CurrentMaterial = LODLevel->RequiredModule->Material;
		Tick_ModuleUpdate(DeltaTime, LODLevel);

		SpawnFraction = Tick_SpawnParticles(DeltaTime, LODLevel, bSuppressSpawning, bFirstTime);

		Tick_ModulePostUpdate(DeltaTime, LODLevel);

		if (ActiveParticles > 0)
		{
			// @todo
			// UpdateOrbitData(DeltaTime);
			// UpdateBoundingBox(DeltaTime);
		}

		Tick_ModuleFinalUpdate(DeltaTime, LODLevel);
	}
}

float FParticleEmitterInstance::Tick_EmitterTimeSetup(float DeltaTime, UParticleLODLevel* InCurrentLODLevel)
{
	OldLocation = Location;
	Location	= Component->GetWorldLocation();

	UpdateTransforms();
	SecondsSinceCreation += DeltaTime;

	bool bLooped = false;
	EmitterTime += DeltaTime;
	bLooped = (EmitterDuration > 0.0f) && (EmitterTime >= EmitterDuration);

	float EmitterDelay = CurrentDelay;

	if (bLooped)
	{
		LoopCount++;

		EmitterTime -= EmitterDuration;
	}

	return EmitterDelay;
}

float FParticleEmitterInstance::Tick_SpawnParticles(float DeltaTime, UParticleLODLevel* InCurrentLODLevel, bool bSuppressSpawning, bool bFirstTime)
{
	if (bSuppressSpawning || EmitterTime < 0.0f)
	{
		return 0.0f;
	}

	// EmitterLoops가 0일 경우(무한루프일 경우), 항상 스폰한다.
	if ((InCurrentLODLevel->RequiredModule->EmitterLoops == 0) ||
		(LoopCount < InCurrentLODLevel->RequiredModule->EmitterLoops) ||
		(SecondsSinceCreation < (EmitterDuration * InCurrentLODLevel->RequiredModule->EmitterLoops)) ||
		bFirstTime)
	{
		bFirstTime = false;
		SpawnFraction = Spawn(DeltaTime);
	}

	return SpawnFraction;
}

void FParticleEmitterInstance::Tick_ModuleUpdate(float DeltaTime, UParticleLODLevel* InCurrentLODLevel)
{
	UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels[0];
	assert(HighestLODLevel);
	for (int32 ModuleIndex = 0; ModuleIndex < InCurrentLODLevel->UpdateModules.Num(); ModuleIndex++)
	{
		UParticleModule* CurrentModule = InCurrentLODLevel->UpdateModules[ModuleIndex];
		if (CurrentModule && CurrentModule->bEnabled && CurrentModule->bUpdateModule)
		{
			CurrentModule->Update({*this, (int32)GetModuleDataOffset(HighestLODLevel->UpdateModules[ModuleIndex]), DeltaTime});
		}
	}
}

void FParticleEmitterInstance::Tick_ModulePostUpdate(float DeltaTime, UParticleLODLevel* InCurrentLODLevel)
{
	if (InCurrentLODLevel->TypeDataModule)
	{
		// @todo
		// InCurrentLODLevel->TypeDataModule->Update({*this, TypeDataOffset, DeltaTime});
	}
}

void FParticleEmitterInstance::Tick_ModuleFinalUpdate(float DeltaTime, UParticleLODLevel* InCurrentLODLevel)
{
	UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels[0];
	assert(HighestLODLevel);
	for (int32 ModuleIndex = 0; ModuleIndex < InCurrentLODLevel->UpdateModules.Num(); ModuleIndex++)
	{
		UParticleModule* CurrentModule = InCurrentLODLevel->UpdateModules[ModuleIndex];
		if (CurrentModule && CurrentModule->bEnabled && CurrentModule->bFinalUpdateModule)
		{
			CurrentModule->FinalUpdate({*this, (int32)GetModuleDataOffset(HighestLODLevel->UpdateModules[ModuleIndex]), DeltaTime});
		}
	}

	if (InCurrentLODLevel->TypeDataModule && InCurrentLODLevel->TypeDataModule->bEnabled && InCurrentLODLevel->TypeDataModule->bFinalUpdateModule)
	{
		InCurrentLODLevel->TypeDataModule->FinalUpdate({*this, (int32)GetModuleDataOffset(HighestLODLevel->TypeDataModule), DeltaTime});
	}
}

uint32 FParticleEmitterInstance::RequiredBytes()
{
	uint32 uiBytes = 0;
	bool bHasSubUV = false;
	for (int32 LODIndex = 0; (LODIndex < SpriteTemplate->LODLevels.Num()) && !bHasSubUV; LODIndex++)
	{
		UParticleLODLevel* LODLevel = SpriteTemplate->GetLODLevel(LODIndex);

		if (LODLevel)
		{
			if (LODIndex > 0)
			{
				/** @todo SubUV 좀 더 알아보기, 현재는 아무것도 하지 않음 */
			}
		}
	}
	return uiBytes;
}

uint32 FParticleEmitterInstance::GetModuleDataOffset(UParticleModule* Module)
{
	assert(SpriteTemplate);

	uint32* Offset = SpriteTemplate->ModuleOffsetMap.Find(Module);
	return (Offset != nullptr) ? *Offset : 0;
}

uint8* FParticleEmitterInstance::GetModuleInstanceData(UParticleModule* Module)
{
	if (InstanceData && SpriteTemplate)
	{
		uint32* Offset = SpriteTemplate->ModuleInstanceOffsetMap.Find(Module);
		if (Offset)
		{
			assert(*Offset < (uint32)InstancePayloadSize);
			return &(InstanceData[*Offset]);
		}
	}
	return nullptr;
}

uint32 FParticleEmitterInstance::CalculateParticleStride(uint32 ParticleSize)
{
	return ParticleSize;
}

void FParticleEmitterInstance::ResetParticleParameters(float DeltaTime)
{
	UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();
	UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels[0];
	assert(HighestLODLevel);

	// @todo Orbit

	for (int32 ParticleIndex = 0; ParticleIndex < ActiveParticles; ParticleIndex++)
	{
		DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[ParticleIndex]);
		Particle.Velocity = Particle.BaseVelocity;
		Particle.Size = Particle.BaseSize;
		Particle.RotationRate = Particle.BaseRotationRate;
	}
}

float FParticleEmitterInstance::Spawn(float DeltaTime)
{
	UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();

	float SpawnRate = 0.0f;
	int32 SpawnCount = 0;
	float OldLeftOver = SpawnFraction;

	UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels[0];

	bool bProcessSpawnRate = true;

	for (int32 SpawnModIndex = 0; SpawnModIndex < LODLevel->SpawningModules.Num(); SpawnModIndex++)
	{
		UParticleModuleSpawnBase* SpawnModule = LODLevel->SpawningModules[SpawnModIndex];
		if (SpawnModule && SpawnModule->bEnabled)
		{
			UParticleModule* OffsetModule = HighestLODLevel->SpawningModules[SpawnModIndex];
			uint32 Offset = GetModuleDataOffset(OffsetModule);

			int32 Number = 0;
			float Rate = 0.0f;
			if (SpawnModule->GetSpawnAmount({*this}, Offset, OldLeftOver, DeltaTime, Number, Rate) == false)
			{
				bProcessSpawnRate = false;
			}

			Number = FMath::Max<int32>(0, Number);
			Rate = FMath::Max<float>(0.0f, Rate);

			SpawnCount += Number;
			SpawnRate += Rate;
		}
	}

	if (bProcessSpawnRate)
	{
		// @todo SpawnRate 스케일
		// float RateScale = LODLevel->SpawnModule->RateScale.GetValue(EmitterTime, Component.GetDistributionRate()) * LODLevel->SpawnModule->GetGlobalRateScale();
		// SpawnRate += LODLevel->SpawnModule->Rate.GetValue(EmitterTime, Component.GetDistributionData()) * RateScale;
		// SpawnRate = FMath::Max<float>(0.0f, SpawnRate);
	}
	else
	{
		SpawnRate = 0.0f;
		SpawnCount = 0;
	}

	if ((SpawnRate > 0.f))
	{
		float SafetyLeftover	= OldLeftOver;
		float NewLeftover		= OldLeftOver + DeltaTime * SpawnRate;
		int32 Number			= std::floor(NewLeftover);
		float Increment			= (SpawnRate > 0.0f) ? (1.f / SpawnRate) : 0.0f;
		float StartTime			= DeltaTime + OldLeftOver * Increment - Increment;
		NewLeftover				= NewLeftover - Number;

		bool bProcessSpawn = true;
		int32 NewCount = ActiveParticles + Number;

		if (NewCount >= MaxActiveParticles)
		{
			if (DeltaTime < PeakActiveParticleUpdateDelta)
			{
				bProcessSpawn = Resize(NewCount + (int32)(FMath::Sqrt(FMath::Sqrt((float)NewCount)) + 1));
			}
			else
			{
				bProcessSpawn = Resize((NewCount + (int32)(FMath::Sqrt(FMath::Sqrt((float)NewCount)) + 1)), false);
			}
		}

		if (bProcessSpawn == true)
		{
			const FVector InitialLocation = EmitterToSimulation.TransformPosition(FVector::One());

			SpawnParticles( Number, StartTime, Increment, InitialLocation, FVector::Zero() );

			return NewLeftover;
		}
		return SafetyLeftover;
	}
	return SpawnFraction;
}

void FParticleEmitterInstance::SpawnParticles(int32 Count, float StartTime, float Increment, const FVector& InitialLocation, const FVector& InitialVelocity/*, struct FParticleEventInstancePayload* EventPayload*/)
{
	UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();

	assert(ActiveParticles <= MaxActiveParticles);

	assert(ActiveParticles + Count <= MaxActiveParticles);
	Count = FMath::Min<int32>(Count, MaxActiveParticles - ActiveParticles);

	UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels[0];
	float SpawnTime = StartTime;
	// @todo 보간 변수들 사용처 확인 (bLegacySpawnBehavior 플래그 확인)
	float Interp = 1.0f;
	const float InterpIncrement = (Count > 0 && Increment > 0.0f) ? (1.0f / (float)Count) : 0.0f;
	for (int32 i = 0; i < Count; i++)
	{
		assert(ParticleData && ParticleIndices);

		uint16 NextFreeIndex = ParticleIndices[ActiveParticles];
		assert(NextFreeIndex < MaxActiveParticles);


		FBaseParticle* Particle = (FBaseParticle*)(ParticleData + ParticleStride * NextFreeIndex);
		const uint32 CurrentParticleIndex = ActiveParticles++;

		PreSpawn(Particle, InitialLocation, InitialVelocity);
		for (int32 ModuleIndex = 0; ModuleIndex < LODLevel->SpawnModules.Num(); ModuleIndex++)
		{
			UParticleModule* SpawnModule = LODLevel->SpawnModules[ModuleIndex];
			if (SpawnModule->bEnabled)
			{
				UParticleModule* OffsetModule = HighestLODLevel->SpawnModules[ModuleIndex];
				SpawnModule->Spawn({*this, (int32)GetModuleDataOffset(OffsetModule), SpawnTime, Particle});
			}
		}
		PostSpawn(Particle, Interp, SpawnTime);

		// Spawn modules may set a relative time greater than 1.0f to indicate that a particle should not be spawned. We kill these particles.
		// if (Particle->RelativeTime > 1.0f)
		// {
		// 	KillParticle(CurrentParticleIndex);
		//
		// 	// Process next particle
		// 	continue;
		// }
	}
}

void FParticleEmitterInstance::PreSpawn(FBaseParticle* Particle, const FVector& InitialLocation, const FVector& InitialVelocity)
{
	assert(Particle);
	assert(ParticleSize > 0);

	std::memset(Particle, 0, ParticleSize);

	Particle->Location = InitialLocation;
	Particle->BaseVelocity = InitialVelocity;
	Particle->Velocity = InitialVelocity;
}

void FParticleEmitterInstance::PostSpawn(FBaseParticle* Particle, float InterpolationPercentage, float SpawnTime)
{
	UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();
	// @todo
	 // if (LODLevel->RequiredModule->bUseLocalSpace == false)
	 // {
		// if (FVector::DistSquared(OldLocation, Location) > 1.f)
		// {
		// 	Particle->Location += InterpolationPercentage * (OldLocation - Location);
		// }
	 // }
	Particle->OldLocation = Particle->Location;
	Particle->Location = FVector(Particle->Velocity) * SpawnTime;
}

void FParticleEmitterInstance::SetupEmitterDuration()
{
	if (SpriteTemplate == nullptr)
	{
		return;
	}

	int32 EDCount = EmitterDurations.Num();
	if ((EDCount == 0) || (EDCount != SpriteTemplate->LODLevels.Num()))
	{
		EmitterDurations.Empty();
		EmitterDurations.SetNum(SpriteTemplate->LODLevels.Num());
	}

	// 각 LOD 레벨에 대하여 지속시간 계산
	for (int32 LODIndex = 0; LODIndex < SpriteTemplate->LODLevels.Num(); LODIndex++)
	{
		UParticleLODLevel* TempLOD = SpriteTemplate->LODLevels[LODIndex];
		UParticleModuleRequired* RequiredModule = TempLOD->RequiredModule;

		EmitterDurations[TempLOD->Level] = RequiredModule->EmitterDuration + CurrentDelay;
	}

	EmitterDuration = EmitterDurations[CurrentLODLevelIndex];
}

void FParticleEmitterInstance::KillParticles()
{
	if (ActiveParticles > 0)
	{
		UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();

		bool bFoundCorruptIndices = false;
		for (int32 i = ActiveParticles - 1; i >= 0; i--)
		{
			const int32 CurrentIndex = ParticleIndices[i];
			if (CurrentIndex < MaxActiveParticles)
			{
				const uint8* ParticleBase = ParticleData + CurrentIndex * ParticleStride;
				FBaseParticle& Particle = *((FBaseParticle*)ParticleBase);

				if (Particle.RelativeTime > 1.0f)
				{
					ParticleIndices[i] = ParticleIndices[ActiveParticles - 1];
					ParticleIndices[ActiveParticles - 1] = CurrentIndex;
					ActiveParticles--;
				}
			}
			else
			{
				bFoundCorruptIndices = true;
			}
		}

		if (bFoundCorruptIndices)
		{
			// @todo Fixup 로직 필요
		}
	}
}

void FParticleEmitterInstance::KillParticle(int32 Index)
{
	if (Index < ActiveParticles)
	{
		UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();

		int32 KillIndex = ParticleIndices[Index];

		for (int32 i = Index; i < ActiveParticles - 1; i++)
		{
			ParticleIndices[i] = ParticleIndices[i + 1];
		}
		ParticleIndices[ActiveParticles - 1] = KillIndex;
		ActiveParticles--;
	}
}

void FParticleEmitterInstance::UpdateTransforms()
{
}

UParticleLODLevel* FParticleEmitterInstance::GetCurrentLODLevelChecked()
{
	assert(SpriteTemplate != nullptr);
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	assert(LODLevel != nullptr);
	assert(LODLevel->RequiredModule != nullptr);
	return LODLevel;
}

bool FParticleEmitterInstance::FillReplayData(FDynamicEmitterReplayDataBase& OutData)
{
	if (!SpriteTemplate)
	{
		return false;
	}

	if (ActiveParticles <= 0 || !bEnabled)
	{
		return false;
	}

	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	if ((LODLevel == nullptr) || (LODLevel->bEnabled == false))
	{
		return false;
	}

	assert(MaxActiveParticles >= ActiveParticles);

	OutData.eEmitterType = DET_Unknown;
	OutData.ActiveParticleCount = ActiveParticles;
	OutData.ParticleStride = ParticleStride;
	OutData.Scale = Component->GetRelativeScale();

	int32 ParticleMemSize = MaxActiveParticles * ParticleStride;

	OutData.DataContainer.Alloc(ParticleMemSize, MaxActiveParticles);

	std::memcpy(OutData.DataContainer.ParticleData, ParticleData, ParticleMemSize);
	std::memcpy(OutData.DataContainer.ParticleIndices, ParticleIndices, MaxActiveParticles * sizeof(uint16));

	{
		FDynamicSpriteEmitterReplayDataBase* NewReplayData =
			static_cast<FDynamicSpriteEmitterReplayDataBase*>(&OutData);

		NewReplayData->RequiredModule = LODLevel->RequiredModule;
		NewReplayData->ScreenAlignment = LODLevel->RequiredModule->ScreenAlignment;
		NewReplayData->bUseLocalSpace = LODLevel->RequiredModule->bUseLocalSpace;
	}

	return true;
}

/*-----------------------------------------------------------------------------
	ParticleSpriteEmitterInstance
-----------------------------------------------------------------------------*/

FParticleSpriteEmitterInstance::FParticleSpriteEmitterInstance(UParticleSystemComponent* InComponent)
	: FParticleEmitterInstance(InComponent)
{
	// @todo
}

FDynamicEmitterDataBase* FParticleSpriteEmitterInstance::GetDynamicData(bool bSelected)
{
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	if (!bEnabled)
	{
		return nullptr;
	}

	FDynamicSpriteEmitterData* NewEmitterData = new FDynamicSpriteEmitterData(LODLevel->RequiredModule);

	if (!FillReplayData(NewEmitterData->Source))
	{
		delete NewEmitterData;
		return nullptr;
	}

	NewEmitterData->Init(bSelected);

	return NewEmitterData;
}

FDynamicEmitterReplayDataBase* FParticleSpriteEmitterInstance::GetReplayData()
{
	if (ActiveParticles <= 0 || !bEnabled)
	{
		return nullptr;
	}

	FDynamicEmitterReplayDataBase* NewEmitterReplayData = new FDynamicSpriteEmitterReplayDataBase();
	assert(NewEmitterReplayData != nullptr);

	if (!FillReplayData(*NewEmitterReplayData))
	{
		delete NewEmitterReplayData;
		return nullptr;
	}

	return NewEmitterReplayData;
}

bool FParticleSpriteEmitterInstance::FillReplayData(FDynamicEmitterReplayDataBase& OutData)
{
	if (ActiveParticles <= 0)
	{
		return false;
	}

	if (!FParticleEmitterInstance::FillReplayData(OutData))
	{
		return false;
	}

	OutData.eEmitterType = DET_Sprite;

	FDynamicSpriteEmitterReplayDataBase* NewReplayData = static_cast<FDynamicSpriteEmitterReplayDataBase*>(&OutData);

	NewReplayData->MaterialInterface = GetCurrentMaterial();

	return true;
}

UMaterialInterface* FParticleSpriteEmitterInstance::GetCurrentMaterial()
{
	return CurrentMaterial;
}
