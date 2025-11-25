#pragma once

#include "Actor.h"
#include "Enums.h"
#include "AParticleSystemActor.generated.h"

class UParticleSystemComponent;

UCLASS(DisplayName = "파티클 액터", Description = "파티클 액터입니다")
class AParticleSystemActor : public AActor
{
	GENERATED_REFLECTION_BODY()

public:
	AParticleSystemActor();

	UParticleSystemComponent* GetParticleSystemComponent()
	{
		return ParticleSystemComponent;
	}

private:
	UParticleSystemComponent* ParticleSystemComponent = nullptr;
};
