#pragma once
#include "ParticleModuleTypeDataBase.h"
#include "ParticleSystemComponent.h"
#include "Source/Runtime/Engine/Distribution/Distributions.h"

enum EMeshScreenAlignment : int
{
	PSMA_MeshFaceCameraWithRoll,
	PSMA_MeshFaceCameraWithSpin,
	PSMA_MeshFaceCameraWithLockedAxis,
	PSMA_MAX,
};

UCLASS()
class UParticleModuleTypeDataMesh : public UParticleModuleTypeDataBase
{
	DECLARE_CLASS(UParticleModuleTypeDataMesh, UParticleModuleTypeDataBase)
public:
	UStaticMesh* Mesh;

	/**
	 * 메시 정렬 방식은 다음 중 한가지이다.
	 * PSMA_MeshFaceCameraWithRoll
	 * PSMA_MeshFaceCameraWithSpin
	 * PSMA_MeshFaceCameraWithLockedAxis
	 */
	EMeshScreenAlignment MeshAlignment;

	/**
	 * True일 경우, 스태틱 메시에 적용된 것이 아닌 이미터 머티리얼을 사용한다.
	 */
	uint8 bOverrideMaterial:1;

	/** @todo PROPERTY */
	FRawDistributionVector RollPitchYawRange;

public:
	UParticleModuleTypeDataMesh();

	virtual ~UParticleModuleTypeDataMesh() = default;

	void InitializeDefaults();

	virtual FParticleEmitterInstance* CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent) override;

	virtual bool IsMeshEmitter() const override { return true; }
};
