#pragma once
#include "ParticleModule.h"

// @todo 주석
class UParticleModuleRequired : public UParticleModule
{
public:
	UMaterialInterface* Material;

	bool bUseLocalSpace;

	float EmitterDuration;

	float EmitterDurationLow;

	int32 EmitterLoops;

	float EmitterDelay;

	bool bKillOnDeactivate;

	bool bKillOnCompleted;

public:
	UParticleModuleRequired();

	virtual ~UParticleModuleRequired() = default;
};
