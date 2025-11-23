#pragma once
#include "ParticleModule.h"

class UParticleModuleLifetimeBase : public UParticleModule
{
public:
	virtual float GetMaxLifetime()
	{
		return 0.0f;
	}

	// @todo 주석
	virtual float GetLifetimeValue(const FContext& Context, float InTime, UObject* Data = nullptr) {}
};

class UParticleModuleLifetime : public UParticleModuleLifetimeBase
{
public:

};
