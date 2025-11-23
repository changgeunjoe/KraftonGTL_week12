#include "pch.h"
#include "ParticleHelper.h"

#include "ParticleSpriteEmitter.h"

FDynamicEmitterDataBase::FDynamicEmitterDataBase(const class UParticleModuleRequired* RequiredModule)
	: bSelected(false)
	, EmitterIndex(-1)
{
}

void FDynamicSpriteEmitterData::Init(bool bInSelected)
{
	bSelected = bInSelected;
}

void FParticleDataContainer::Alloc(int32 InParticleDataNumBytes, int32 InParticleIndicesNumShorts)
{
	Free();

	ParticleDataNumBytes = InParticleDataNumBytes;
	ParticleIndicesNumShorts = InParticleIndicesNumShorts;

	if ((ParticleDataNumBytes > 0) && (ParticleIndicesNumShorts > 0))
	{
		MemBlockSize = ParticleDataNumBytes + (ParticleIndicesNumShorts * sizeof(uint16));

		ParticleData = (uint8*)std::malloc(MemBlockSize);
		ParticleIndices = (uint16*)(ParticleData + ParticleDataNumBytes);
	}
}

void FParticleDataContainer::Free()
{
	if (ParticleData)
	{
		assert(MemBlockSize > 0);
		std::free(ParticleData);
	}
	MemBlockSize = 0;
	ParticleDataNumBytes = 0;
	ParticleIndicesNumShorts = 0;
	ParticleData = nullptr;
	ParticleIndices = nullptr;
}

FDynamicSpriteEmitterReplayDataBase::FDynamicSpriteEmitterReplayDataBase()
	: MaterialInterface(nullptr)
	, bUseLocalSpace(false)
	, ScreenAlignment(PSA_Square)
{
}
