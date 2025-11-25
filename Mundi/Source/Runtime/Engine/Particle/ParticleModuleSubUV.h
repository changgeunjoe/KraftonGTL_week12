#pragma once
#include "ParticleModule.h"
#include "Source/Runtime/Engine/Distribution/Distributions.h"
#include "UParticleModuleSubUV.generated.h"

UCLASS()
class UParticleModuleSubUV : public UParticleModule
{
	GENERATED_REFLECTION_BODY()

public:

	/** 스프라이트 시트의 가로 이미지 개수 */
	UPROPERTY(EditAnywhere, Category="SubUV")
	int32 SubImages_Horizontal;

	/** 스프라이트 시트의 세로 이미지 개수 */
	UPROPERTY(EditAnywhere, Category="SubUV")
	int32 SubImages_Vertical;

	/** * 시간에 따른 이미지 인덱스 변화를 정의하는 커브
	 * 입력: 0.0 ~ 1.0 (파티클 수명)
	 * 출력: 0.0 ~ (H*V - 1) (이미지 인덱스)
	 */
	UPROPERTY(EditAnywhere, Category="SubUV")
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
