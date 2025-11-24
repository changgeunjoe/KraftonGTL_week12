#pragma once
#include "ParticleModule.h"
#include "UParticleModuleTypeDataBase.generated.h"

class UParticleSystemComponent;
UCLASS()
class UParticleModuleTypeDataBase : public UParticleModule
{
	GENERATED_REFLECTION_BODY()

public:
	UParticleModuleTypeDataBase(){};
	~UParticleModuleTypeDataBase(){};

	//~ Begin UParticleModule Interface
	virtual EModuleType GetModuleType() const override { return EPMT_TypeData; }
	//~ End UParticleModule Interface

	/** 이 데이터 내부의 모듈 포인터를 캐시한다. */
	virtual void CacheModuleInfo(UParticleEmitter* Emitter) {}

	virtual FParticleEmitterInstance* CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent)
	{
		return nullptr;
	}

	virtual bool IsMeshEmitter() const { return false; }
};
