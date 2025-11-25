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
}

FParticleEmitterInstance* UParticleModuleTypeDataMesh::CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent)
{
	SetToSensibleDefaults(InEmitterParent);
	FParticleMeshEmitterInstance* Instance = new FParticleMeshEmitterInstance(InComponent);
	assert(Instance);

	Instance->InitParameters(InEmitterParent);
	// @note 일단 메시 정보를 이곳에서 전달함
	Instance->Mesh = Mesh;

	return Instance;
}
