#include "pch.h"
#include "ParticleModuleTypeDataBeam.h"
#include "ParticleEmitterInstances.h"

UParticleModuleTypeDataBeam::UParticleModuleTypeDataBeam()
	: BeamSourceMethod(PEB2M_Emitter)
	, BeamTargetMethod(PEB2M_Distance)
	, TargetDirection(FVector(1.0f, 0.0f, 0.0f))
	, MaxBeamCount(10)
	, TextureTile(1.0f)
	, bUseNoise(false)
	, NoisePoints(5)
	, NoiseSpeed(1.0f)
	, TaperMethod(PEBTM_None)
	, SourceTaperScale(1.0f)
	, TargetTaperScale(1.0f)
{
	InitializeDefaults();
}

void UParticleModuleTypeDataBeam::InitializeDefaults()
{
	// Source Offset 초기화
	if (!SourceOffset.IsCreated())
	{
		UDistributionVectorConstant* Dist = NewObject<UDistributionVectorConstant>();
		Dist->Constant = FVector::Zero();
		SourceOffset.Distribution = Dist;
	}

	// Target Distance 초기화 (기본 100 유닛)
	if (!TargetDistance.IsCreated())
	{
		UDistributionFloatConstant* Dist = NewObject<UDistributionFloatConstant>();
		Dist->Constant = 100.0f;
		TargetDistance.Distribution = Dist;
	}

	// Target Offset 초기화
	if (!TargetOffset.IsCreated())
	{
		UDistributionVectorConstant* Dist = NewObject<UDistributionVectorConstant>();
		Dist->Constant = FVector::Zero();
		TargetOffset.Distribution = Dist;
	}

	// Beam Width 초기화 (기본 10 유닛)
	if (!BeamWidth.IsCreated())
	{
		UDistributionFloatConstant* Dist = NewObject<UDistributionFloatConstant>();
		Dist->Constant = 10.0f;
		BeamWidth.Distribution = Dist;
	}

	// Noise Strength 초기화
	if (!NoiseStrength.IsCreated())
	{
		UDistributionFloatConstant* Dist = NewObject<UDistributionFloatConstant>();
		Dist->Constant = 20.0f;
		NoiseStrength.Distribution = Dist;
	}
}

FParticleEmitterInstance* UParticleModuleTypeDataBeam::CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent)
{
	SetToSensibleDefaults(InEmitterParent);
	FParticleBeamEmitterInstance* Instance = new FParticleBeamEmitterInstance(InComponent);
	assert(Instance);

	Instance->InitParameters(InEmitterParent);

	return Instance;
}
