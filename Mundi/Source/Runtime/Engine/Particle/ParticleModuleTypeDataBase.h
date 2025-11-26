#pragma once
#include "ParticleModule.h"
#include "UParticleModuleTypeDataBase.generated.h"

class UParticleSystemComponent;

/**
 * 파티클 타입 데이터 모듈의 기본 클래스.
 * 이미터의 렌더링 타입(스프라이트, 메시 등)을 정의하는 모듈들의 부모 클래스.
 */
UCLASS(DisplayName="TypeData Base", Description="타입 데이터 모듈의 기본 클래스")
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
