#pragma once
#include "ParticleModule.h"

enum EParticleScreenAlignment : int;

class UParticleEmitter;

UCLASS()
class UParticleModuleRequired : public UParticleModule
{
	DECLARE_CLASS(UParticleModuleRequired, UParticleModule)

public:
	/** 파티클에 적용할 머티리얼 인터페이스 */
	UMaterialInterface* Material;

	/** 파티클이 화면에 정렬되는 방식 */
	EParticleScreenAlignment ScreenAlignment;

	/** true일 경우 파티클은 이미터(Local) 공간에서 시뮬레이션된다. false면 월드 공간이다. */
	bool bUseLocalSpace;

	/** 이미터가 한 번 실행되는 주기(초) */
	float EmitterDuration;

	/** 이미터 주기의 최소값 (랜덤 범위 사용 시) */
	float EmitterDurationLow;

	/** 반복 횟수이다. 0이면 무한 반복. */
	int32 EmitterLoops;

	/** 시작 전 대기 시간 */
	float EmitterDelay;

public:
	UParticleModuleRequired();

	virtual ~UParticleModuleRequired() = default;

	/** 기본값을 초기화한다. */
	void InitializeDefaults();

	virtual void SetToSensibleDefaults(UParticleEmitter* Owner) override;
};
