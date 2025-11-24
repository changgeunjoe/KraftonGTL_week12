#pragma once
#include "ParticleModule.h"
#include "UParticleModuleTypeDataBase.generated.h"

UCLASS()
class UParticleModuleTypeDataBase : public UParticleModule
{
	GENERATED_REFLECTION_BODY()

public:
	UParticleModuleTypeDataBase(){};
	~UParticleModuleTypeDataBase(){};
	/** 이 데이터 내부의 모듈 포인터를 캐시한다. */
	virtual void CacheModuleInfo(UParticleEmitter* Emitter) {}
};
