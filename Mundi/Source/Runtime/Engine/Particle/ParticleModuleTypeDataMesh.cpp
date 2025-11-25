#include "pch.h"
#include "ParticleModuleTypeDataMesh.h"
#include "ParticleEmitterInstances.h"

IMPLEMENT_CLASS(UParticleModuleTypeDataMesh, UParticleModuleTypeDataBase)

UParticleModuleTypeDataMesh::UParticleModuleTypeDataMesh()
	: Mesh(nullptr)
	, bOverrideMaterial(true)
	, MeshAlignment(PSMA_MeshFaceCameraWithSpin)
{
	// NOTE: 하드 코딩으로 스태틱 메시 추가 (추후 삭제 필요)
	Mesh = UResourceManager::GetInstance().Load<UStaticMesh>(GDataDir + "/Model/cube-tex.obj");

	InitializeDefaults();
}

void UParticleModuleTypeDataMesh::InitializeDefaults()
{
	if (!RollPitchYawRange.IsCreated())
	{
		UDistributionVectorUniform* Dist = NewObject<UDistributionVectorUniform>();
		Dist->Min = FVector(0, 0, 0);
		Dist->Max = FVector(90, 90, 90);
		RollPitchYawRange.Distribution = Dist;
	}
}

FParticleEmitterInstance* UParticleModuleTypeDataMesh::CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent)
{
	SetToSensibleDefaults(InEmitterParent);
	FParticleMeshEmitterInstance* Instance = new FParticleMeshEmitterInstance(InComponent);
	assert(Instance);

	Instance->InitParameters(InEmitterParent);

	return Instance;
}
