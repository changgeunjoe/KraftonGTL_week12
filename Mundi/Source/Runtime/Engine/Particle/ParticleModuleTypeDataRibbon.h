#pragma once
#include "ParticleModuleTypeDataBase.h"
#include "ParticleSystemComponent.h"
#include "Source/Runtime/Engine/Distribution/Distributions.h"
#include "UParticleModuleTypeDataRibbon.generated.h"

/** 리본 소스 결정 방법 */
enum ERibbonSourceMethod : int
{
	/** 파티클 위치 사용 */
	PRSM_Particle = 0,
	/** 이미터 위치 사용 */
	PRSM_Emitter = 1,
	PRSM_MAX
};

/**
 * 리본(트레일) 파티클 타입 데이터 모듈.
 * 파티클의 이동 경로를 따라 리본 형태로 렌더링한다.
 * 검기, 미사일 궤적, 마법 효과 등에 사용.
 */
UCLASS(DisplayName="TypeData Ribbon", Description="리본/트레일 타입 파티클 설정")
class UParticleModuleTypeDataRibbon : public UParticleModuleTypeDataBase
{
	GENERATED_REFLECTION_BODY()

public:
	//========== 기본 설정 ==========

	/** 리본 소스 결정 방법 */
	ERibbonSourceMethod SourceMethod;

	/** 트레일당 최대 파티클 수 (히스토리 길이) */
	UPROPERTY(EditAnywhere, Category="Ribbon")
	int32 MaxTrailCount;

	/** 리본 너비 */
	UPROPERTY(EditAnywhere, Category="Ribbon")
	FRawDistributionFloat RibbonWidth;

	/** 텍스처 타일링 모드 (0: 전체 스트레치, 1+: 타일링 횟수) */
	UPROPERTY(EditAnywhere, Category="Ribbon")
	float TextureTile;

	//========== 페이드 설정 ==========

	/** 꼬리 쪽 너비 배율 (0~1, 끝으로 갈수록 가늘어짐) */
	UPROPERTY(EditAnywhere, Category="Fade")
	float TailWidthScale;

	/** 꼬리 쪽 알파 배율 (0~1, 끝으로 갈수록 투명해짐) */
	UPROPERTY(EditAnywhere, Category="Fade")
	float TailAlphaScale;

	//========== 샘플링 설정 ==========

	/** 새 포인트를 추가하는 최소 거리 */
	UPROPERTY(EditAnywhere, Category="Sampling")
	float MinSpawnDistance;

	/** 포인트 추가 간격 (초) - 0이면 매 프레임 */
	UPROPERTY(EditAnywhere, Category="Sampling")
	float SpawnInterval;

	/** 트레일 포인트의 수명 (초) - 이 시간이 지나면 페이드 아웃 */
	UPROPERTY(EditAnywhere, Category="Sampling")
	float TrailLifetime;

	//========== 텍스처 스크롤 설정 ==========

	/** 텍스처 스크롤 속도 (0이면 스크롤 없음) */
	UPROPERTY(EditAnywhere, Category="TextureScroll")
	float TextureScrollSpeed;

public:
	UParticleModuleTypeDataRibbon();

	virtual ~UParticleModuleTypeDataRibbon() = default;

	void InitializeDefaults();

	virtual FParticleEmitterInstance* CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent) override;

	virtual bool IsRibbonEmitter() const { return true; }
};
