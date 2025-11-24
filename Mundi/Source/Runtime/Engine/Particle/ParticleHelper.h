#pragma once

#include "UEContainer.h"
#include "MeshBatchElement.h"
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

/*-----------------------------------------------------------------------------
	FBaseParticle
-----------------------------------------------------------------------------*/

class UParticleModuleRequired;

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
	float			Placeholder2;
	float			Placeholder3;
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

	FDynamicSpriteEmitterReplayDataBase();

	virtual ~FDynamicSpriteEmitterReplayDataBase() = default;
};

struct FDynamicSpriteEmitterReplayData : public FDynamicSpriteEmitterReplayDataBase
{
	FDynamicSpriteEmitterReplayData()
	{
	}
};

/** 모든 이미터 타입에 대한 베이스 클래스 */
struct FDynamicEmitterDataBase
{
	FDynamicEmitterDataBase(const class UParticleModuleRequired* RequiredModule);

	virtual ~FDynamicEmitterDataBase() = default;

	/** 이 파티클 시스템의 소스 데이터를 반환한다. */
	virtual const FDynamicEmitterReplayDataBase& GetSource() const = 0;

	virtual void GetDynamicMeshElementsEmitter(TArray<FMeshBatchElement>& Collector, const FSceneView* View) const {}

	uint32	bSelected:1;

	int32  EmitterIndex;
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
	FDynamicSpriteEmitterData(const UParticleModuleRequired* RequiredModule);

	virtual ~FDynamicSpriteEmitterData();

	void Init(bool bInSelected);

	virtual int32 GetDynamicParameterVertexStride() const
	{
		return 0;
	}

	virtual const FDynamicSpriteEmitterReplayDataBase& GetSource() const override
	{
		return Source;
	}

	virtual const FDynamicSpriteEmitterReplayDataBase* GetSourceData() const override
	{
		return &Source;
	}

	virtual void GetDynamicMeshElementsEmitter(TArray<FMeshBatchElement>& Collector, const FSceneView* View) const override;

	FDynamicSpriteEmitterReplayData Source;

	// GPU에 바인딩될 정점 버퍼입니다.
	ID3D11Buffer* VertexBuffer = nullptr;

	// GPU에 바인딩될 인덱스 버퍼입니다.
	ID3D11Buffer* IndexBuffer = nullptr;
};
