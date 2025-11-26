#pragma once
#include "ParticleModuleTypeDataBase.h"
#include "ParticleSystemComponent.h"
#include "Source/Runtime/Engine/Distribution/Distributions.h"
#include "UParticleModuleTypeDataMesh.generated.h"

enum EMeshScreenAlignment : int
{
	PSMA_MeshFaceCameraWithRoll,
	PSMA_MeshFaceCameraWithSpin,
	PSMA_MeshFaceCameraWithLockedAxis,
	PSMA_MAX,
};

/**
 * 메시 파티클 타입 데이터 모듈.
 * 파티클을 스프라이트 대신 3D 메시로 렌더링한다.
 */
UCLASS(DisplayName="TypeData Mesh", Description="메시 타입 파티클 설정")
class UParticleModuleTypeDataMesh : public UParticleModuleTypeDataBase
{
	GENERATED_REFLECTION_BODY()
public:
	/** 파티클로 사용할 스태틱 메시 */
	UPROPERTY(EditAnywhere, Category="Mesh")
	UStaticMesh* Mesh;

	/** 메시 정렬 방식
	 * PSMA_MeshFaceCameraWithRoll
	 * PSMA_MeshFaceCameraWithSpin
	 * PSMA_MeshFaceCameraWithLockedAxis
	 */
	EMeshScreenAlignment MeshAlignment;

	/** True일 경우, 스태틱 메시에 적용된 것이 아닌 이미터 머티리얼을 사용한다. */
	uint8 bOverrideMaterial:1;

	/** 메시의 회전 범위 (Roll, Pitch, Yaw) */
	UPROPERTY(EditAnywhere, Category="Rotation")
	FRawDistributionVector RollPitchYawRange;

	UPROPERTY(EditAnywhere, Category="Rotation")
	FRawDistributionVector RotationRate;

public:
	UParticleModuleTypeDataMesh();

	virtual ~UParticleModuleTypeDataMesh() = default;

	void InitializeDefaults();

	virtual FParticleEmitterInstance* CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent) override;

	virtual bool IsMeshEmitter() const override { return true; }
};
