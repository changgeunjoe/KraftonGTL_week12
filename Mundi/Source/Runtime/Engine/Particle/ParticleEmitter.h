#pragma once
#include "ParticleModule.h"

class UParticleSystemComponent;
class UParticleLODLevel;

UCLASS()
class UParticleEmitter : public UObject
{
	DECLARE_CLASS(UParticleEmitter, UObject)

public:
	//~=============================================================================
	//	General variables
	//~=============================================================================
	/** 이미터의 이름 */
	FName EmitterName;

	//~=============================================================================
	//	'Private' data - not required by the editor
	//~=============================================================================

	/** * 이 이미터의 LOD 레벨 배열
	 * 인덱스 0이 가장 높은 품질(Highest Quality)
	 */
	TArray<UParticleLODLevel*> LODLevels;

	/** * 이 이미터가 동시에 가질 수 있는 최대 파티클 수의 추정치이다.
	 * 메모리 프리사이징(Pre-sizing)에 사용
	 */
	int32 PeakActiveParticles;

	//~=============================================================================
	//	Performance/LOD Data
	//~=============================================================================

	/** 초기 할당 수 - 0보다 크면 PeakActiveParticles 대신 이 값을 사용해 메모리를 미리 할당한다. */
	int32 InitialAllocationCount;

	/** * 모듈 포인터 -> 파티클 페이로드 내 오프셋(Offset) 매핑
	 * 예: ModuleColor 포인터를 넣으면, 파티클 데이터 내에서 색상 값이 시작되는 바이트 위치를 반환한다.
	 */
	TMap<UParticleModule*, uint32> ModuleOffsetMap;

	/** 모듈 포인터 -> 이미터 인스턴스 데이터 내 오프셋 매핑 (Per-Instance Data) */
	TMap<UParticleModule*, uint32> ModuleInstanceOffsetMap;

	/** MeshMaterial 모듈로부터 수집된 머티리얼들 */
	TArray<UMaterialInterface*> MeshMaterials;

	/** * 파티클 1개당 필요한 '추가' 데이터의 크기 (바이트 단위)
	 * FBaseParticle 크기는 제외된 값이다.
	 */
	int32 ParticleSize;

	/** 이미터 인스턴스 하나당 필요한 데이터 크기 */
	int32 ReqInstanceBytes;

	/** 이미터 인스턴스 데이터를 필요로하는 모듈의 배열 */
	TArray<UParticleModule*> ModulesNeedingInstanceData;

public:
	UParticleEmitter();

	virtual ~UParticleEmitter() = default;

	//~Begin UObject Interface.

	// Serialize...

	//~End UObject Interface.

	virtual void SetToSensibleDefaults() {}

	/** 모듈 리스트를 갱신하고 오프셋을 재계산한다. */
	virtual void UpdateModuleLists();

	/** 런타임 시뮬레이션 인스턴스를 생성한다. (Subclass에서 구현) */
	virtual FParticleEmitterInstance* CreateInstance(UParticleSystemComponent* InComponent);

	/** 이미터 이름을 설정한다. */
	void SetEmitterName(FName Name);

	/** 이미터 이름을 가져온다. */
	FName& GetEmitterName();

	/** 현재 인스턴스의 LOD 레벨을 반환한다. */
	UParticleLODLevel* GetCurrentLODLevel(FParticleEmitterInstance* Instance);

	/**
	 * 지정된 LODLevel을 반환한다. LODLevel은 [0..# LOD Levels] 범위에 속해야한다.
	 *
	 * @param LODLevel - 범위 [0..# LOD Levels] 내에서 요청된 LOD 레벨
	 * @return 요청된 LODLevel이 유효하지 않을 경우 nullptr
	 *		   유효할 경우 UParticleLODLevel에 대한 포인터
	 */
	UParticleLODLevel* GetLODLevel(int32 LODLevel);

	/** 최대 활성 파티클 수를 계산하여 PeakActiveParticles를 갱신한다. */
	virtual bool CalculateMaxActiveParticleCount();

	/** 모듈로부터 온 데이터 크기/오프셋과 다른 정보들을 사전 계산한다. */
	void CacheEmitterModuleInfo();

	/** 모든 모듈로부터 이미터에 의한 시뮬레이션을 위해 필요한 데이터를 빌드한다. */
	void Build();
};
