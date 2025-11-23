#pragma once

/*-----------------------------------------------------------------------------
	FBaseParticle
-----------------------------------------------------------------------------*/

struct FBaseParticle
{
	// 48 bytes
	FVector		OldLocation;		// 지난 프레임의 위치 (충돌을 위해 사용됨)
	FVector		Location;			// 현재 위치

	// 16 bytes
	FVector			BaseVelocity;		// 각 프레임의 시작 시점에서 Velocity = BaseVelocity
	float			Rotation;			// 파티클의 회전 (라디안)

	// 16 bytes
	FVector			Velocity;			// 현재 속도, 각 프레임마다 BaseVelocity로 초기화됨
	float			BaseRotationRate;	// 파티클의 초기 각 속도 (라디안 / 초)

	// 16 bytes
	FVector			BaseSize;			// 각 프레임의 시작 시점에서 Size = BaseSize
	float			RotationRate;		// 현재 회전 비율, 각 프레임마다 BaseRotationRate로 초기화

	// 16 bytes
	FVector			Size;				// 현재 크기, 각 프레임마다 BaseSize로 초기화됨
	int32			Flags;				// 다양한 파티클 상태에 관한 플래그

	// 16 bytes
	FLinearColor	Color;				// 파티클의 현재 색상

	// 16 bytes
	FLinearColor	BaseColor;			// 파티클의 기본 색상

	float			RelativeTime;
	float			OneOverMaxLifeTime; // lifetime의 역수
	float			Placeholder0;
	float			Placeholder1;
};

/*-----------------------------------------------------------------------------
	Particle Dynamic Data
-----------------------------------------------------------------------------*/

enum EDynamicEmitterType
{
	DET_Unknown = 0,
	DET_Sprite,
	DET_Mesh,
	DET_Beam2,
	DET_Ribbon,
	DET_AnimTrail,
	DET_Custom
};

struct FParticleDataContainer
{
	int32 MemBlockSize;
	int32 ParticleDataNumBytes;
	int32 ParticleIndicesNumShorts;
	uint8* ParticleData; // 할당된 메모리
	uint16* ParticleIndices; // 직접 할당하지 않음, 메모리 블록의 마지막 (Alloc 참고)

	FParticleDataContainer()
		: MemBlockSize(0)
		, ParticleDataNumBytes(0)
		, ParticleIndicesNumShorts(0)
		, ParticleData(nullptr)
		, ParticleIndices(nullptr)
	{
	}
	~FParticleDataContainer()
	{
		Free();
	}
	void Alloc(int32 InParticleDataNumBytes, int32 InParticleIndicesNumShorts);
	void Free();
};

/** 모든 이미터 타입에 대한 데이터 베이스 클래스 */
struct FDynamicEmitterReplayDataBase
{
	/** 이미터 타입 */
	EDynamicEmitterType	eEmitterType;

	/** 이 이미터 내에서 현재 활성화된 파티클 수 */
	int32 ActiveParticleCount;

	int32 ParticleStride;
	FParticleDataContainer DataContainer;

	FVector Scale;

	/** Constructor */
	FDynamicEmitterReplayDataBase()
		: eEmitterType( DET_Unknown ),
		  ActiveParticleCount( 0 ),
		  ParticleStride( 0 ),
		  Scale( FVector( 1.0f ) )
	{
	}

	virtual ~FDynamicEmitterReplayDataBase()
	{
	}

	/** 직렬화 */
	// virtual void Serialize( FArchive& Ar );
};

/** 모든 이미터 타입에 대한 베이스 클래스 */
struct FDynamicEmitterDataBase
{
	FDynamicEmitterDataBase(const class UParticleModuleRequired* RequiredModule);

	virtual ~FDynamicEmitterDataBase()
	{
	}

	/** Custom new/delete with recycling */
	void* operator new(size_t Size);
	void operator delete(void *RawMemory, size_t Size);

	/**
	 *	Create the render thread resources for this emitter data
	 *
	 *	@param	InOwnerProxy	The proxy that owns this dynamic emitter data
	 */
	virtual void UpdateRenderThreadResourcesEmitter(const FParticleSystemSceneProxy* InOwnerProxy)
	{
	}

	/**
	 *	Release the render thread resources for this emitter data
	 *
	 *	@param	InOwnerProxy	The proxy that owns this dynamic emitter data
	 */
	virtual void ReleaseRenderThreadResources(const FParticleSystemSceneProxy* InOwnerProxy)
	{
	}

	virtual void GetDynamicMeshElementsEmitter(const FParticleSystemSceneProxy* Proxy, const FSceneView* View, const FSceneViewFamily& ViewFamily, int32 ViewIndex, FMeshElementCollector& Collector) const {}

	/**
	 *	Retrieve the material render proxy to use for rendering this emitter. PURE VIRTUAL
	 *
	 *	@param	bSelected				Whether the object is selected
	 *
	 *	@return	FMaterialRenderProxy*	The material proxt to render with.
	 */
	virtual const FMaterialRenderProxy* GetMaterialRenderProxy() = 0;

	/** Callback from the renderer to gather simple lights that this proxy wants renderered. */
	virtual void GatherSimpleLights(const FParticleSystemSceneProxy* Proxy, const FSceneViewFamily& ViewFamily, FSimpleLightArray& OutParticleLights) const {}

	/** Returns the source data for this particle system */
	virtual const FDynamicEmitterReplayDataBase& GetSource() const = 0;

	/** Returns the current macro uv override. Specialized by FGPUSpriteDynamicEmitterData  */
	virtual const FMacroUVOverride& GetMacroUVOverride() const { return GetSource().MacroUVOverride; }

	/** Stat id of this object, 0 if nobody asked for it yet */
	mutable TStatId StatID;
	/** true if this emitter is currently selected */
	uint32	bSelected:1;
	/** true if this emitter has valid rendering data */
	uint32	bValid:1;

	int32  EmitterIndex;
};

struct FDynamicSpriteEmitterReplayDataBase : public FDynamicEmitterReplayDataBase
{
	UMaterialInterface*				MaterialInterface;
	struct FParticleRequiredModule	*RequiredModule;
	FVector							NormalsSphereCenter;
	FVector							NormalsCylinderDirection;
	float							InvDeltaSeconds;
	FVector							LWCTile;
	int32							MaxDrawCount;
	int32							OrbitModuleOffset;
	int32							DynamicParameterDataOffset;
	int32							LightDataOffset;
	float							LightVolumetricScatteringIntensity;
	int32							CameraPayloadOffset;
	int32							SubUVDataOffset;
	int32							SubImages_Horizontal;
	int32							SubImages_Vertical;
	bool							bUseLocalSpace;
	bool							bLockAxis;
	uint8							ScreenAlignment;
	uint8							LockAxisFlag;
	uint8							EmitterRenderMode;
	uint8							EmitterNormalsMode;
	FVector2D						PivotOffset;
	bool							bUseVelocityForMotionBlur;
	bool							bRemoveHMDRoll;
	float							MinFacingCameraBlendDistance;
	float							MaxFacingCameraBlendDistance;

	/** 생성자 */
	FDynamicSpriteEmitterReplayDataBase();
	~FDynamicSpriteEmitterReplayDataBase();

	/** 직렬화 */
	// virtual void Serialize( FArchive& Ar );
};

/** 스프라이트 이미터와 다른 이미터 타입들을 위한 베이스 클래스 */
struct FDynamicSpriteEmitterDataBase : public FDynamicEmitterDataBase
{
	FDynamicSpriteEmitterDataBase(const UParticleModuleRequired* RequiredModule) :
		FDynamicEmitterDataBase( RequiredModule ),
		bUsesDynamicParameter(false)
	{
		// MaterialResource = nullptr;
	}

	virtual ~FDynamicSpriteEmitterDataBase()
	{
	}

	/** 정점 스트라이드를 반환한다. */
	virtual int32 GetDynamicParameterVertexStride() const
	{
		assert(0, "GetDynamicParameterVertexStride는 무조건 오버라이드되어야 합니다.");
		return 0;
	}

	/** 이 이미터에 대한 source replay data를 반환한다. */
	virtual const FDynamicSpriteEmitterReplayDataBase* GetSourceData() const
	{
		assert(0, "GetSourceData는 무조건 오버라이드되어야 합니다.");
		return nullptr;
	}

	/** 파티클 이미터가 DynamicParamter 모듈을 사용한다면 True */
	uint32 bUsesDynamicParameter:1;
};

struct FDynamicSpriteEmitterData : public FDynamicSpriteEmitterDataBase
{
	virtual int32 GetDynamicVertexStride(ERHIFeatureLevel::Type InFeatureLevel) const override
	{
		return sizeof(FParticleSpriteVertex);
	}

	...
};
