#pragma once

class UParticleModuleTypeDataMesh;
class UParticleModuleTypeDataBeam;
struct FBeamSegment;
struct FBeamParticlePayloadData;
struct FDynamicEmitterReplayDataBase;
struct FDynamicEmitterDataBase;
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
	/** 이미터 로컬 공간에서 시뮬레이션(컴포넌트) 공간으로의 변환				*/
	FMatrix EmitterToSimulation;
	/**	시뮬레이션 공간에서 월드 공간으로의 변환								*/
	FMatrix SimulationToWorld;
	/** 컴포넌트는 이 이미터의 렌더링과 틱을 비활성화 할 수 있음					*/
	uint8 bEnabled : 1;
	/** 파티클 데이터의 총 크기 (바이트)				                        */
	int32 ParticleSize;
	/** ParticleData 배열에서 파티클 간의 스트라이드							*/
	int32 ParticleStride;
	/** 이미터 내의 현재 활성화된 파티클의 수									*/
	int32 ActiveParticles;
	/** 파티클 데이터 배열이 보유할 수 있는 최대 활성 파티클 수					*/
	int32 MaxActiveParticles;
	/** 스폰 이후 남은 시간												*/
	float SpawnFraction;
	/** 인스턴스가 생성된 이후 흐른 시간 (초)									*/
	float SecondsSinceCreation;
	/** 현재 루프 내에서의 시간 (EmitterDuration에 도달하면 루프하거나 종료)	*/
	float EmitterTime;
	/** */
	float LastDeltaTime;
	/** 인스턴스의 이전 위치												*/
	FVector OldLocation;
	/** 파티클의 바운딩 박스												*/
	FAABB ParticleBoundingBox;
	/** 현재 루프에 적용된 지연 시간										*/
	float CurrentDelay;
	/** 이 인스턴스에 의해 완료된 루프의 수									*/
	int32 LoopCount;
	/** 이미터 인스턴스의 총 지속시간										*/
	float EmitterDuration;
	/** 이 인스턴스의 각 LOD 레벨에 대한 이미터 지속시간						*/
	TArray<float> EmitterDurations;

	/** 이 인스턴스를 렌더링할 때 사용할 머티리얼								*/
	UMaterialInterface* CurrentMaterial;

	// SubUV 모듈 캐싱
	class UParticleModuleSubUV* SubUVModule;

public:
	FParticleEmitterInstance(UParticleSystemComponent* InComponent);

	virtual ~FParticleEmitterInstance();

	virtual void InitParameters(UParticleEmitter* InTemplate);
	virtual void Init();

	/**
	 * 요청된 파티클 수를 위한 충분한 메모리를 보장한다.
	 *
	 * @param NewMaxActiveParticles		메모리를 할당받아야 할 파티클의 수
	 * @param bSetMaxActiveCount		True이면 이 LOD에 대한 최대 활성 파티클 수를 설정
	 * @return							최소 NewMaxActiveParticles만큼의 메모리를 할당 받으면 true
	 */
	virtual bool Resize(int32 NewMaxActiveParticles, bool bSetMaxActiveCount = true);

	virtual void Tick(float DeltaTime, bool bSuppressSpawning);

	virtual void UpdateBoundingBox(float DeltaTime);

	/**
	 * EmitterTime 설정, 루핑(looping) 등을 처리하는 틱(Tick) 하위 함수이다.
	 *
	 * @param DeltaTime        현재 타임 슬라이스(프레임 시간).
	 * @param InCurrentLODLevel  인스턴스의 현재 LOD 레벨.
	 *
	 * @return    float        이미터 지연 시간(EmitterDelay).
	 */
	virtual float Tick_EmitterTimeSetup(float DeltaTime, UParticleLODLevel* InCurrentLODLevel);

	/**
	 * 파티클의 스폰(spawning)을 처리하는 틱 하위 함수이다.
	 *
	 * @param DeltaTime        현재 타임 슬라이스.
	 * @param InCurrentLODLevel  인스턴스의 현재 LOD 레벨.
	 * @param bSuppressSpawning 소유 중인 파티클 시스템 컴포넌트에서 스폰이 억제(suppress)된 경우 true.
	 * @param bFirstTime       이 인스턴스가 처음으로 틱(tick) 되는 경우 true.
	 *
	 * @return    float        남은 스폰 분수(SpawnFraction, 정수 스폰 후 남은 소수점 단위 값).
	 */
	virtual float Tick_SpawnParticles(float DeltaTime, UParticleLODLevel* InCurrentLODLevel, bool bSuppressSpawning, bool bFirstTime);

	/**
	 * 모듈 업데이트를 처리하는 틱 하위 함수이다.
	 *
	 * @param DeltaTime        현재 타임 슬라이스.
	 * @param InCurrentLODLevel  인스턴스의 현재 LOD 레벨.
	 */
	virtual void Tick_ModuleUpdate(float DeltaTime, UParticleLODLevel* InCurrentLODLevel);

	/**
	 * 모듈의 포스트(후) 업데이트를 처리하는 틱 하위 함수이다.
	 *
	 * @param DeltaTime        현재 타임 슬라이스.
	 * @param InCurrentLODLevel  인스턴스의 현재 LOD 레벨.
	 */
	virtual void Tick_ModulePostUpdate(float DeltaTime, UParticleLODLevel* InCurrentLODLevel);

	/**
	 * 모듈의 최종(FINAL) 업데이트를 처리하는 틱 하위 함수이다.
	 *
	 * @param DeltaTime        현재 타임 슬라이스.
	 * @param InCurrentLODLevel  인스턴스의 현재 LOD 레벨.
	 */
	virtual void Tick_ModuleFinalUpdate(float DeltaTime, UParticleLODLevel* InCurrentLODLevel);

	/**
	 * 이 이미터 타입이 필요로하는 파티클 당 바이트를 반환한다.
	 * @return uint32		인스턴스 내 파티클을 위해 필요한 바이트 수
	 */
	virtual uint32 RequiredBytes();
	/** 특정 모듈을 위한 파티클 페이로드 데이터의 오프셋을 가져온다. */
	uint32 GetModuleDataOffset(UParticleModule* Module);
	/** 특정 모듈을 위한 이미터 인스턴스 페이로드 데이터의 포인터를 가져온다. */
	uint8* GetModuleInstanceData(UParticleModule* Module);
	virtual uint32 CalculateParticleStride(uint32 ParticleSize);
	virtual void ResetParticleParameters(float DeltaTime);

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

	virtual void KillParticles();

	virtual void KillParticle(int32 Index);

	virtual FDynamicEmitterDataBase* GetDynamicData(bool bSelected)
	{
		return nullptr;
	}

	virtual FDynamicEmitterReplayDataBase* GetReplayData()
	{
		return nullptr;
	}

	void UpdateTransforms();

	/**
	 * 현재 LOD 레벨을 가져오고 유효한지 검증한다.
	 */
	UParticleLODLevel* GetCurrentLODLevelChecked();

protected:
	/**
	 * 파티클 데이터를 렌더링용 데이터 구조체(OutData)에 복사한다.
	 * @param OutData   데이터가 복사될 대상 구조체
	 * @return          성공 여부
	 */
	virtual bool FillReplayData( FDynamicEmitterReplayDataBase& OutData );
};

/*-----------------------------------------------------------------------------
	ParticleSpriteEmitterInstance
-----------------------------------------------------------------------------*/

struct FParticleSpriteEmitterInstance : public FParticleEmitterInstance
{
	FParticleSpriteEmitterInstance(UParticleSystemComponent* InComponent);

	virtual ~FParticleSpriteEmitterInstance();

	/**
	 * 렌더링 스레드에서 사용할 동적 데이터를 생성하고 반환한다.
	 * @param bSelected         에디터에서 선택되었는지 여부
	 * @return                  생성된 렌더링 데이터 객체 (FDynamicEmitterDataBase*)
	 */
	virtual FDynamicEmitterDataBase* GetDynamicData(bool bSelected) override;

	/**
	 * 리플레이 데이터(파티클 스냅샷)를 생성하여 채운다.
	 * 렌더링 데이터 생성 과정의 핵심 부분이다.
	 * @return 성공 시, 리플레이 데이터 객체 반환
	 */
	virtual FDynamicEmitterReplayDataBase* GetReplayData() override;

protected:

	/**
	 * 파티클 데이터를 렌더링용 데이터 구조체(OutData)에 복사한다.
	 * @param OutData   데이터가 복사될 대상 구조체
	 * @return          성공 여부
	 */
	virtual bool FillReplayData( FDynamicEmitterReplayDataBase& OutData ) override;

	UMaterialInterface* GetCurrentMaterial();

	class FDynamicSpriteEmitterData* NewEmitterData;
};

/*-----------------------------------------------------------------------------
	ParticleMeshEmitterInstance
-----------------------------------------------------------------------------*/

struct FParticleMeshEmitterInstance : public FParticleEmitterInstance
{
	UParticleModuleTypeDataMesh* MeshTypeData;

	/** 파티클 페이로드 내의 메시 회전 데이터 오프셋 */
	int32 MeshRotationOffset;

	FParticleMeshEmitterInstance(UParticleSystemComponent* InComponent);

	virtual ~FParticleMeshEmitterInstance();

	//~ FParticleEmitterInstance 인터페이스 구현
	virtual void InitParameters(UParticleEmitter* InTemplate) override;
	virtual void Init() override;

	// 파티클 메모리 할당 (기본 + 3D 회전값)
	virtual uint32 RequiredBytes() override;

	// 스폰 시 초기 회전값 설정
	virtual void PostSpawn(FBaseParticle* Particle, float InterpolationPercentage, float SpawnTime) override;

	// 매 프레임 회전 및 바운딩 박스 업데이트
	virtual void Tick(float DeltaTime, bool bSuppressSpawning) override;
	// virtual void UpdateBoundingBox(float DeltaTime) override;

	// 렌더 스레드로 데이터 전송
	virtual FDynamicEmitterDataBase* GetDynamicData(bool bSelected) override;
	virtual FDynamicEmitterReplayDataBase* GetReplayData() override;

protected:
	// 렌더 데이터 복사 헬퍼
	virtual bool FillReplayData(FDynamicEmitterReplayDataBase& OutData) override;

	class FDynamicMeshEmitterData* NewEmitterData;
};

/*-----------------------------------------------------------------------------
	ParticleBeamEmitterInstance
-----------------------------------------------------------------------------*/

struct FParticleBeamEmitterInstance : public FParticleEmitterInstance
{
	UParticleModuleTypeDataBeam* BeamTypeData;

	/** 빔 페이로드 오프셋 */
	int32 BeamPayloadOffset;

	/** 세그먼트 데이터 저장소 (렌더링에 사용) */
	TArray<FBeamSegment> SegmentData;

	FParticleBeamEmitterInstance(UParticleSystemComponent* InComponent);

	virtual ~FParticleBeamEmitterInstance();

	//~ FParticleEmitterInstance 인터페이스 구현
	virtual void InitParameters(UParticleEmitter* InTemplate) override;
	virtual void Init() override;

	/** 빔 페이로드 크기 반환 */
	virtual uint32 RequiredBytes() override;

	/** 빔 초기화 (Source/Target 계산) */
	virtual void PostSpawn(FBaseParticle* Particle, float InterpolationPercentage, float SpawnTime) override;

	/** 빔 업데이트 (노이즈, 세그먼트 재계산) */
	virtual void Tick(float DeltaTime, bool bSuppressSpawning) override;

	/** 렌더링 데이터 생성 */
	virtual FDynamicEmitterDataBase* GetDynamicData(bool bSelected) override;
	virtual FDynamicEmitterReplayDataBase* GetReplayData() override;

protected:
	virtual bool FillReplayData(FDynamicEmitterReplayDataBase& OutData) override;

	/** Source 위치 계산 */
	FVector CalculateSourcePoint(FBaseParticle* Particle);

	/** Target 위치 계산 */
	FVector CalculateTargetPoint(FBaseParticle* Particle);

	/** 세그먼트 생성 (Source → Target 사이 분할) */
	void BuildSegments(FBaseParticle* Particle, FBeamParticlePayloadData* PayloadData);

	/** 노이즈 적용 */
	void ApplyNoise(FBeamParticlePayloadData* PayloadData, float DeltaTime);

	class FDynamicBeamEmitterData* NewEmitterData;
};
