#pragma once

class UParticleModule;
struct FBaseParticle;
class UParticleLODLevel;
class UParticleSystemComponent;
class UParticleEmitter;

/*-----------------------------------------------------------------------------
	FParticleEmitterInstance
-----------------------------------------------------------------------------*/
struct FParticleEmitterInstance
{
public:
	/** @todo 주석 */
	static const float PeakActiveParticleUpdateDelta;

	/** 이 인스턴스가 기반으로 하는 템플릿									*/
	UParticleEmitter* SpriteTemplate;
	/** 이 구조체를 소유하는 컴포넌트										*/
	UParticleSystemComponent* Component;
	/** 현재 설정된 LOD 레벨												*/
	UParticleLODLevel* CurrentLODLevel;
	/** 현재 설정된 LOD 레벨의 인덱스										*/
	int32 CurrentLODLevelIndex;

	/** ...																*/

	/** 파티클 데이터 배열에 대한 포인터		                                */
	uint8* ParticleData;
	/** 파티클 인덱스 배열에 대한 포인터			                            */
	uint16* ParticleIndices;
	/** 인스턴스 데이터 배열에 대한 포인터	                                */
	uint8* InstanceData;
	/** 인스턴스 데이터 배열의 크기				                            */
	int32 InstancePayloadSize;
	/** 파티클 데이터에 대한 오프셋			                                */
	int32 PayloadOffset;
	/** 이미터 인스턴스의 위치				                                */
	FVector Location;
	/** 이미터 로컬 공간에서 시뮬레이션 공간으로의 변환							*/
	FMatrix EmitterToSimulation;
	/** 파티클 데이터의 총 크기 (바이트)				                        */
	int32 ParticleSize;
	/** ParticleData 배열에서 파티클 간의 스트라이드							*/
	int32 ParticleStride;
	/** 이미터 내의 현재 활성화된 파티클의 수									*/
	int32 ActiveParticles;
	/** 단조 증가하는 카운터												*/
	uint32 ParticleCounter;
	/** 파티클 데이터 배열이 보유할 수 있는 최대 활성 파티클 수					*/
	int32 MaxActiveParticles;
	/** 스폰 이후 남은 시간												*/
	float SpawnFraction;
	/** 인스턴스가 생성된 이후 흐른 시간 (초)									*/
	float SecondsSinceCreation;
	/** 인스턴스의 이전 위치												*/
	FVector OldLocation;

	/** 이 인스턴스를 렌더링할 때 사용할 머티리얼								*/
	UMaterialInterface* CurrentMaterial;

public:
	FParticleEmitterInstance(UParticleSystemComponent* InComponent);

	virtual ~FParticleEmitterInstance();

	virtual void InitParamters(UParticleEmitter* InTemplate);
	virtual void Init();

	/**
	 * 요청된 파티클 수를 위한 충분한 메모리를 보장한다.
	 *
	 * @param NewMaxActiveParticles		메모리를 할당받아야 할 파티클의 수
	 * @param bSetMaxActiveCount		True이면 이 LOD에 대한 최대 활성 파티클 수를 설정
	 * @return							최소 NewMaxActiveParticles만큼의 메모리를 할당 받으면 true
	 */
	virtual bool Resize(int32 NewMaxActiveParticles, bool bSetMaxActiveCount = true);

	virtual void Tick(float DeltaTime/**, bool bSuppressSpawning*/);

	/**
	 * 이 이미터 타입이 필요로하는 파티클 당 바이트를 반환한다.
	 * @return uint32		인스턴스 내 파티클을 위해 필요한 바이트 수
	 */
	virtual uint32 RequiredBytes();
	/** 특정 모듈을 위한 파티클 페이로드 데이터의 오프셋을 가져온다. */
	uint32 GetModuleDataOffset(UParticleModule* Module);
	/** 특정 모듈을 위한 이미터 인스턴스 페이로드 데이터의 포인터를 가져온다. */
	uint8* GetModuleInstanceData(UParticleModule* Module);

	/**
	 * 이 이미터 인스턴스에 대한 파티클을 스폰한다.
	 * @param DeltaTime		스폰이 이루어질 시간 조각(Time Slice)
	 * @return float		스폰후 남은 분수 값(남는 스폰 비율)
	 */
	float Spawn(float DeltaTime);

	/**
	 * 지정된 수의 파티클을 스폰한다.
	 * @param Count				스폰할 파티클 수
	 * @param StartTime			파티클 스폰을 시작하는 로컬 이미터 시간
	 * @param Increment			스폰된 파티클 사이의 시간 차
	 * @param InitialLocation	스폰된 파티클의 초기 위치
	 * @param InitialVelocity	스폰된 파티클의 초기 속도
	 */
	void SpawnParticles( int32 Count, float StartTime, float Increment, const FVector& InitialLocation, const FVector& InitialVelocity/*, struct FParticleEventInstancePayload* EventPayload */);

	/**
	 * 파티클이 요구하는 모든 사전-스폰(pre-spawning) 동작들을 수행한다.
	 * @param Particle			스폰되는 파티클
	 * @param InitialLocation	파티클의 초기 위치
	 * @param InitialVelocity	파티클의 초기 속도
	 */
	virtual void PreSpawn(FBaseParticle* Particle, const FVector& InitialLocation, const FVector& InitialVelocity);

	/**
	 * 인스턴스가 요구하는 모든 사후-스폰(post-spawning) 동작들을 수행한다.
	 * @param Particle					스폰된 파티클
	 * @param InterpolationPercentage
	 * @param SpawnTime
	 */
	virtual void PostSpawn(FBaseParticle* Particle, float InterpolationPercentage, float SpawnTime);

	/**
	 * 인스턴스에 대한 이미터 지속시간을 계산한다.
	 */
	void SetupEmitterDuration();

	void KillParticles();

	void Update(float DeltaTime);

	void UpdateTransforms();

	/**
	 * 렌더링되어야 할 메쉬 파티클의 머티리얼들을 설정한다.
	 * @param InMaterials - 머티리얼들
	 */
	virtual void SetMeshMaterials(const TArray<UMaterialInterface*>& InMaterials)
	{
	}

	/**
	 * 현재 LOD 레벨을 가져오고 유효한지 검증한다.
	 */
	UParticleLODLevel* GetCurrentLODLevelChecked();
};
