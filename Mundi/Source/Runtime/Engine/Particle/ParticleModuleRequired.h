#pragma once
#include "ParticleModule.h"
#include "UParticleModuleRequired.generated.h"

enum EParticleScreenAlignment : int;

class UParticleEmitter;

UCLASS()
class UParticleModuleRequired : public UParticleModule
{
	GENERATED_REFLECTION_BODY()

public:
	/** 파티클에 적용할 머티리얼 인터페이스 */
	UPROPERTY(EditAnywhere, Category="Required")
	UMaterialInterface* Material;

	/** 파티클이 화면에 정렬되는 방식 */
	UPROPERTY(EditAnywhere, Category="Required")
	EParticleScreenAlignment ScreenAlignment;

	/** true일 경우 파티클은 이미터(Local) 공간에서 시뮬레이션된다. false면 월드 공간이다. */
	UPROPERTY(EditAnywhere, Category="Required")
	bool bUseLocalSpace;

	/** 이미터가 한 번 실행되는 주기(초) */
	UPROPERTY(EditAnywhere, Category="Duration")
	float EmitterDuration;

	/** 이미터 주기의 최소값 (랜덤 범위 사용 시) */
	UPROPERTY(EditAnywhere, Category="Duration")
	float EmitterDurationLow;

	/** 반복 횟수이다. 0이면 무한 반복. */
	UPROPERTY(EditAnywhere, Category="Duration")
	int32 EmitterLoops;

	/** 시작 전 대기 시간 */
	UPROPERTY(EditAnywhere, Category="Duration")
	float EmitterDelay;

public:
	UParticleModuleRequired();

	virtual ~UParticleModuleRequired() = default;

	/** 기본값을 초기화한다. */
	void InitializeDefaults();

	virtual void SetToSensibleDefaults(UParticleEmitter* Owner) override;
};
