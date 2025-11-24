#pragma once
#include "ParticleEmitter.h"

class UParticleSystemComponent;

enum EParticleScreenAlignment : int
{
	PSA_FacingCameraPosition,
	PSA_Square,
	PSA_Rectangle,
	PSA_Velocity,
	PSA_AwayFromCenter,
	PSA_TypeSpecific,
	PSA_FacingCameraDistanceBlend,
	PSA_MAX,
};

class UParticleSpriteEmitter : public UParticleEmitter
{
public:

	//~ Begin UParticleEmitter Interface
	virtual FParticleEmitterInstance* CreateInstance(UParticleSystemComponent* InComponent) override;
	virtual void SetToSensibleDefaults() override;
	//~ End UParticleEmitter Interface
};
