#include "pch.h"
#include "ParticleModuleSubUV.h"

#include "ParticleHelper.h"

UParticleModuleSubUV::UParticleModuleSubUV()
{
	bSpawnModule = true;
	bUpdateModule = true;

	SubImages_Horizontal = 1;
	SubImages_Vertical = 1;

	InitializeDefaults();
}

void UParticleModuleSubUV::InitializeDefaults()
{
	if (!SubImageIndex.IsCreated())
	{
		UDistributionFloatBezier* Dist = NewObject<UDistributionFloatBezier>();

		float StartVal = 0.0f;
		float EndVal = (float)((SubImages_Horizontal * SubImages_Vertical) - 1);
		if (EndVal < 0.0f)
		{
			EndVal = 0.0f;
		}

		float Range = EndVal - StartVal;

		Dist->P0 = StartVal;
		Dist->P1 = StartVal + (Range * (1.0f / 3.0f));
		Dist->P2 = StartVal + (Range * (2.0f / 3.0f));
		Dist->P3 = EndVal;

		Dist->MinInput = 0.0f;
		Dist->MaxInput = 1.0f;

		SubImageIndex.Distribution = Dist;
	}
}

void UParticleModuleSubUV::Spawn(const FSpawnContext& Context)
{
	SPAWN_INIT
	float Index = SubImageIndex.GetValue(Particle.RelativeTime);

	Particle.SubImageIndex = Index;
}

void UParticleModuleSubUV::Update(const FUpdateContext& Context)
{
	FParticleEmitterInstance* Owner = &Context.Owner;

	if (Owner == nullptr ||
		Owner->ActiveParticles <= 0 ||
		Owner->ParticleData == nullptr ||
		Owner->ParticleIndices == nullptr)
	{
		return;
	}

	BEGIN_UPDATE_LOOP;
	{
		float IndexValue = SubImageIndex.GetValue(Particle.RelativeTime, Context.GetDistributionData());

		Particle.SubImageIndex = IndexValue;
	}
	END_UPDATE_LOOP;
}
