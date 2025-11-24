#include "pch.h"
#include "ParticleSystemActor.h"
#include "ParticleSystemComponent.h"

AParticleSystemActor::AParticleSystemActor()
{
	bTickInEditor = true;

	ObjectName = "Particle System Actor";
	ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>("Particle System Component");

	// 루트 교체
	RootComponent = ParticleSystemComponent;
}
