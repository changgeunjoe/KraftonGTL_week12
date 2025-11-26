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

void AParticleSystemActor::DuplicateSubObjects()
{
	Super::DuplicateSubObjects();
	for (UActorComponent* Component : OwnedComponents)
	{
		if (UParticleSystemComponent* ParticleComp = Cast<UParticleSystemComponent>(Component))
		{
			ParticleSystemComponent = ParticleComp;
			break;
		}
	}
}

void AParticleSystemActor::Serialize(const bool bInIsLoading, JSON& InOutHandle)
{
	Super::Serialize(bInIsLoading, InOutHandle);

	if (bInIsLoading)
	{
		ParticleSystemComponent = Cast<UParticleSystemComponent>(RootComponent);
	}
}
