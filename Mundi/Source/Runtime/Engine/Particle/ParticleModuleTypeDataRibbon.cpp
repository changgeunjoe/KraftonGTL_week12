#include "pch.h"
#include "ParticleModuleTypeDataRibbon.h"
#include "ParticleEmitterInstances.h"

UParticleModuleTypeDataRibbon::UParticleModuleTypeDataRibbon()
	: SourceMethod(PRSM_Particle)
	, MaxTrailCount(64)      // 더 많은 포인트 허용
	, TextureTile(1.0f)
	, TailWidthScale(0.5f)   // 꼬리가 절반 굵기
	, TailAlphaScale(0.0f)   // 꼬리가 완전 투명
	, MinSpawnDistance(1.0f) // 더 촘촘하게 샘플링 (5 -> 1)
	, SpawnInterval(0.0f)
	, TrailLifetime(1.0f)    // 트레일 포인트 수명 1초
	, TextureScrollSpeed(0.0f)
{
	InitializeDefaults();
}

void UParticleModuleTypeDataRibbon::InitializeDefaults()
{
	// Ribbon Width 초기화 (기본 10 유닛)
	if (!RibbonWidth.IsCreated())
	{
		UDistributionFloatConstant* Dist = NewObject<UDistributionFloatConstant>();
		Dist->Constant = 10.0f;
		RibbonWidth.Distribution = Dist;
	}
}

FParticleEmitterInstance* UParticleModuleTypeDataRibbon::CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent)
{
	SetToSensibleDefaults(InEmitterParent);
	FParticleRibbonEmitterInstance* Instance = new FParticleRibbonEmitterInstance(InComponent);
	assert(Instance);

	Instance->InitParameters(InEmitterParent);

	return Instance;
}
