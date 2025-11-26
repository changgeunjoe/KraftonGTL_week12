#include "pch.h"
#include "ParticleHelper.h"

#include "ParticleSpriteEmitter.h"

constexpr uint32 InitMaxParticles = 128u;

FDynamicEmitterDataBase::FDynamicEmitterDataBase()
	: bSelected(false)
	, EmitterIndex(-1)
{
}

FDynamicSpriteEmitterDataBase::FDynamicSpriteEmitterDataBase() :
	bUsesDynamicParameter(false)
{
}

void FDynamicSpriteEmitterDataBase::CreateParticleStructuredBuffer(uint32 Stride, uint32 NumElements)
{
	ID3D11Device* Device = GEngine.GetRHIDevice()->GetDevice();
	assert(Device);

	// Release existing resources first
	if (ParticleStructuredBuffer) ParticleStructuredBuffer->Release();
	if (ParticleStructuredBufferSRV) ParticleStructuredBufferSRV->Release();

	// Create the structured buffer
	D3D11_BUFFER_DESC Sbd{};
	Sbd.Usage = D3D11_USAGE_DYNAMIC;
	Sbd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	Sbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Sbd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	Sbd.StructureByteStride = Stride;
	Sbd.ByteWidth = static_cast<UINT>(Stride * NumElements);

	HRESULT Hr = Device->CreateBuffer(&Sbd, nullptr, &ParticleStructuredBuffer);
	assert(SUCCEEDED(Hr) && "Failed to create particle structured buffer");

	// Create the shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC Srvd{};
	Srvd.Format = DXGI_FORMAT_UNKNOWN;
	Srvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	Srvd.Buffer.FirstElement = 0;
	Srvd.Buffer.NumElements = NumElements;

	Hr = Device->CreateShaderResourceView(ParticleStructuredBuffer, &Srvd, &ParticleStructuredBufferSRV);
	assert(SUCCEEDED(Hr) && "Failed to create particle structured buffer SRV");
}


FDynamicSpriteEmitterData::FDynamicSpriteEmitterData()
{
	// 초기 최대 파티클 수 설정 (런타임에 동적으로 2배씩 커짐)
	ParticleStructuredBufferSize = InitMaxParticles;

	// 파티클별 데이터를 위한 동적 구조화 버퍼
	CreateParticleStructuredBuffer(sizeof(FParticleSpriteVertex), ParticleStructuredBufferSize);

	ID3D11Device* Device = GEngine.GetRHIDevice()->GetDevice();
	if (!Device) return;

	// 인스턴싱을 위한 정적 정점 버퍼 (단일 쿼드)
	if (!VertexBuffer)
	{
		FParticleVertex QuadVertices[4];
		QuadVertices[0] = { FVector(-0.5f, -0.5f, 0.0f), FVector2D(0.0f, 1.0f) }; // LB
		QuadVertices[1] = { FVector(-0.5f,  0.5f, 0.0f), FVector2D(0.0f, 0.0f) }; // LT
		QuadVertices[2] = { FVector(0.5f,  0.5f, 0.0f), FVector2D(1.0f, 0.0f) }; // RT
		QuadVertices[3] = { FVector(0.5f, -0.5f, 0.0f), FVector2D(1.0f, 1.0f) }; // RB

		D3D11_BUFFER_DESC Vbd{};
		Vbd.Usage = D3D11_USAGE_DEFAULT;
		Vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		Vbd.ByteWidth = sizeof(FParticleVertex) * 4;

		D3D11_SUBRESOURCE_DATA Srd{};
		Srd.pSysMem = QuadVertices;

		HRESULT Hr = Device->CreateBuffer(&Vbd, &Srd, &VertexBuffer);
		assert(SUCCEEDED(Hr) && "Failed to create particle instancing vertex buffer");
	}

	// 인스턴싱을 위한 정적 인덱스 버퍼 (단일 쿼드)
	if (!IndexBuffer)
	{
		uint32 Indices[] = { 0, 1, 2, 0, 2, 3 };

		D3D11_BUFFER_DESC Ibd{};
		Ibd.Usage = D3D11_USAGE_DEFAULT;
		Ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		Ibd.ByteWidth = sizeof(uint32) * 6;

		D3D11_SUBRESOURCE_DATA Srd{};
		Srd.pSysMem = Indices;

		HRESULT Hr = Device->CreateBuffer(&Ibd, &Srd, &IndexBuffer);
		assert(SUCCEEDED(Hr) && "Failed to create particle instancing index buffer");
	}
}

FDynamicSpriteEmitterData::~FDynamicSpriteEmitterData()
{
	if (ParticleStructuredBuffer)
	{
		ParticleStructuredBuffer->Release();
		ParticleStructuredBuffer = nullptr;
	}
	if (ParticleStructuredBufferSRV)
	{
		ParticleStructuredBufferSRV->Release();
		ParticleStructuredBufferSRV = nullptr;
	}
	if (VertexBuffer)
	{
		VertexBuffer->Release();
		VertexBuffer = nullptr;
	}
	if (IndexBuffer)
	{
		IndexBuffer->Release();
		IndexBuffer = nullptr;
	}
}

void FDynamicSpriteEmitterData::Init(bool bInSelected)
{
	bSelected = bInSelected;
}

void FDynamicSpriteEmitterData::GetDynamicMeshElementsEmitter(TArray<FMeshBatchElement>& Collector, const FSceneView* View)
{
	const FDynamicSpriteEmitterReplayDataBase* Src = GetSourceData();
	if (!Src) return;

	const int32 ParticleCount = static_cast<uint32>(Src->ActiveParticleCount);

	// 동적으로 ParticleStructuredBufferSize 크기를 2배씩 늘림
	if (ParticleCount > ParticleStructuredBufferSize)
	{
		uint32 NewSize = ParticleStructuredBufferSize;
		while (NewSize < ParticleCount)
		{
			NewSize = FMath::Max(NewSize * 2, InitMaxParticles);
		}
		ParticleStructuredBufferSize = NewSize;

		CreateParticleStructuredBuffer(sizeof(FParticleSpriteVertex), NewSize);
	}

	if (ParticleCount <= 0) return;
	if (!Src->DataContainer.ParticleData) return;
	if (!ParticleStructuredBuffer || !VertexBuffer || !IndexBuffer) return;

	// 1. 파티클 데이터를 구조화 버퍼에 업로드
	ID3D11DeviceContext* Context = GEngine.GetRHIDevice()->GetDeviceContext();
	if (Context)
	{
		D3D11_MAPPED_SUBRESOURCE Mapped{};
		HRESULT Hr = Context->Map(ParticleStructuredBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped);
		if (SUCCEEDED(Hr))
		{
			const int32 Stride = Src->ParticleStride;
			uint8* BasePtr = Src->DataContainer.ParticleData;
			const uint16* Indices = Src->DataContainer.ParticleIndices;
			bool bHasIndices = (Indices != nullptr);

			TArray<FParticleSpriteVertex> GpuParticles;
			GpuParticles.SetNum(ParticleCount);
			FVector2D UVScale = FVector2D(1.0f / Src->SubImages_Horizontal, 1.0f / Src->SubImages_Vertical);

			for (int32 i = 0; i < ParticleCount; ++i)
			{
				int32 SrcIndex = bHasIndices ? Indices[i] : i;
				FBaseParticle* BaseParticle = reinterpret_cast<FBaseParticle*>(BasePtr + SrcIndex * Stride);
				if (!BaseParticle) continue;

				GpuParticles[i].Position = BaseParticle->Location;
				GpuParticles[i].Rotation = BaseParticle->Rotation;
				GpuParticles[i].Size = FVector2D(BaseParticle->Size.X, BaseParticle->Size.Y);
				GpuParticles[i].Color = BaseParticle->Color;

				uint32 TotalCount = Src->SubImages_Horizontal * Src->SubImages_Vertical;

				uint32 Horizontal = (int)((float)TotalCount * BaseParticle->SubImageIndex) % Src->SubImages_Horizontal;
				uint32 Vertical = (TotalCount * BaseParticle->SubImageIndex) / Src->SubImages_Vertical;

				GpuParticles[i].UVScale = UVScale;
				GpuParticles[i].UVOffset = FVector2D(Horizontal * UVScale.X, Vertical * UVScale.Y);
			}

			// 카메라 기준 Z(depth)로 내림차순 정렬 (멀리 있는 것부터 가까운 것)
			GpuParticles.Sort([&](const FParticleSpriteVertex& A, const FParticleSpriteVertex& B)
				{
					float DistA = (A.Position - View->ViewLocation).SizeSquared();
					float DistB = (B.Position - View->ViewLocation).SizeSquared();
					return DistA > DistB; // Back-to-Front
				});

			memcpy(Mapped.pData, GpuParticles.GetData(), sizeof(FParticleSpriteVertex) * ParticleCount);
			Context->Unmap(ParticleStructuredBuffer, 0);
		}
	}

	// 2. 인스턴싱 드로우 콜을 위한 배치 엘리먼트 생성
	FMeshBatchElement BatchElement;

	UShader* ParticleShader = UResourceManager::GetInstance().Load<UShader>("Shaders/Particles/ParticleSprite.hlsl");

	if (ParticleShader)
	{
		BatchElement.VertexShader = ParticleShader->GetVertexShader();
		BatchElement.PixelShader = ParticleShader->GetPixelShader();
		BatchElement.InputLayout = ParticleShader->GetInputLayout();
	}

	// 재질 또는 텍스처 설정
	BatchElement.Material = Src->MaterialInterface;
	if (!BatchElement.Material)
	{
		BatchElement.InstanceShaderResourceView = UResourceManager::GetInstance().Load<UTexture>("Data/Textures/FakeLight.png")->GetShaderResourceView();
	}

	BatchElement.PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// 인스턴싱 설정
	BatchElement.VertexBuffer = VertexBuffer;
	BatchElement.IndexBuffer = IndexBuffer;
	BatchElement.VertexStride = sizeof(FParticleVertex);
	BatchElement.IndexCount = 6;
	BatchElement.bIsInstanced = true;
	BatchElement.InstanceCount = ParticleCount;

	// 파티클 데이터 설정
	BatchElement.ParticleDataSRV = ParticleStructuredBufferSRV;
	BatchElement.bIsParticle = true;

	BatchElement.WorldMatrix = FMatrix::Identity();
	BatchElement.ObjectID = 0;

	Collector.Add(BatchElement);
}

FDynamicMeshEmitterData::FDynamicMeshEmitterData()
	: StaticMesh(nullptr)
{
	// 초기 최대 파티클 수 설정 (런타임에 동적으로 2배씩 커짐)
	ParticleStructuredBufferSize = InitMaxParticles;

	// 파티클별 데이터를 위한 동적 구조화 버퍼
	CreateParticleStructuredBuffer(sizeof(FMeshParticleInstanceVertex), ParticleStructuredBufferSize);
}

void FDynamicMeshEmitterData::Init(bool bInSelected, const FParticleMeshEmitterInstance* InEmitterInstance, UStaticMesh* InStaticMesh, bool InUseStaticMeshLODs, float InLODSizeScale)
{
	bSelected = bInSelected;

	StaticMesh = InStaticMesh;
	assert(StaticMesh);

	// @todo
}

void FDynamicMeshEmitterData::GetDynamicMeshElementsEmitter(TArray<FMeshBatchElement>& Collector, const FSceneView* View)
{
	const FDynamicMeshEmitterReplayData* Src = static_cast<const FDynamicMeshEmitterReplayData*>(GetSourceData());
	if (!Src) return;

	const uint32 ParticleCount = static_cast<uint32>(Src->ActiveParticleCount);

	// 동적으로 ParticleStructuredBufferSize 크기를 2배씩 늘림
	if (ParticleCount > ParticleStructuredBufferSize)
	{
		uint32 NewSize = ParticleStructuredBufferSize;
		while (NewSize < ParticleCount)
		{
			NewSize = FMath::Max(NewSize * 2, InitMaxParticles);
		}
		ParticleStructuredBufferSize = NewSize;

		CreateParticleStructuredBuffer(sizeof(FMeshParticleInstanceVertex), NewSize);
	}

	if (ParticleCount <= 0) return;
	if (!Src->DataContainer.ParticleData) return;
	if (!ParticleStructuredBuffer || !StaticMesh) return;

	// 1. 파티클 데이터를 구조화 버퍼에 업로드
	ID3D11DeviceContext* Context = GEngine.GetRHIDevice()->GetDeviceContext();
	if (Context)
	{
		D3D11_MAPPED_SUBRESOURCE Mapped{};
		HRESULT Hr = Context->Map(ParticleStructuredBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped);
		if (SUCCEEDED(Hr))
		{
			const int32 Stride = Src->ParticleStride;
			uint8* BasePtr = Src->DataContainer.ParticleData;
			const uint16* Indices = Src->DataContainer.ParticleIndices;
			bool bHasIndices = (Indices != nullptr);

			TArray<FMeshParticleInstanceVertex> GpuParticles;
			GpuParticles.SetNum(ParticleCount);

			for (int32 i = 0; i < ParticleCount; ++i)
			{
				int32 SrcIndex = bHasIndices ? Indices[i] : i;
				FBaseParticle* BaseParticle = reinterpret_cast<FBaseParticle*>(BasePtr + SrcIndex * Stride);
				FMeshRotationPayloadData* RotationPayload = reinterpret_cast<FMeshRotationPayloadData*>(BasePtr + SrcIndex * Stride + Src->MeshRotationOffset);
				if (!BaseParticle) continue;

				FTransform Transform(BaseParticle->Location, FQuat::MakeFromEulerZYX(RotationPayload->Rotation), BaseParticle->Size);
				GpuParticles[i].Transform = Transform.ToMatrix();
				GpuParticles[i].Color = BaseParticle->Color;
			}

			// 카메라 기준 Z(depth)로 내림차순 정렬 (멀리 있는 것부터 가까운 것)
			GpuParticles.Sort([&](const FMeshParticleInstanceVertex& A, const FMeshParticleInstanceVertex& B)
				{
					float DistA = (FVector(A.Transform.M[3][0], A.Transform.M[3][1], A.Transform.M[3][2]) - View->ViewLocation).SizeSquared();
					float DistB = (FVector(B.Transform.M[3][0], B.Transform.M[3][1], B.Transform.M[3][2]) - View->ViewLocation).SizeSquared();
					return DistA > DistB; // Back-to-Front
				});

			memcpy(Mapped.pData, GpuParticles.GetData(), sizeof(FMeshParticleInstanceVertex) * ParticleCount);
			Context->Unmap(ParticleStructuredBuffer, 0);
		}
	}

	// 2. 인스턴싱 드로우 콜을 위한 배치 엘리먼트 생성
	FMeshBatchElement BatchElement;

	UShader* ParticleShader = UResourceManager::GetInstance().Load<UShader>("Shaders/Particles/ParticleMesh.hlsl");

	if (ParticleShader)
	{
		BatchElement.VertexShader = ParticleShader->GetVertexShader();
		BatchElement.PixelShader = ParticleShader->GetPixelShader();
		BatchElement.InputLayout = ParticleShader->GetInputLayout();
	}

	// 재질 또는 텍스처 설정
	BatchElement.Material = Src->MaterialInterface;
	if (!BatchElement.Material)
	{
		BatchElement.InstanceShaderResourceView = UResourceManager::GetInstance().Load<UTexture>("Data/Model/cube_texture.png")->GetShaderResourceView();
	}

	BatchElement.PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// 인스턴싱 설정
	BatchElement.VertexBuffer = StaticMesh->GetVertexBuffer();
	BatchElement.IndexBuffer = StaticMesh->GetIndexBuffer();
	BatchElement.VertexStride = StaticMesh->GetVertexStride();
	BatchElement.IndexCount = StaticMesh->GetIndexCount();
	BatchElement.bIsInstanced = true;
	BatchElement.InstanceCount = ParticleCount;

	// 파티클 데이터 설정
	BatchElement.ParticleDataSRV = ParticleStructuredBufferSRV;
	BatchElement.bIsParticle = true;

	BatchElement.WorldMatrix = FMatrix::Identity();
	BatchElement.ObjectID = 0;

	Collector.Add(BatchElement);
}

void FParticleDataContainer::Alloc(int32 InParticleDataNumBytes, int32 InParticleIndicesNumShorts)
{
	Free();

	ParticleDataNumBytes = InParticleDataNumBytes;
	ParticleIndicesNumShorts = InParticleIndicesNumShorts;

	if ((ParticleDataNumBytes > 0) && (ParticleIndicesNumShorts > 0))
	{
		MemBlockSize = ParticleDataNumBytes + (ParticleIndicesNumShorts * sizeof(uint16));

		ParticleData = (uint8*)std::malloc(MemBlockSize);
		ParticleIndices = (uint16*)(ParticleData + ParticleDataNumBytes);
	}
}

void FParticleDataContainer::Free()
{
	if (ParticleData)
	{
		assert(MemBlockSize > 0);
		std::free(ParticleData);
	}
	MemBlockSize = 0;
	ParticleDataNumBytes = 0;
	ParticleIndicesNumShorts = 0;
	ParticleData = nullptr;
	ParticleIndices = nullptr;
}

FDynamicSpriteEmitterReplayDataBase::FDynamicSpriteEmitterReplayDataBase()
	: MaterialInterface(nullptr)
	, bUseLocalSpace(false)
	, ScreenAlignment(PSA_Square)
{
}

/*-----------------------------------------------------------------------------
	FDynamicBeamEmitterData
-----------------------------------------------------------------------------*/

FDynamicBeamEmitterData::FDynamicBeamEmitterData()
	: BeamVertexBuffer(nullptr)
	, BeamIndexBuffer(nullptr)
{
	// 빔용 초기화
	ParticleStructuredBufferSize = 128u;

	// 빔 파티클별 데이터를 위한 동적 구조화 버퍼
	CreateParticleStructuredBuffer(sizeof(FBeamParticleVertex), ParticleStructuredBufferSize);

	ID3D11Device* Device = GEngine.GetRHIDevice()->GetDevice();
	if (!Device) return;

	// 세그먼트 분할된 빔 메시 생성
	// NumSegments개의 세그먼트 = (NumSegments+1)개의 정점 라인
	const int32 NumSegments = 10;
	const int32 NumVertices = (NumSegments + 1) * 2;  // 각 라인당 상/하 2개
	const int32 NumIndices = NumSegments * 6;  // 각 세그먼트당 2개 삼각형

	if (!VertexBuffer)
	{
		TArray<FParticleVertex> Vertices;
		Vertices.SetNum(NumVertices);

		for (int32 i = 0; i <= NumSegments; i++)
		{
			float t = (float)i / (float)NumSegments;  // 0 ~ 1
			float x = t - 0.5f;  // -0.5 ~ 0.5

			// 상단 정점
			Vertices[i * 2 + 0] = { FVector(x, 0.5f, 0.0f), FVector2D(t, 0.0f) };
			// 하단 정점
			Vertices[i * 2 + 1] = { FVector(x, -0.5f, 0.0f), FVector2D(t, 1.0f) };
		}

		D3D11_BUFFER_DESC Vbd{};
		Vbd.Usage = D3D11_USAGE_DEFAULT;
		Vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		Vbd.ByteWidth = sizeof(FParticleVertex) * NumVertices;

		D3D11_SUBRESOURCE_DATA Srd{};
		Srd.pSysMem = Vertices.GetData();

		HRESULT Hr = Device->CreateBuffer(&Vbd, &Srd, &VertexBuffer);
		assert(SUCCEEDED(Hr) && "Failed to create beam vertex buffer");
	}

	if (!IndexBuffer)
	{
		TArray<uint32> Indices;
		Indices.SetNum(NumIndices);

		for (int32 i = 0; i < NumSegments; i++)
		{
			int32 topLeft = i * 2;
			int32 bottomLeft = i * 2 + 1;
			int32 topRight = (i + 1) * 2;
			int32 bottomRight = (i + 1) * 2 + 1;

			int32 idx = i * 6;
			// 첫 번째 삼각형 (시계 방향 CW)
			Indices[idx + 0] = topLeft;
			Indices[idx + 1] = topRight;
			Indices[idx + 2] = bottomLeft;
			// 두 번째 삼각형 (시계 방향 CW)
			Indices[idx + 3] = bottomLeft;
			Indices[idx + 4] = topRight;
			Indices[idx + 5] = bottomRight;
		}

		D3D11_BUFFER_DESC Ibd{};
		Ibd.Usage = D3D11_USAGE_DEFAULT;
		Ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		Ibd.ByteWidth = sizeof(uint32) * NumIndices;

		D3D11_SUBRESOURCE_DATA Srd{};
		Srd.pSysMem = Indices.GetData();

		HRESULT Hr = Device->CreateBuffer(&Ibd, &Srd, &IndexBuffer);
		assert(SUCCEEDED(Hr) && "Failed to create beam index buffer");
	}

	// 인덱스 카운트 저장
	CachedSegmentCount = NumIndices;
}

FDynamicBeamEmitterData::~FDynamicBeamEmitterData()
{
	if (ParticleStructuredBuffer)
	{
		ParticleStructuredBuffer->Release();
		ParticleStructuredBuffer = nullptr;
	}
	if (ParticleStructuredBufferSRV)
	{
		ParticleStructuredBufferSRV->Release();
		ParticleStructuredBufferSRV = nullptr;
	}
	if (BeamVertexBuffer)
	{
		BeamVertexBuffer->Release();
		BeamVertexBuffer = nullptr;
	}
	if (BeamIndexBuffer)
	{
		BeamIndexBuffer->Release();
		BeamIndexBuffer = nullptr;
	}
	if (VertexBuffer)
	{
		VertexBuffer->Release();
		VertexBuffer = nullptr;
	}
	if (IndexBuffer)
	{
		IndexBuffer->Release();
		IndexBuffer = nullptr;
	}
}

void FDynamicBeamEmitterData::Init(bool bInSelected, const FParticleBeamEmitterInstance* InEmitterInstance)
{
	bSelected = bInSelected;
}

void FDynamicBeamEmitterData::GetDynamicMeshElementsEmitter(TArray<FMeshBatchElement>& Collector, const FSceneView* View)
{
	// 노이즈 애니메이션용 시간 (정적 변수로 누적)
	static float NoiseTime = 0.0f;
	NoiseTime += 0.016f;  // 약 60fps 기준

	const FDynamicBeamEmitterReplayData* Src = static_cast<const FDynamicBeamEmitterReplayData*>(GetSourceData());
	if (!Src) return;

	const int32 ParticleCount = static_cast<uint32>(Src->ActiveParticleCount);

	// 동적으로 ParticleStructuredBufferSize 크기를 2배씩 늘림
	if (ParticleCount > ParticleStructuredBufferSize)
	{
		uint32 NewSize = ParticleStructuredBufferSize;
		while (NewSize < ParticleCount)
		{
			NewSize = FMath::Max(NewSize * 2, 128u);
		}
		ParticleStructuredBufferSize = NewSize;

		CreateParticleStructuredBuffer(sizeof(FBeamParticleVertex), NewSize);
	}

	if (ParticleCount <= 0) return;
	if (!Src->DataContainer.ParticleData) return;
	if (!ParticleStructuredBuffer || !VertexBuffer || !IndexBuffer) return;

	// 1. 빔 데이터를 구조화 버퍼에 업로드
	ID3D11DeviceContext* Context = GEngine.GetRHIDevice()->GetDeviceContext();
	if (Context)
	{
		D3D11_MAPPED_SUBRESOURCE Mapped{};
		HRESULT Hr = Context->Map(ParticleStructuredBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped);
		if (SUCCEEDED(Hr))
		{
			const int32 Stride = Src->ParticleStride;
			uint8* BasePtr = Src->DataContainer.ParticleData;
			const uint16* Indices = Src->DataContainer.ParticleIndices;
			bool bHasIndices = (Indices != nullptr);

			TArray<FBeamParticleVertex> GpuBeams;
			GpuBeams.SetNum(ParticleCount);

			for (int32 i = 0; i < ParticleCount; ++i)
			{
				int32 SrcIndex = bHasIndices ? Indices[i] : i;
				FBaseParticle* BaseParticle = reinterpret_cast<FBaseParticle*>(BasePtr + SrcIndex * Stride);
				FBeamParticlePayloadData* BeamPayload = reinterpret_cast<FBeamParticlePayloadData*>(
					BasePtr + SrcIndex * Stride + Src->BeamPayloadOffset);
				if (!BaseParticle) continue;

				// 빔 데이터 설정
				GpuBeams[i].SourcePoint = BeamPayload->SourcePoint;
				GpuBeams[i].TargetPoint = BeamPayload->TargetPoint;
				GpuBeams[i].Width = BaseParticle->Size.X;  // Size.X를 너비로 사용
				GpuBeams[i].TextureTile = Src->TextureTile;  // 텍스처 타일링
				GpuBeams[i].Color = BaseParticle->Color;
				GpuBeams[i].SourceTaper = Src->SourceTaperScale;
				GpuBeams[i].TargetTaper = Src->TargetTaperScale;
				GpuBeams[i].NoiseStrength = Src->NoiseStrength;
				GpuBeams[i].NoiseTime = NoiseTime;
				GpuBeams[i].TextureScrollSpeed = Src->TextureScrollSpeed;
				GpuBeams[i].PulseSpeed = Src->PulseSpeed;
				GpuBeams[i].PulseScale = Src->PulseScale;
				GpuBeams[i].NoiseOctaves = Src->NoiseOctaves;
			}

			memcpy(Mapped.pData, GpuBeams.GetData(), sizeof(FBeamParticleVertex) * ParticleCount);
			Context->Unmap(ParticleStructuredBuffer, 0);
		}
	}

	// 2. 인스턴싱 드로우 콜을 위한 배치 엘리먼트 생성
	FMeshBatchElement BatchElement;

	UShader* BeamShader = UResourceManager::GetInstance().Load<UShader>("Shaders/Particles/ParticleBeam.hlsl");

	if (BeamShader)
	{
		BatchElement.VertexShader = BeamShader->GetVertexShader();
		BatchElement.PixelShader = BeamShader->GetPixelShader();
		BatchElement.InputLayout = BeamShader->GetInputLayout();
	}

	// 재질 또는 텍스처 설정
	BatchElement.Material = Src->MaterialInterface;
	if (!BatchElement.Material)
	{
		BatchElement.InstanceShaderResourceView = UResourceManager::GetInstance().Load<UTexture>("Data/Textures/FakeLight.png")->GetShaderResourceView();
	}

	BatchElement.PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// 인스턴싱 설정 (세그먼트 분할된 메시)
	BatchElement.VertexBuffer = VertexBuffer;
	BatchElement.IndexBuffer = IndexBuffer;
	BatchElement.VertexStride = sizeof(FParticleVertex);
	BatchElement.IndexCount = CachedSegmentCount;  // 10세그먼트 = 60 인덱스
	BatchElement.bIsInstanced = true;
	BatchElement.InstanceCount = ParticleCount;

	// 파티클 데이터 설정
	BatchElement.ParticleDataSRV = ParticleStructuredBufferSRV;
	BatchElement.bIsParticle = true;

	BatchElement.WorldMatrix = FMatrix::Identity();
	BatchElement.ObjectID = 0;

	Collector.Add(BatchElement);
}

/*-----------------------------------------------------------------------------
	FDynamicRibbonEmitterData
-----------------------------------------------------------------------------*/

FDynamicRibbonEmitterData::FDynamicRibbonEmitterData()
{
	// 리본용 초기화
	ParticleStructuredBufferSize = 128u;

	// 리본 세그먼트 데이터를 위한 동적 구조화 버퍼
	CreateParticleStructuredBuffer(sizeof(FRibbonSegmentVertex), ParticleStructuredBufferSize);

	ID3D11Device* Device = GEngine.GetRHIDevice()->GetDevice();
	if (!Device) return;

	// 리본 메시: 빔과 동일한 세그먼트 분할 방식
	const int32 NumSegments = 32;  // 리본은 더 많은 세그먼트
	const int32 NumVertices = (NumSegments + 1) * 2;
	const int32 NumIndices = NumSegments * 6;

	if (!VertexBuffer)
	{
		TArray<FParticleVertex> Vertices;
		Vertices.SetNum(NumVertices);

		for (int32 i = 0; i <= NumSegments; i++)
		{
			float t = (float)i / (float)NumSegments;  // 0 ~ 1
			float x = t - 0.5f;  // -0.5 ~ 0.5

			// 상단 정점
			Vertices[i * 2 + 0] = { FVector(x, 0.5f, 0.0f), FVector2D(t, 0.0f) };
			// 하단 정점
			Vertices[i * 2 + 1] = { FVector(x, -0.5f, 0.0f), FVector2D(t, 1.0f) };
		}

		D3D11_BUFFER_DESC Vbd{};
		Vbd.Usage = D3D11_USAGE_DEFAULT;
		Vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		Vbd.ByteWidth = sizeof(FParticleVertex) * NumVertices;

		D3D11_SUBRESOURCE_DATA Srd{};
		Srd.pSysMem = Vertices.GetData();

		HRESULT Hr = Device->CreateBuffer(&Vbd, &Srd, &VertexBuffer);
		assert(SUCCEEDED(Hr) && "Failed to create ribbon vertex buffer");
	}

	if (!IndexBuffer)
	{
		TArray<uint32> Indices;
		Indices.SetNum(NumIndices);

		for (int32 i = 0; i < NumSegments; i++)
		{
			int32 topLeft = i * 2;
			int32 bottomLeft = i * 2 + 1;
			int32 topRight = (i + 1) * 2;
			int32 bottomRight = (i + 1) * 2 + 1;

			int32 idx = i * 6;
			// 첫 번째 삼각형 (시계 방향 CW)
			Indices[idx + 0] = topLeft;
			Indices[idx + 1] = topRight;
			Indices[idx + 2] = bottomLeft;
			// 두 번째 삼각형 (시계 방향 CW)
			Indices[idx + 3] = bottomLeft;
			Indices[idx + 4] = topRight;
			Indices[idx + 5] = bottomRight;
		}

		D3D11_BUFFER_DESC Ibd{};
		Ibd.Usage = D3D11_USAGE_DEFAULT;
		Ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		Ibd.ByteWidth = sizeof(uint32) * NumIndices;

		D3D11_SUBRESOURCE_DATA Srd{};
		Srd.pSysMem = Indices.GetData();

		HRESULT Hr = Device->CreateBuffer(&Ibd, &Srd, &IndexBuffer);
		assert(SUCCEEDED(Hr) && "Failed to create ribbon index buffer");
	}

	CurrentIndexCount = NumIndices;
}

FDynamicRibbonEmitterData::~FDynamicRibbonEmitterData()
{
	if (ParticleStructuredBuffer)
	{
		ParticleStructuredBuffer->Release();
		ParticleStructuredBuffer = nullptr;
	}
	if (ParticleStructuredBufferSRV)
	{
		ParticleStructuredBufferSRV->Release();
		ParticleStructuredBufferSRV = nullptr;
	}
	if (VertexBuffer)
	{
		VertexBuffer->Release();
		VertexBuffer = nullptr;
	}
	if (IndexBuffer)
	{
		IndexBuffer->Release();
		IndexBuffer = nullptr;
	}
}

void FDynamicRibbonEmitterData::Init(bool bInSelected, const FParticleRibbonEmitterInstance* InEmitterInstance)
{
	bSelected = bInSelected;
}

void FDynamicRibbonEmitterData::GetDynamicMeshElementsEmitter(TArray<FMeshBatchElement>& Collector, const FSceneView* View)
{
	// 텍스처 스크롤용 시간
	static float ScrollTime = 0.0f;
	ScrollTime += 0.016f;

	const FDynamicRibbonEmitterReplayData* Src = static_cast<const FDynamicRibbonEmitterReplayData*>(GetSourceData());
	if (!Src) return;
	if (!View) return;

	const int32 ParticleCount = static_cast<uint32>(Src->ActiveParticleCount);

	if (ParticleCount <= 0) return;
	if (!Src->DataContainer.ParticleData) return;

	const int32 Stride = Src->ParticleStride;
	uint8* BasePtr = Src->DataContainer.ParticleData;
	const uint16* Indices = Src->DataContainer.ParticleIndices;
	bool bHasIndices = (Indices != nullptr);

	// 카메라 위치 가져오기
	FVector CameraPos = View->ViewLocation;

	// 먼저 총 포인트 수와 세그먼트 수를 계산
	int32 TotalPointCount = 0;
	int32 TotalSegmentCount = 0;
	for (int32 i = 0; i < ParticleCount; ++i)
	{
		int32 SrcIndex = bHasIndices ? Indices[i] : i;
		FRibbonParticlePayloadData* RibbonPayload = reinterpret_cast<FRibbonParticlePayloadData*>(
			BasePtr + SrcIndex * Stride + Src->RibbonPayloadOffset);
		int32 PointCount = RibbonPayload->PointCount;
		if (PointCount >= 2)
		{
			TotalPointCount += PointCount;
			TotalSegmentCount += (PointCount - 1);
		}
	}

	if (TotalPointCount <= 0) return;

	// 정점 버퍼 크기 계산 (각 포인트마다 좌/우 2개 정점)
	int32 TotalVertexCount = TotalPointCount * 2;
	// 인덱스 버퍼 크기 계산 (각 세그먼트마다 6개 인덱스 = 2 트라이앵글)
	int32 TotalIndexCount = TotalSegmentCount * 6;

	// 동적 버퍼 크기 확인 및 재생성
	if (TotalVertexCount > (int32)ParticleStructuredBufferSize)
	{
		uint32 NewSize = ParticleStructuredBufferSize;
		while (NewSize < (uint32)TotalVertexCount)
		{
			NewSize = FMath::Max(NewSize * 2, 256u);
		}
		ParticleStructuredBufferSize = NewSize;

		// 정점 버퍼 재생성
		CreateParticleStructuredBuffer(sizeof(FRibbonVertex), NewSize);
	}

	if (!ParticleStructuredBuffer) return;

	// CPU에서 빌보드 정점 생성
	ID3D11DeviceContext* Context = GEngine.GetRHIDevice()->GetDeviceContext();
	if (!Context) return;

	TArray<FRibbonVertex> Vertices;
	TArray<uint32> IndexData;
	Vertices.Reserve(TotalVertexCount);
	IndexData.Reserve(TotalIndexCount);

	float TailWidthScale = Src->TailWidthScale > 0.001f ? Src->TailWidthScale : 0.5f;
	float TailAlphaScale = Src->TailAlphaScale > 0.001f ? Src->TailAlphaScale : 0.0f;
	float TrailLifetime = Src->TrailLifetime > 0.001f ? Src->TrailLifetime : 1.0f;

	// Catmull-Rom 스플라인 보간 함수
	auto CatmullRom = [](const FVector& P0, const FVector& P1, const FVector& P2, const FVector& P3, float t) -> FVector
	{
		float t2 = t * t;
		float t3 = t2 * t;
		return (P1 * 2.0f +
			(P2 - P0) * t +
			(P0 * 2.0f - P1 * 5.0f + P2 * 4.0f - P3) * t2 +
			(P1 * 3.0f - P0 - P2 * 3.0f + P3) * t3) * 0.5f;
	};

	// 세그먼트당 보간 포인트 수 (원본 포인트 사이에 추가할 포인트 수)
	// 1 = 보간 없음, 3 = 각 세그먼트마다 3개 포인트
	const int32 SubdivisionCount = 3;

	uint32 BaseVertexIndex = 0;

	for (int32 i = 0; i < ParticleCount; ++i)
	{
		int32 SrcIndex = bHasIndices ? Indices[i] : i;
		FBaseParticle* BaseParticle = reinterpret_cast<FBaseParticle*>(BasePtr + SrcIndex * Stride);
		FRibbonParticlePayloadData* RibbonPayload = reinterpret_cast<FRibbonParticlePayloadData*>(
			BasePtr + SrcIndex * Stride + Src->RibbonPayloadOffset);
		if (!BaseParticle) continue;

		int32 PointCount = RibbonPayload->PointCount;
		if (PointCount < 2) continue;

		int32 HeadIdx = RibbonPayload->HeadIndex;
		uint32 RibbonStartVertex = Vertices.Num();

		// 이전 Right 벡터 저장 (일관성 유지용)
		FVector PrevRight = FVector(0, 0, 0);

		// 보간된 총 포인트 수 계산
		int32 InterpolatedPointCount = (PointCount - 1) * SubdivisionCount + 1;

		// 각 보간된 포인트에 대해 좌/우 정점 생성
		for (int32 interpIdx = 0; interpIdx < InterpolatedPointCount; ++interpIdx)
		{
			// 원본 세그먼트 인덱스와 세그먼트 내 t값 계산
			float globalT = (float)interpIdx / (float)(InterpolatedPointCount - 1);
			int32 segmentIdx = FMath::Min((int32)(globalT * (PointCount - 1)), PointCount - 2);
			float localT = (globalT * (PointCount - 1)) - segmentIdx;

			// Catmull-Rom에 필요한 4개 포인트 인덱스
			int32 j0 = FMath::Max(segmentIdx - 1, 0);
			int32 j1 = segmentIdx;
			int32 j2 = segmentIdx + 1;
			int32 j3 = FMath::Min(segmentIdx + 2, PointCount - 1);

			int32 Idx0 = (HeadIdx - j0 + FRibbonParticlePayloadData::MAX_TRAIL_POINTS) % FRibbonParticlePayloadData::MAX_TRAIL_POINTS;
			int32 Idx1 = (HeadIdx - j1 + FRibbonParticlePayloadData::MAX_TRAIL_POINTS) % FRibbonParticlePayloadData::MAX_TRAIL_POINTS;
			int32 Idx2 = (HeadIdx - j2 + FRibbonParticlePayloadData::MAX_TRAIL_POINTS) % FRibbonParticlePayloadData::MAX_TRAIL_POINTS;
			int32 Idx3 = (HeadIdx - j3 + FRibbonParticlePayloadData::MAX_TRAIL_POINTS) % FRibbonParticlePayloadData::MAX_TRAIL_POINTS;

			FVector P0 = RibbonPayload->PositionHistory[Idx0];
			FVector P1 = RibbonPayload->PositionHistory[Idx1];
			FVector P2 = RibbonPayload->PositionHistory[Idx2];
			FVector P3 = RibbonPayload->PositionHistory[Idx3];

			// Catmull-Rom 보간으로 위치 계산
			FVector Pos = CatmullRom(P0, P1, P2, P3, localT);

			// 위치 기반 페이드 (globalT: 0=머리, 1=꼬리 끝)
			// Width와 Alpha 계산 (위치 기반 - 꼬리 끝에서 페이드)
			float Width = Src->RibbonWidth * FMath::Lerp(1.0f, TailWidthScale, globalT);
			float Alpha = FMath::Lerp(1.0f, TailAlphaScale, globalT);

			// 리본 방향 계산 (스플라인 접선)
			FVector Dir;
			float epsilon = 0.01f;
			float t_prev = FMath::Max(localT - epsilon, 0.0f);
			float t_next = FMath::Min(localT + epsilon, 1.0f);
			FVector PosPrev = CatmullRom(P0, P1, P2, P3, t_prev);
			FVector PosNext = CatmullRom(P0, P1, P2, P3, t_next);
			Dir = (PosPrev - PosNext).GetSafeNormal();

			// 카메라를 향하는 right 벡터 계산
			FVector ToCamera = (CameraPos - Pos).GetSafeNormal();
			FVector RightRaw = FVector::Cross(Dir, ToCamera);
			float RightLen = RightRaw.Size();
			FVector Right;
			if (RightLen < 0.001f)
			{
				// Dir과 ToCamera가 평행한 경우 fallback
				RightRaw = FVector::Cross(Dir, FVector(0, 0, 1));
				RightLen = RightRaw.Size();
				if (RightLen < 0.001f)
				{
					RightRaw = FVector::Cross(Dir, FVector(0, 1, 0));
					RightLen = RightRaw.Size();
				}
			}
			Right = RightRaw / FMath::Max(RightLen, 0.001f);

			// 이전 Right와 방향 일관성 체크 (뒤집힘 방지)
			if (interpIdx > 0 && FVector::Dot(Right, PrevRight) < 0.0f)
			{
				Right = -Right;
			}
			PrevRight = Right;

			// UV 계산
			float U = globalT * Src->TextureTile + ScrollTime * Src->TextureScrollSpeed;

			// 색상 (알파 적용)
			FLinearColor Color = BaseParticle->Color;
			Color.A *= Alpha;

			// 좌측 정점
			FRibbonVertex LeftVertex;
			LeftVertex.Position = Pos - Right * Width * 0.5f;
			LeftVertex.UV = FVector2D(U, 0.0f);
			LeftVertex.Color = Color;
			Vertices.Add(LeftVertex);

			// 우측 정점
			FRibbonVertex RightVertex;
			RightVertex.Position = Pos + Right * Width * 0.5f;
			RightVertex.UV = FVector2D(U, 1.0f);
			RightVertex.Color = Color;
			Vertices.Add(RightVertex);
		}

		// 인덱스 생성 (트라이앵글 리스트)
		for (int32 j = 0; j < InterpolatedPointCount - 1; ++j)
		{
			uint32 V0 = RibbonStartVertex + j * 2;      // 현재 좌측
			uint32 V1 = RibbonStartVertex + j * 2 + 1;  // 현재 우측
			uint32 V2 = RibbonStartVertex + (j + 1) * 2;      // 다음 좌측
			uint32 V3 = RibbonStartVertex + (j + 1) * 2 + 1;  // 다음 우측

			// 첫 번째 트라이앵글 (V0, V2, V1)
			IndexData.Add(V0);
			IndexData.Add(V2);
			IndexData.Add(V1);

			// 두 번째 트라이앵글 (V1, V2, V3)
			IndexData.Add(V1);
			IndexData.Add(V2);
			IndexData.Add(V3);
		}

	}

	if (Vertices.Num() == 0 || IndexData.Num() == 0) return;

	UE_LOG("Ribbon: Vertices=%d, Indices=%d, Points=%d, Segments=%d, ParticleCount=%d",
		Vertices.Num(), IndexData.Num(), TotalPointCount, TotalSegmentCount, ParticleCount);

	// 동적 정점 버퍼 생성/업데이트
	static ID3D11Buffer* DynamicVertexBuffer = nullptr;
	static uint32 DynamicVertexBufferSize = 0;

	if (Vertices.Num() > (int32)DynamicVertexBufferSize || !DynamicVertexBuffer)
	{
		if (DynamicVertexBuffer)
		{
			DynamicVertexBuffer->Release();
			DynamicVertexBuffer = nullptr;
		}

		DynamicVertexBufferSize = FMath::Max((uint32)Vertices.Num() * 2, 256u);

		D3D11_BUFFER_DESC Desc = {};
		Desc.ByteWidth = DynamicVertexBufferSize * sizeof(FRibbonVertex);
		Desc.Usage = D3D11_USAGE_DYNAMIC;
		Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		GEngine.GetRHIDevice()->GetDevice()->CreateBuffer(&Desc, nullptr, &DynamicVertexBuffer);
	}

	if (DynamicVertexBuffer)
	{
		D3D11_MAPPED_SUBRESOURCE Mapped{};
		HRESULT Hr = Context->Map(DynamicVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped);
		if (SUCCEEDED(Hr))
		{
			memcpy(Mapped.pData, Vertices.GetData(), sizeof(FRibbonVertex) * Vertices.Num());
			Context->Unmap(DynamicVertexBuffer, 0);
		}
	}

	// 동적 인덱스 버퍼 생성/업데이트
	static ID3D11Buffer* DynamicIndexBuffer = nullptr;
	static uint32 DynamicIndexBufferSize = 0;

	if (IndexData.Num() > (int32)DynamicIndexBufferSize || !DynamicIndexBuffer)
	{
		if (DynamicIndexBuffer)
		{
			DynamicIndexBuffer->Release();
			DynamicIndexBuffer = nullptr;
		}

		DynamicIndexBufferSize = FMath::Max((uint32)IndexData.Num() * 2, 1024u);

		D3D11_BUFFER_DESC Desc = {};
		Desc.ByteWidth = DynamicIndexBufferSize * sizeof(uint32);
		Desc.Usage = D3D11_USAGE_DYNAMIC;
		Desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		GEngine.GetRHIDevice()->GetDevice()->CreateBuffer(&Desc, nullptr, &DynamicIndexBuffer);
	}

	if (DynamicIndexBuffer)
	{
		D3D11_MAPPED_SUBRESOURCE IdxMapped{};
		HRESULT Hr = Context->Map(DynamicIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &IdxMapped);
		if (SUCCEEDED(Hr))
		{
			memcpy(IdxMapped.pData, IndexData.GetData(), sizeof(uint32) * IndexData.Num());
			Context->Unmap(DynamicIndexBuffer, 0);
		}
	}

	// 배치 엘리먼트 생성
	FMeshBatchElement BatchElement;

	UShader* RibbonShader = UResourceManager::GetInstance().Load<UShader>("Shaders/Particles/ParticleRibbonVB.hlsl");

	if (RibbonShader)
	{
		BatchElement.VertexShader = RibbonShader->GetVertexShader();
		BatchElement.PixelShader = RibbonShader->GetPixelShader();
		BatchElement.InputLayout = RibbonShader->GetInputLayout();
	}

	// 재질 또는 텍스처 설정
	BatchElement.Material = Src->MaterialInterface;
	if (!BatchElement.Material)
	{
		BatchElement.InstanceShaderResourceView = UResourceManager::GetInstance().Load<UTexture>("Data/Textures/FakeLight.png")->GetShaderResourceView();
	}

	BatchElement.PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// 정점/인덱스 버퍼 설정
	BatchElement.VertexBuffer = DynamicVertexBuffer;
	BatchElement.IndexBuffer = DynamicIndexBuffer;
	BatchElement.VertexStride = sizeof(FRibbonVertex);
	BatchElement.IndexCount = IndexData.Num();
	BatchElement.bIsInstanced = false;
	BatchElement.InstanceCount = 1;

	// 파티클이 아닌 일반 메시로 렌더링
	BatchElement.ParticleDataSRV = nullptr;
	BatchElement.bIsParticle = false;

	BatchElement.WorldMatrix = FMatrix::Identity();
	BatchElement.ObjectID = 0;

	Collector.Add(BatchElement);
}
