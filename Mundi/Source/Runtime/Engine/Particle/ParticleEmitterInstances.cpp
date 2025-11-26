#include "pch.h"
#include "ParticleEmitterInstances.h"

#include "ParticleEmitter.h"
#include "ParticleHelper.h"
#include "ParticleLODLevel.h"
#include "ParticleModule.h"
#include "ParticleModuleRequired.h"
#include "ParticleModuleSpawn.h"
#include "ParticleModuleTypeDataBase.h"
#include "ParticleModuleTypeDataMesh.h"
#include "ParticleModuleTypeDataBeam.h"
#include "ParticleSystemComponent.h"
#include "ParticleModuleSubUV.h"

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
	, EmitterTime(0.0f)
	, bEnabled(1)
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

	// SubUV 모듈 포인터 캐싱 로직
	this->SubUVModule = nullptr;

	// HighLODLevel에 포함된 모듈 목록을 순회하며 SubUV 모듈을 찾습니다.
	if (HighLODLevel->Modules.Num() > 0)
	{
		for (UParticleModule* Module : HighLODLevel->Modules)
		{
			if (Module && Module->IsA(UParticleModuleSubUV::StaticClass()))
			{
				this->SubUVModule = Cast<UParticleModuleSubUV>(Module);
				break; // 찾았으면 순회 중단
			}
		}
	}

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
		ParticleSize = ((ParticleSize + 15) & (~15));

		// E.g. trail emitters store trailing particles directly after leading one.
		ParticleStride = CalculateParticleStride(ParticleSize);
	}

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
	assert(ParticleIndices);

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
			UpdateBoundingBox(DeltaTime);
		}

		Tick_ModuleFinalUpdate(DeltaTime, LODLevel);
	}
}

void FParticleEmitterInstance::UpdateBoundingBox(float DeltaTime)
{
	if (ActiveParticles <= 0)
	{
		return;
	}

	UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();

	if (!Component || !SpriteTemplate)
	{
		return;
	}

	const bool bUseLocalSpace = LODLevel->RequiredModule->bUseLocalSpace;
	const FMatrix ComponentToWorld = bUseLocalSpace
		? Component->GetWorldTransform().ToMatrix()
		: FMatrix::Identity();

	FVector NewLocation;
	float NewRotation;

	FVector MinVal(FLT_MAX);
	FVector MaxVal(-FLT_MAX);

	FVector Scale = Component->GetRelativeScale();

	for (int32 i = 0; i < ActiveParticles; i++)
	{
		DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);

		Particle.OldLocation = Particle.Location;

		NewLocation = Particle.Location + (Particle.Velocity * DeltaTime);
		NewRotation = Particle.Rotation + (Particle.RotationRate * DeltaTime);

		Particle.Location = NewLocation;
		Particle.Rotation = NewRotation;

		FVector WorldPosition = NewLocation;
		if (bUseLocalSpace)
		{
			WorldPosition = ComponentToWorld.TransformPosition(WorldPosition);
		}

		// @todo 바운딩 박스 업데이트
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
		Particle.RelativeTime += Particle.OneOverMaxLifetime * DeltaTime;
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
		float RateScale = LODLevel->SpawnModule->RateScale;

		SpawnRate += LODLevel->SpawnModule->GetEstimatedSpawnRate() * RateScale;

		SpawnRate = FMath::Max<float>(0.0f, SpawnRate);
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
			const FVector InitialLocation = EmitterToSimulation.TransformPosition(FVector::Zero());

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

	/** Interp: 위치 보간 비율
	 *         1.0 = OldLocation (프레임 시작 시점의 위치)
	 *         0.0 = Location (현재 프레임 시점의 위치)
	 *         파티클들은 과거(1.0)에서 현재(0.0) 사이의 경로상에 배치되어야 한다.
	 */
	float Interp = 1.0f;

	/** InterpIncrement: 파티클 하나당 줄어들 보간 비율 (1.0 / 파티클 개수) */
	const float InterpIncrement = (Count > 0 && Increment > 0.0f) ? (1.0f / (float)Count) : 0.0f;

	for (int32 i = 0; i < Count; i++)
	{
		assert(ParticleData && ParticleIndices);

		uint16 NextFreeIndex = ParticleIndices[ActiveParticles];
		assert(NextFreeIndex < MaxActiveParticles);

		DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * NextFreeIndex);
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

		/**
		 * 루프가 진행될수록 다음 파티클은 '더 늦게(최근에)' 태어난 파티클이다.
		 * 따라서 나이(SpawnTime)는 줄어들어야 하고,
		 * 생성 위치는 과거(OldLocation)에서 현재(Location) 쪽으로 이동해야 한다.
		 */
		SpawnTime -= Increment;
		Interp -= InterpIncrement;

		if (Particle->RelativeTime > 1.0f)
		{
			KillParticle(CurrentParticleIndex);

			continue;
		}
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
	Particle->Location += FVector(Particle->Velocity) * SpawnTime;
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
	assert(SpriteTemplate != nullptr);

	UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();
	FMatrix ComponentToWorld = Component->GetWorldTransform().ToMatrix();
	// @todo 현재는 RequiredModule에 EmitterOrigin, EmitterRotation과 같은 오프셋 정보가 없음
	FMatrix EmitterToComponent = FMatrix::Identity();

	if (LODLevel->RequiredModule->bUseLocalSpace)
	{
		EmitterToSimulation = EmitterToComponent;
		SimulationToWorld = ComponentToWorld;
	}
	else
	{
		/**
		 * 시뮬레이션 자체가 월드 좌표계에서 돌아간다.
		 * 파티클 위치값 (100, 200, 300)은 월드 좌표 (100, 200, 300)이다.
		 * 따라서 이미터의 로컬 좌표를 월드로 미리 다 변환해버린다.
		 */
		EmitterToSimulation = EmitterToComponent * ComponentToWorld;
		SimulationToWorld = FMatrix::Identity();
	}
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

		// SubUV 모듈의 그리드 정보 복사 (캐싱된 SubUVModule 사용)
		if (this->SubUVModule)
		{
			NewReplayData->SubImages_Horizontal = this->SubUVModule->SubImages_Horizontal;
			NewReplayData->SubImages_Vertical = this->SubUVModule->SubImages_Vertical;

			// 안전 장치: 나눗셈 오류 방지를 위해 최소 1로 설정
			if (NewReplayData->SubImages_Horizontal <= 0) NewReplayData->SubImages_Horizontal = 1;
			if (NewReplayData->SubImages_Vertical <= 0) NewReplayData->SubImages_Vertical = 1;
		}
		else
		{
			// SubUV 모듈이 없으면 기본값 1x1 유지
			NewReplayData->SubImages_Horizontal = 1;
			NewReplayData->SubImages_Vertical = 1;
		}
	}

	return true;
}

/*-----------------------------------------------------------------------------
	ParticleSpriteEmitterInstance
-----------------------------------------------------------------------------*/

FParticleSpriteEmitterInstance::FParticleSpriteEmitterInstance(UParticleSystemComponent* InComponent)
	: FParticleEmitterInstance(InComponent)
{
	NewEmitterData = new FDynamicSpriteEmitterData();
}

FParticleSpriteEmitterInstance::~FParticleSpriteEmitterInstance()
{
	if (NewEmitterData)
	{
		delete NewEmitterData;
		NewEmitterData = nullptr;
	}
}

FDynamicEmitterDataBase* FParticleSpriteEmitterInstance::GetDynamicData(bool bSelected)
{
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	if (!bEnabled)
	{
		return nullptr;
	}

	if (!FillReplayData(NewEmitterData->Source))
	{
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

/*-----------------------------------------------------------------------------
	FParticleMeshEmitterInstance 구현
-----------------------------------------------------------------------------*/

FParticleMeshEmitterInstance::FParticleMeshEmitterInstance(UParticleSystemComponent* InComponent)
    : FParticleEmitterInstance(InComponent)
    , MeshTypeData(nullptr)
    , MeshRotationOffset(0)
{
	NewEmitterData = new FDynamicMeshEmitterData();
}

FParticleMeshEmitterInstance::~FParticleMeshEmitterInstance()
{
	if (NewEmitterData)
	{
		delete NewEmitterData;
		NewEmitterData = nullptr;
	}
}

void FParticleMeshEmitterInstance::InitParameters(UParticleEmitter* InTemplate)
{
    FParticleEmitterInstance::InitParameters(InTemplate);

	UParticleLODLevel* LODLevel = InTemplate->GetLODLevel(0);
	assert(LODLevel);
	MeshTypeData = Cast<UParticleModuleTypeDataMesh>(LODLevel->TypeDataModule);
	assert(MeshTypeData);
}

void FParticleMeshEmitterInstance::Init()
{
    FParticleEmitterInstance::Init();
}

uint32 FParticleMeshEmitterInstance::RequiredBytes()
{
    uint32 Bytes = FParticleEmitterInstance::RequiredBytes();

    MeshRotationOffset = PayloadOffset + Bytes;
    Bytes += sizeof(FMeshRotationPayloadData);

    return Bytes;
}

void FParticleMeshEmitterInstance::PostSpawn(FBaseParticle* Particle, float InterpolationPercentage, float SpawnTime)
{
    FParticleEmitterInstance::PostSpawn(Particle, InterpolationPercentage, SpawnTime);

    FMeshRotationPayloadData* PayloadData = (FMeshRotationPayloadData*)((uint8*)Particle + MeshRotationOffset);
    PayloadData->InitialOrientation = MeshTypeData->RollPitchYawRange.GetValue(SpawnTime);
    PayloadData->Rotation = PayloadData->InitialOrientation;
    PayloadData->RotationRate = FVector::Zero();
    PayloadData->CurContinuousRotation = FVector::Zero();
}

void FParticleMeshEmitterInstance::Tick(float DeltaTime, bool bSuppressSpawning)
{
    FParticleEmitterInstance::Tick(DeltaTime, bSuppressSpawning);

    if (bEnabled && ActiveParticles > 0)
    {
        for (int32 i = 0; i < ActiveParticles; i++)
        {
            DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
            FMeshRotationPayloadData* PayloadData = (FMeshRotationPayloadData*)((uint8*)&Particle + MeshRotationOffset);

            PayloadData->CurContinuousRotation += PayloadData->RotationRate * DeltaTime;

            PayloadData->Rotation = PayloadData->InitialOrientation + PayloadData->CurContinuousRotation;
        }
    }
}

FDynamicEmitterDataBase* FParticleMeshEmitterInstance::GetDynamicData(bool bSelected)
{
    // 메시가 없거나 파티클이 없으면 렌더링 안 함
    if (MeshTypeData->Mesh == nullptr || ActiveParticles <= 0)
    {
        return nullptr;
    }

    UParticleLODLevel* LODLevel = SpriteTemplate->GetLODLevel(0);

    if (!FillReplayData(NewEmitterData->Source))
    {
        return nullptr;
    }

    NewEmitterData->Init(
        bSelected,
        this,   // 복사된 데이터를 사용하므로 this 접근 주의
        MeshTypeData->Mesh,   // 렌더링할 메시 전달
        false,  // UseStaticMeshLODs (단순화: 미사용)
        1.0f    // LODSizeScale
    );

    return NewEmitterData;
}

bool FParticleMeshEmitterInstance::FillReplayData(FDynamicEmitterReplayDataBase& OutData)
{
    if (!FParticleEmitterInstance::FillReplayData(OutData))
    {
        return false;
    }

    OutData.eEmitterType = DET_Mesh;

    // 2. 메시 정보 복사
    FDynamicMeshEmitterReplayData* MeshReplayData = static_cast<FDynamicMeshEmitterReplayData*>(&OutData);
    MeshReplayData->MeshRotationOffset = MeshRotationOffset;

    return true;
}

FDynamicEmitterReplayDataBase* FParticleMeshEmitterInstance::GetReplayData()
{
    if (ActiveParticles <= 0 || !bEnabled) return nullptr;

    FDynamicMeshEmitterReplayData* ReplayData = new FDynamicMeshEmitterReplayData();
    if (!FillReplayData(*ReplayData))
    {
        delete ReplayData;
        return nullptr;
    }
    return ReplayData;
}

/*-----------------------------------------------------------------------------
	FParticleBeamEmitterInstance
-----------------------------------------------------------------------------*/

FParticleBeamEmitterInstance::FParticleBeamEmitterInstance(UParticleSystemComponent* InComponent)
    : FParticleEmitterInstance(InComponent)
    , BeamTypeData(nullptr)
    , BeamPayloadOffset(0)
    , NewEmitterData(nullptr)
{
    NewEmitterData = new FDynamicBeamEmitterData();
}

FParticleBeamEmitterInstance::~FParticleBeamEmitterInstance()
{
    if (NewEmitterData)
    {
        delete NewEmitterData;
        NewEmitterData = nullptr;
    }
}

void FParticleBeamEmitterInstance::InitParameters(UParticleEmitter* InTemplate)
{
    FParticleEmitterInstance::InitParameters(InTemplate);

    UParticleLODLevel* LODLevel = InTemplate->GetLODLevel(0);
    assert(LODLevel);
    BeamTypeData = Cast<UParticleModuleTypeDataBeam>(LODLevel->TypeDataModule);
    assert(BeamTypeData);
}

void FParticleBeamEmitterInstance::Init()
{
    FParticleEmitterInstance::Init();

    // 세그먼트 배열 초기화
    int32 MaxSegments = BeamTypeData ? BeamTypeData->MaxBeamCount + 1 : 11;
    SegmentData.Reserve(MaxSegments);
}

uint32 FParticleBeamEmitterInstance::RequiredBytes()
{
    uint32 Bytes = FParticleEmitterInstance::RequiredBytes();

    BeamPayloadOffset = PayloadOffset + Bytes;
    Bytes += sizeof(FBeamParticlePayloadData);

    return Bytes;
}

FVector FParticleBeamEmitterInstance::CalculateSourcePoint(FBaseParticle* Particle)
{
    FVector SourcePoint = FVector::Zero();

    switch (BeamTypeData->BeamSourceMethod)
    {
    case PEB2M_Emitter:
        // 이미터 위치 사용
        SourcePoint = Location;
        break;

    case PEB2M_Distance:
    case PEB2M_Target:
    default:
        SourcePoint = Location;
        break;
    }

    // Source 오프셋 적용
    FVector Offset = BeamTypeData->SourceOffset.GetValue(Particle->RelativeTime);
    SourcePoint += Offset;

    return SourcePoint;
}

FVector FParticleBeamEmitterInstance::CalculateTargetPoint(FBaseParticle* Particle)
{
    FVector TargetPoint = FVector::Zero();
    FVector SourcePoint = CalculateSourcePoint(Particle);

    switch (BeamTypeData->BeamTargetMethod)
    {
    case PEB2M_Distance:
    {
        // 지정된 거리만큼 떨어진 위치
        float Distance = BeamTypeData->TargetDistance.GetValue(Particle->RelativeTime);
        FVector Direction = BeamTypeData->TargetDirection.GetSafeNormal();
        if (Direction.IsZero())
        {
            Direction = FVector(1.0f, 0.0f, 0.0f);
        }
        TargetPoint = SourcePoint + Direction * Distance;
        break;
    }

    case PEB2M_Emitter:
    case PEB2M_Target:
    default:
    {
        // 기본값: Distance 모드와 동일
        float Distance = BeamTypeData->TargetDistance.GetValue(Particle->RelativeTime);
        FVector Direction = BeamTypeData->TargetDirection.GetSafeNormal();
        if (Direction.IsZero())
        {
            Direction = FVector(1.0f, 0.0f, 0.0f);
        }
        TargetPoint = SourcePoint + Direction * Distance;
        break;
    }
    }

    // Target 오프셋 적용
    FVector Offset = BeamTypeData->TargetOffset.GetValue(Particle->RelativeTime);
    TargetPoint += Offset;

    return TargetPoint;
}

void FParticleBeamEmitterInstance::BuildSegments(FBaseParticle* Particle, FBeamParticlePayloadData* PayloadData)
{
    FVector Source = PayloadData->SourcePoint;
    FVector Target = PayloadData->TargetPoint;
    FVector Direction = Target - Source;
    float Length = Direction.Size();

    if (Length < KINDA_SMALL_NUMBER)
    {
        Length = 1.0f;
        Direction = FVector(1.0f, 0.0f, 0.0f);
    }
    else
    {
        Direction /= Length;
    }

    int32 NumSegments = BeamTypeData->MaxBeamCount;
    if (NumSegments < 1) NumSegments = 1;

    SegmentData.SetNum(NumSegments + 1);  // +1 for end point

    for (int32 i = 0; i <= NumSegments; i++)
    {
        float T = (float)i / NumSegments;  // 0.0 ~ 1.0

        FBeamSegment& Seg = SegmentData[i];
        Seg.Position = FMath::Lerp(Source, Target, T);
        Seg.Tangent = Direction;

        // 너비 계산
        float Width = BeamTypeData->BeamWidth.GetValue(Particle->RelativeTime);

        // Taper (굵기 변화) 적용
        switch (BeamTypeData->TaperMethod)
        {
        case PEBTM_Full:
        {
            float TaperScale = FMath::Lerp(BeamTypeData->SourceTaperScale, BeamTypeData->TargetTaperScale, T);
            Width *= TaperScale;
            break;
        }
        case PEBTM_Partial:
        {
            // 중간이 가장 굵음
            float TaperScale = std::sin(T * PI);
            Width *= TaperScale;
            break;
        }
        case PEBTM_None:
        default:
            break;
        }

        Seg.Width = Width;

        // UV 계산
        if (BeamTypeData->TextureTile > 0)
        {
            Seg.TexCoord = T * BeamTypeData->TextureTile;
        }
        else
        {
            Seg.TexCoord = T;
        }
    }

    PayloadData->SegmentCount = NumSegments;
    PayloadData->BeamLength = Length;
}

void FParticleBeamEmitterInstance::ApplyNoise(FBeamParticlePayloadData* PayloadData, float DeltaTime)
{
    if (!BeamTypeData->bUseNoise) return;
    if (SegmentData.Num() < 3) return;  // 최소 3개 (시작, 중간, 끝) 필요

    PayloadData->NoiseTime += DeltaTime * BeamTypeData->NoiseSpeed;

    float Strength = BeamTypeData->NoiseStrength.GetValue(0);

    // 시작점과 끝점은 노이즈 적용 안함
    for (int32 i = 1; i < SegmentData.Num() - 1; i++)
    {
        float T = (float)i / (SegmentData.Num() - 1);

        // 중앙으로 갈수록 노이즈 강해짐 (0 → 1 → 0)
        float NoiseScale = sin(T * PI) * Strength;

        // 간단한 사인 기반 노이즈 (Perlin 대신)
        float Phase = PayloadData->NoiseSeed + PayloadData->NoiseTime;
        float NoiseX = sin(Phase + i * 1.7f) * cos(Phase * 0.7f + i * 0.5f);
        float NoiseY = cos(Phase + i * 2.3f) * sin(Phase * 0.5f + i * 0.3f);

        // 빔 방향에 수직인 방향으로 오프셋
        FVector Right = FVector::Cross(SegmentData[i].Tangent, FVector(0.0f, 0.0f, 1.0f));
        if (Right.IsZero())
        {
            Right = FVector::Cross(SegmentData[i].Tangent, FVector(0.0f, 1.0f, 0.0f));
        }
        Right.Normalize();

        FVector Up = FVector::Cross(Right, SegmentData[i].Tangent);
        Up.Normalize();

        SegmentData[i].Position += Right * NoiseX * NoiseScale;
        SegmentData[i].Position += Up * NoiseY * NoiseScale;
    }
}

void FParticleBeamEmitterInstance::PostSpawn(FBaseParticle* Particle, float InterpolationPercentage, float SpawnTime)
{
    FParticleEmitterInstance::PostSpawn(Particle, InterpolationPercentage, SpawnTime);

    FBeamParticlePayloadData* PayloadData = (FBeamParticlePayloadData*)((uint8*)Particle + BeamPayloadOffset);

    // Source/Target 계산
    PayloadData->SourcePoint = CalculateSourcePoint(Particle);
    PayloadData->TargetPoint = CalculateTargetPoint(Particle);

    // 방향 벡터 계산
    FVector Dir = (PayloadData->TargetPoint - PayloadData->SourcePoint).GetSafeNormal();
    PayloadData->SourceTangent = Dir;
    PayloadData->TargetTangent = Dir;

    // 노이즈 초기화
    PayloadData->NoiseSeed = rand() * 1000.0f;
    PayloadData->NoiseTime = 0.0f;

    // 세그먼트 생성
    BuildSegments(Particle, PayloadData);
}

void FParticleBeamEmitterInstance::Tick(float DeltaTime, bool bSuppressSpawning)
{
    FParticleEmitterInstance::Tick(DeltaTime, bSuppressSpawning);

    if (bEnabled && ActiveParticles > 0)
    {
        for (int32 i = 0; i < ActiveParticles; i++)
        {
            DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
            FBeamParticlePayloadData* PayloadData = (FBeamParticlePayloadData*)((uint8*)&Particle + BeamPayloadOffset);

            // Source/Target 업데이트 (이미터가 움직였을 수 있음)
            PayloadData->SourcePoint = CalculateSourcePoint(&Particle);
            PayloadData->TargetPoint = CalculateTargetPoint(&Particle);

            // 세그먼트 재생성
            BuildSegments(&Particle, PayloadData);

            // 노이즈 적용
            ApplyNoise(PayloadData, DeltaTime);
        }
    }
}

FDynamicEmitterDataBase* FParticleBeamEmitterInstance::GetDynamicData(bool bSelected)
{
    if (ActiveParticles <= 0)
    {
        return nullptr;
    }

    if (!FillReplayData(NewEmitterData->Source))
    {
        return nullptr;
    }

    NewEmitterData->Init(bSelected, this);

    return NewEmitterData;
}

bool FParticleBeamEmitterInstance::FillReplayData(FDynamicEmitterReplayDataBase& OutData)
{
    if (!FParticleEmitterInstance::FillReplayData(OutData))
    {
        return false;
    }

    OutData.eEmitterType = DET_Beam2;

    FDynamicBeamEmitterReplayData* BeamReplayData = static_cast<FDynamicBeamEmitterReplayData*>(&OutData);

    // 머티리얼 설정 (스프라이트와 동일하게 머티리얼에서 텍스처를 가져옴)
    BeamReplayData->MaterialInterface = CurrentMaterial;
    BeamReplayData->BeamPayloadOffset = BeamPayloadOffset;
    BeamReplayData->MaxSegments = BeamTypeData ? BeamTypeData->MaxBeamCount : 10;
    BeamReplayData->TextureTile = BeamTypeData ? BeamTypeData->TextureTile : 1.0f;
    BeamReplayData->SourceTaperScale = BeamTypeData ? BeamTypeData->SourceTaperScale : 1.0f;
    BeamReplayData->TargetTaperScale = BeamTypeData ? BeamTypeData->TargetTaperScale : 1.0f;

    // 세그먼트 데이터 복사
    BeamReplayData->SegmentData = SegmentData;

    return true;
}

FDynamicEmitterReplayDataBase* FParticleBeamEmitterInstance::GetReplayData()
{
    if (ActiveParticles <= 0 || !bEnabled) return nullptr;

    FDynamicBeamEmitterReplayData* ReplayData = new FDynamicBeamEmitterReplayData();
    if (!FillReplayData(*ReplayData))
    {
        delete ReplayData;
        return nullptr;
    }
    return ReplayData;
}
