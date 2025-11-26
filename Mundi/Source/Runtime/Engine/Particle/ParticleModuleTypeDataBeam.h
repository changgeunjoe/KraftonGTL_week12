#pragma once
#include "ParticleModuleTypeDataBase.h"
#include "ParticleSystemComponent.h"
#include "Source/Runtime/Engine/Distribution/Distributions.h"
#include "UParticleModuleTypeDataBeam.generated.h"

/** 빔 소스/타겟 결정 방법 */
enum EBeamMethod : int
{
	/** 이미터 위치 사용 */
	PEB2M_Emitter = 0,
	/** 지정된 거리만큼 떨어진 위치 */
	PEB2M_Distance = 1,
	/** 타겟 포인트/액터 사용 */
	PEB2M_Target = 2,
	PEB2M_MAX
};

/** 빔 테이퍼(굵기 변화) 방식 */
enum EBeamTaperMethod : int
{
	/** 굵기 변화 없음 */
	PEBTM_None = 0,
	/** Source에서 Target으로 갈수록 가늘어짐 */
	PEBTM_Full = 1,
	/** 중간이 가장 굵음 */
	PEBTM_Partial = 2,
	PEBTM_MAX
};

/**
 * 빔 파티클 타입 데이터 모듈.
 * 두 점 사이를 연결하는 빔 형태의 파티클을 렌더링한다.
 * 레이저, 번개, 전기 효과 등에 사용.
 */
UCLASS(DisplayName="TypeData Beam", Description="빔 타입 파티클 설정")
class UParticleModuleTypeDataBeam : public UParticleModuleTypeDataBase
{
	GENERATED_REFLECTION_BODY()

public:
	//========== Source 설정 ==========

	/** 빔 시작점 결정 방법 */
	EBeamMethod BeamSourceMethod;

	/** Source 오프셋 (로컬 좌표) */
	UPROPERTY(EditAnywhere, Category="Source")
	FRawDistributionVector SourceOffset;

	//========== Target 설정 ==========

	/** 빔 끝점 결정 방법 */
	EBeamMethod BeamTargetMethod;

	/** Target이 Distance일 때의 거리 */
	UPROPERTY(EditAnywhere, Category="Target")
	FRawDistributionFloat TargetDistance;

	/** Target 오프셋 */
	UPROPERTY(EditAnywhere, Category="Target")
	FRawDistributionVector TargetOffset;

	/** Target 방향 (Distance 모드일 때 사용) */
	UPROPERTY(EditAnywhere, Category="Target")
	FVector TargetDirection;

	//========== Beam 설정 ==========

	/** 빔을 구성하는 세그먼트 수 (많을수록 부드러움) */
	UPROPERTY(EditAnywhere, Category="Beam")
	int32 MaxBeamCount;

	/** 빔 너비 (두께) */
	UPROPERTY(EditAnywhere, Category="Beam")
	FRawDistributionFloat BeamWidth;

	/** 텍스처 타일링 횟수 (0이면 타일링 없음) */
	UPROPERTY(EditAnywhere, Category="Beam")
	float TextureTile;

	//========== Noise 설정 (번개 효과) ==========

	/** 노이즈 사용 여부 */
	bool bUseNoise;

	/** 노이즈 포인트 수 */
	UPROPERTY(EditAnywhere, Category="Noise")
	int32 NoisePoints;

	/** 노이즈 강도 (진폭) */
	UPROPERTY(EditAnywhere, Category="Noise")
	FRawDistributionFloat NoiseStrength;

	/** 노이즈 변화 속도 */
	UPROPERTY(EditAnywhere, Category="Noise")
	float NoiseSpeed;

	/** 노이즈 옥타브 수 (1~4, 높을수록 디테일한 노이즈) */
	UPROPERTY(EditAnywhere, Category="Noise")
	float NoiseOctaves;

	//========== 텍스처 스크롤 설정 ==========

	/** 텍스처 스크롤 속도 (0이면 스크롤 없음) */
	UPROPERTY(EditAnywhere, Category="TextureScroll")
	float TextureScrollSpeed;

	//========== 두께 펄스 설정 ==========

	/** 두께 펄스 속도 */
	UPROPERTY(EditAnywhere, Category="Pulse")
	float PulseSpeed;

	/** 두께 펄스 스케일 (0이면 펄스 없음) */
	UPROPERTY(EditAnywhere, Category="Pulse")
	float PulseScale;

	//========== Taper 설정 ==========

	/** 굵기 변화 방식 */
	EBeamTaperMethod TaperMethod;

	/** 소스 쪽 굵기 배율 */
	UPROPERTY(EditAnywhere, Category="Taper")
	float SourceTaperScale;

	/** 타겟 쪽 굵기 배율 */
	UPROPERTY(EditAnywhere, Category="Taper")
	float TargetTaperScale;

public:
	UParticleModuleTypeDataBeam();

	virtual ~UParticleModuleTypeDataBeam() = default;

	void InitializeDefaults();

	virtual FParticleEmitterInstance* CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent) override;

	virtual bool IsBeamEmitter() const { return true; }
};
