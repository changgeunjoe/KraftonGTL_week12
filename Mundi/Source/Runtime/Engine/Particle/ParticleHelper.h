#pragma once

#include "UEContainer.h"
#include "MeshBatchElement.h"
#include "ParticleEmitterInstances.h"
#include "SceneView.h"

/*-----------------------------------------------------------------------------
	Helper macros.
-----------------------------------------------------------------------------*/
#define DECLARE_PARTICLE(Name,Address)	\
FBaseParticle& Name = *((FBaseParticle*) (Address));

#define DECLARE_PARTICLE_CONST(Name,Address)		\
const FBaseParticle& Name = *((const FBaseParticle*) (Address));

#define DECLARE_PARTICLE_PTR(Name,Address)		\
FBaseParticle* Name = (FBaseParticle*) (Address);

#define SPAWN_INIT																									\
const int32		ActiveParticles	= Context.Owner.ActiveParticles;													\
const uint32	ParticleStride	= Context.Owner.ParticleStride;														\
uint32			CurrentOffset	= Context.Offset;																	\
FBaseParticle*	ParticleBase	= Context.ParticleBase;																\
FBaseParticle&	Particle		= *(ParticleBase);

#define BEGIN_UPDATE_LOOP																								\
{																													\
	int32&			ActiveParticles = Context.Owner.ActiveParticles;												\
	int32			Offset	= Context.Offset;																		\
	uint32			CurrentOffset	= Offset;																		\
	float			DeltaTime = Context.DeltaTime;																	\
	const uint8*	ParticleData	= Context.Owner.ParticleData;													\
	const uint32	ParticleStride	= Context.Owner.ParticleStride;													\
	uint16*			ParticleIndices	= Context.Owner.ParticleIndices;												\
	for(int32 i=ActiveParticles-1; i>=0; i--)																		\
	{																												\
		const int32	CurrentIndex	= ParticleIndices[i];															\
		const uint8* ParticleBase	= ParticleData + CurrentIndex * ParticleStride;									\
		FBaseParticle& Particle		= *((FBaseParticle*) ParticleBase);												\
		if ((Particle.Flags) == 0)															\
		{																											\

#define END_UPDATE_LOOP																									\
		}																											\
		CurrentOffset				= Offset;																		\
	}																												\
}

/*-----------------------------------------------------------------------------
	FBaseParticle
-----------------------------------------------------------------------------*/

class UParticleModuleRequired;

// 인스턴싱을 위한 정점 구조체
struct FParticleVertex
{
	FVector Position;
	FVector2D UV;
};

/**
 * GPU에 전달되는 파티클 당 데이터
 */
struct alignas(16) FParticleSpriteVertex
{
	/** 파티클의 위치 */
	FVector Position;
	/** 파티클의 상대시간 */
	float RelativeTime;
	/** 파티클의 이전 위치 */
	FVector	OldPosition;
	/** 파티클의 수명 동안 변하지 않는 값 */
	float ParticleId;
	/** 파티클의 크기 */
	FVector2D Size;
	/** 파티클의 회전 */
	float Rotation;

	float Pad;

	/** 파티클의 색 */
	FLinearColor Color;
	// Sub UV
	FVector2D UVOffset;
	FVector2D UVScale;
};

/**
 * GPU에 전달되는 파티클 당 데이터
 */
struct alignas(16) FMeshParticleInstanceVertex
{
	/** 파티클의 색 */
	FLinearColor Color;

	/** 파티클의 월드 변환 행렬 */
	FMatrix Transform;

	/** 파티클의 속도, XYZ: 방향, W: 속도. */
	FVector4 Velocity;

	/**
	 * 파티클의 서브 이미지 텍스쳐 오프셋
	 *
	 * SubUVParams[0]: 현재 프레임의 U 오프셋
	 * SubUVParams[1]: 현재 프레임의 V 오프셋
	 * SubUVParams[2]: 다음 프레임의 U 오프셋
	 * SubUVParams[3]: 다음 프레임의 V 오프셋
	 */
	int32 SubUVParams[4];

	/** 파티클의 서브 이미지 lerp 값 */
	float SubUVLerp;

	/** 파티클의 상대 시간 */
	float RelativeTime;

	float Pad[2];
};


struct FMeshRotationPayloadData
{
	FVector InitialOrientation;
	FVector InitRotation;
	FVector Rotation;
	FVector CurContinuousRotation;
	FVector RotationRate;
	FVector RotationRateBase;
};

/*-----------------------------------------------------------------------------
	Beam Particle Data
-----------------------------------------------------------------------------*/

/** 빔 세그먼트 데이터 */
struct FBeamSegment
{
	FVector Position;    // 세그먼트 위치
	FVector Tangent;     // 방향 벡터
	float Width;         // 해당 위치에서의 너비
	float TexCoord;      // UV 좌표 (0~1 또는 타일링)
};

/** 빔 파티클 페이로드 */
struct FBeamParticlePayloadData
{
	FVector SourcePoint;           // 시작점
	FVector TargetPoint;           // 끝점
	FVector SourceTangent;         // 시작 방향
	FVector TargetTangent;         // 끝 방향

	int32 SegmentCount;            // 세그먼트 수
	float BeamLength;              // 빔 전체 길이

	// 노이즈 관련
	float NoiseSeed;               // 노이즈 시드 (일관된 노이즈를 위해)
	float NoiseTime;               // 노이즈 애니메이션 시간
};

/** GPU에 전달되는 빔 파티클 데이터 (셰이더의 FBeamParticleData와 일치) */
struct alignas(16) FBeamParticleVertex
{
	/** 시작점 */
	FVector SourcePoint;
	/** 너비 */
	float Width;

	/** 끝점 */
	FVector TargetPoint;
	/** 텍스처 타일링 횟수 */
	float TextureTile;

	/** 색상 */
	FLinearColor Color;
};

struct FBaseParticle
{
	// 16 bytes
	FVector		OldLocation;		// 지난 프레임의 위치 (충돌을 위해 사용됨)
	float		Placeholder0;

	// 16 bytes
	FVector		Location;			// 현재 위치
	float		Placeholder1;

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

	// 16 bytes
	float			RelativeTime;
	float			OneOverMaxLifetime; // lifetime의 역수
	float			SubImageIndex;
	float			Placeholder2;
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

// ===== Dynamic Emitter Replay Data =====

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
};

struct FDynamicSpriteEmitterReplayDataBase : public FDynamicEmitterReplayDataBase
{
	UMaterialInterface*				MaterialInterface;
	UParticleModuleRequired*		RequiredModule;
	bool							bUseLocalSpace;
	uint8							ScreenAlignment;

	uint32 SubImages_Horizontal = 1;
	uint32 SubImages_Vertical = 1;

	FDynamicSpriteEmitterReplayDataBase();

	virtual ~FDynamicSpriteEmitterReplayDataBase() = default;
};

struct FDynamicSpriteEmitterReplayData : public FDynamicSpriteEmitterReplayDataBase
{
	FDynamicSpriteEmitterReplayData()
	{
	}
};

struct FDynamicMeshEmitterReplayData : public FDynamicSpriteEmitterReplayDataBase
{
	int32 MeshRotationOffset;

	FDynamicMeshEmitterReplayData()
	{
		eEmitterType = DET_Mesh;
	}
};

struct FDynamicBeamEmitterReplayData : public FDynamicSpriteEmitterReplayDataBase
{
	/** 빔 페이로드 오프셋 */
	int32 BeamPayloadOffset;

	/** 세그먼트 수 */
	int32 MaxSegments;

	/** 텍스처 타일링 횟수 */
	float TextureTile;

	/** 세그먼트 데이터 배열 */
	TArray<FBeamSegment> SegmentData;

	FDynamicBeamEmitterReplayData()
		: BeamPayloadOffset(0)
		, MaxSegments(0)
		, TextureTile(1.0f)
	{
		eEmitterType = DET_Beam2;
	}
};

// ===== Dynamic Emitter Data =====

/** 모든 이미터 타입에 대한 베이스 클래스 */
struct FDynamicEmitterDataBase
{
	FDynamicEmitterDataBase();

	virtual ~FDynamicEmitterDataBase() = default;

	/** 이 파티클 시스템의 소스 데이터를 반환한다. */
	virtual const FDynamicEmitterReplayDataBase& GetSource() const = 0;

	virtual void GetDynamicMeshElementsEmitter(TArray<FMeshBatchElement>& Collector, const FSceneView* View) {}

	uint32	bSelected:1;

	int32  EmitterIndex;
};

/** 스프라이트 이미터와 다른 이미터 타입들을 위한 베이스 클래스 */
struct FDynamicSpriteEmitterDataBase : public FDynamicEmitterDataBase
{
	FDynamicSpriteEmitterDataBase();

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

	// GPU 파티클 데이터를 저장할 구조화 버퍼입니다.
	ID3D11Buffer* ParticleStructuredBuffer = nullptr;
	ID3D11ShaderResourceView* ParticleStructuredBufferSRV = nullptr;

	uint32 ParticleStructuredBufferSize = 0;

protected:
	/** 파티클 구조화 버퍼와 SRV를 생성합니다. */
	void CreateParticleStructuredBuffer(uint32 Stride, uint32 NumElements);
};

struct FDynamicSpriteEmitterData : public FDynamicSpriteEmitterDataBase
{
	FDynamicSpriteEmitterData();

	virtual ~FDynamicSpriteEmitterData();

	void Init(bool bInSelected);

	virtual int32 GetDynamicParameterVertexStride() const override
	{
		return sizeof(FParticleSpriteVertex);
	}

	virtual const FDynamicSpriteEmitterReplayDataBase& GetSource() const override
	{
		return Source;
	}

	virtual const FDynamicSpriteEmitterReplayDataBase* GetSourceData() const override
	{
		return &Source;
	}

	virtual void GetDynamicMeshElementsEmitter(TArray<FMeshBatchElement>& Collector, const FSceneView* View) override;

	FDynamicSpriteEmitterReplayData Source;

	// GPU 인스턴싱을 위한 정적 정점/인덱스 버퍼입니다.
	ID3D11Buffer* VertexBuffer = nullptr;
	ID3D11Buffer* IndexBuffer = nullptr;
};

struct FDynamicMeshEmitterData : public FDynamicSpriteEmitterDataBase
{
	FDynamicMeshEmitterData();

	virtual ~FDynamicMeshEmitterData() = default;

	void Init(	bool bInSelected,
				const FParticleMeshEmitterInstance* InEmitterInstance,
				UStaticMesh* InStaticMesh,
				bool InUseStaticMeshLODs,
				float InLODSizeScale);

	virtual int32 GetDynamicParameterVertexStride() const override
	{
		return sizeof(FMeshParticleInstanceVertex);
	}

	virtual const FDynamicSpriteEmitterReplayDataBase& GetSource() const override
	{
		return Source;
	}

	virtual const FDynamicSpriteEmitterReplayDataBase* GetSourceData() const override
	{
		return &Source;
	}

	virtual void GetDynamicMeshElementsEmitter(TArray<FMeshBatchElement>& Collector, const FSceneView* View) override;

	FDynamicMeshEmitterReplayData Source;

	UStaticMesh* StaticMesh;
};

struct FParticleBeamEmitterInstance;

struct FDynamicBeamEmitterData : public FDynamicSpriteEmitterDataBase
{
	FDynamicBeamEmitterData();

	virtual ~FDynamicBeamEmitterData();

	void Init(bool bInSelected, const FParticleBeamEmitterInstance* InEmitterInstance);

	virtual int32 GetDynamicParameterVertexStride() const override
	{
		return sizeof(FBeamParticleVertex);
	}

	virtual const FDynamicSpriteEmitterReplayDataBase& GetSource() const override
	{
		return Source;
	}

	virtual const FDynamicSpriteEmitterReplayDataBase* GetSourceData() const override
	{
		return &Source;
	}

	virtual void GetDynamicMeshElementsEmitter(TArray<FMeshBatchElement>& Collector, const FSceneView* View) override;

	FDynamicBeamEmitterReplayData Source;

	/** 빔 정점 버퍼 (현재 미사용) */
	ID3D11Buffer* BeamVertexBuffer = nullptr;

	/** 빔 인덱스 버퍼 (현재 미사용) */
	ID3D11Buffer* BeamIndexBuffer = nullptr;

	/** 캐시된 세그먼트 수 (현재 미사용) */
	int32 CachedSegmentCount = 0;

	// 스프라이트와 동일한 방식을 위한 버퍼
	ID3D11Buffer* VertexBuffer = nullptr;
	ID3D11Buffer* IndexBuffer = nullptr;
};
