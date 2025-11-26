#pragma once

#include "PrimitiveComponent.h"
#include "UParticleSystemComponent.generated.h"

struct FDynamicEmitterDataBase;
class UParticleSystem;

struct FHitResult
{
	bool bBlockingHit;
	float Time;
	float Distance;
	FVector Location;
	FVector Normal;
	FVector ImpactNormal;

	FHitResult()
		: bBlockingHit(false), Time(1.0f), Distance(0.0f)
		, Location(FVector::Zero()), Normal(FVector::Zero())
	{}

	bool IsValidBlockingHit() const { return bBlockingHit; }
};

UCLASS(DisplayName = "파티클 컴포넌트", Description = "파티클을 렌더링하는 컴포넌트입니다")
class UParticleSystemComponent : public UPrimitiveComponent
{
	GENERATED_REFLECTION_BODY()

public:
	UParticleSystemComponent();

	virtual ~UParticleSystemComponent();

	//~ Begin UActorComponent Interface //
	virtual void TickComponent(float DeltaTime) override;
	//~ End UActorComponent Interface //

	virtual void CollectMeshBatches(TArray<FMeshBatchElement>& OutMeshBatchElements, const FSceneView* View) override;

	/**
	 * 파티클 시스템을 초기화하고 이미터 인스턴스를 생성한다.
	 * 템플릿 데이터가 유효해야 수행된다.
	 */
	virtual void InitParticles();

	/**
	 * 기존의 인스턴스를 제거하고, 새로운 인스턴스를 생성한다.
	 * 이미터나 모듈 등이 변경되어 메모리 레이아웃이 변경되었을 때 호출해야한다.
	 */
	void InitializeSystem();

	/**
	 * 생성된 모든 이미터 인스턴스를 제거하고 메모리를 해제한다.
	 * @param bEmptyInstances		true일 경우 배열을 비운다.
	 */
	void ResetParticles(bool bEmptyInstances = false);

	/**
	 * 생성된 모든 이미터 인스턴스를 제거하고 메모리를 해제한다.
	 * @param bEmptyInstances true면 배열을 비운다.
	 */
	void ClearDynamicData();

	virtual void UpdateDynamicData();

	virtual void Activate(bool bReset=false);

	virtual void Deactivate();

	virtual bool ParticleLineCheck(FHitResult& Hit, AActor* SourceActor, const FVector& End, const FVector& Start, const FVector& Extent);

	/**
	 * 파티클 시스템 템플릿을 설정한다.
	 * 기존 파티클을 리셋하고 새로운 템플릿으로 재초기화한다.
	 * @param NewTemplate 새로 설정할 파티클 시스템
	 * @param bAutoActivate true일 경우 자동으로 활성화
	 */
	void SetTemplate(UParticleSystem* NewTemplate, bool bAutoActivate = true);

	/**
	 * 현재 설정된 파티클 시스템 템플릿을 반환한다.
	 * @return 현재 템플릿
	 */
	UParticleSystem* GetTemplate() const { return Template; }

	int32 GetTotalActiveParticles() const;

	int32 GetActiveParticleCount(int32 EmitterIndex) const;

	UPROPERTY(EditAnywhere, Category = "파티클")
	UParticleSystem* Template;

private:
	/** 사용할 파티클 시스템 템플릿 */

	/** 이미터별 머티리얼 오버라이드 배열 */
	TArray<UMaterialInterface*> EmitterMaterials;

	/** 실제 시뮬레이션을 수행하는 이미터 인스턴스들의 배열 */
	TArray<struct FParticleEmitterInstance*> EmitterInstances;

	/** 렌더링 스레드로 전달할 데이터 */
	TArray<FDynamicEmitterDataBase*> EmitterRenderData;

	/** 파티클이 비활성화 되었는지 여부*/
	uint8 bWasDeactivated:1;

	/** 파티클 시스템이 완전히 종료되었는지 여부 */
	uint8 bWasCompleted:1;

	/** 파티클 생성을 억제할지 여부 */
	uint8 bSuppressSpawning:1;

	/** 현재 설정된 LOD 레벨 */
	int32 LODLevel;

	/** 현재 활성화된 총 파티클 수 */
	int32 TotalActiveParticles;
};
