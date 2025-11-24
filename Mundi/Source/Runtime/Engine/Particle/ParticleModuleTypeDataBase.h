#pragma once
#include "ParticleModule.h"

UCLASS()
class UParticleModuleTypeDataBase : public UParticleModule
{
	DECLARE_CLASS(UParticleModuleTypeDataBase, UParticleModule)

public:
	/** 이 데이터 내부의 모듈 포인터를 캐시한다. */
	virtual void CacheModuleInfo(UParticleEmitter* Emitter) {}
};
