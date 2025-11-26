#pragma once
#include "ParticleModule.h"
#include "Source/Runtime/Engine/Distribution/Distributions.h"
#include "UParticleModuleSubUV.generated.h"

/**
 * 스프라이트 시트 애니메이션을 설정하는 모듈.
 * 파티클 텍스처를 여러 프레임으로 나누어 애니메이션을 적용한다.
 */
UCLASS(DisplayName="SubUV Animation", Description="스프라이트 시트 애니메이션 설정")
class UParticleModuleSubUV : public UParticleModule
{
	GENERATED_REFLECTION_BODY()

public:
	/** 스프라이트 시트의 가로 이미지 개수 */
	UPROPERTY(EditAnywhere, Category="Grid")
	int32 SubImages_Horizontal;

	/** 스프라이트 시트의 세로 이미지 개수 */
	UPROPERTY(EditAnywhere, Category="Grid")
	int32 SubImages_Vertical;

	/** 시간에 따른 이미지 인덱스 변화를 정의하는 커브
	 * 입력: 0.0 ~ 1.0 (파티클 수명)
	 * 출력: 0.0 ~ (H*V - 1) (이미지 인덱스)
	 */
	UPROPERTY(EditAnywhere, Category="Animation")
	FRawDistributionFloat SubImageIndex;

public:
	UParticleModuleSubUV();

	virtual ~UParticleModuleSubUV() = default;

	void InitializeDefaults();

	//~Begin UParticleModule 인터페이스
	virtual void Spawn(const FSpawnContext& Context) override;
	virtual void Update(const FUpdateContext& Context) override;
	//~End UParticleModule 인터페이스
};
