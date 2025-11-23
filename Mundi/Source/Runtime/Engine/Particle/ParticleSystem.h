#pragma once

class UParticleEmitter;
class UParticleSystemComponent;

UCLASS()
class UParticleSystem : public UObject
{
	DECLARE_CLASS(UParticleSystem, UObject)

public:
	/** FixedTime 모드에서 업데이트하기 위한 초당 프레임 수 */
	float UpdateTime_FPS;

	/** 한 프레임당 걸리는 시간(초 단위, UpdateTime_FPS의 역수) */
	float UpdateTime_Delta;

	/** 시스템 내에 존재하는 이미터들의 배열 */
	TArray<UParticleEmitter*> Emitters;

	/** 캐스케이드 내의 파티클 시스템을 프리뷰하기 위한 컴포넌트 */
	UParticleSystemComponent* PreviewComponent;

	/** 파티클 시스템을 위한 바운딩 박스 (언리얼엔진에서는 FBox 타입을 사용) */
	FAABB FixedRelativeBoundingBox;

public:
	UParticleSystem() = default;

	virtual ~UParticleSystem() = default;

	//~Begin UObject Interface.

	// Serialize...

	//~End UObject Interface.

	/**
	 * @brief 각 이미터에 대하여 최대 활성 파티클 수를 결정한다. 이미터의 생명주기 동안 재할당을 피하기 위해 사용한다.
	 *
	 * @return true		각 이미터들에 대하여 숫자가 결정될 때
	 *		   false	결정되지 않을 때
	 */
	virtual bool CalculateMaxActiveParticleCounts();

	/**
	 * @brief 모든 이미터 모듈 리스트를 업데이트한다.
	 */
	void UpdateAllModuleLists();
};
