#include "pch.h"
#include "ParticleHelper.h"

#include "ParticleSpriteEmitter.h"

constexpr uint32 MaxParticles = 4096u;

FDynamicEmitterDataBase::FDynamicEmitterDataBase(const class UParticleModuleRequired* RequiredModule)
	: bSelected(false)
	, EmitterIndex(-1)
{
}

FDynamicSpriteEmitterDataBase::FDynamicSpriteEmitterDataBase(const UParticleModuleRequired* RequiredModule) :
	FDynamicEmitterDataBase(RequiredModule),
	bUsesDynamicParameter(false)
{
}

FDynamicSpriteEmitterData::FDynamicSpriteEmitterData(const UParticleModuleRequired* RequiredModule) : FDynamicSpriteEmitterDataBase(RequiredModule)
{
	ID3D11Device* Device = GEngine.GetRHIDevice()->GetDevice();
	if (!Device) return;

	// 1. 파티클별 데이터를 위한 동적 구조화 버퍼
	if (!ParticleStructuredBuffer)
	{
		D3D11_BUFFER_DESC Sbd{};
		Sbd.Usage = D3D11_USAGE_DYNAMIC;
		Sbd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		Sbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		Sbd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		Sbd.StructureByteStride = sizeof(FParticleSpriteVertex);
		Sbd.ByteWidth = static_cast<UINT>(sizeof(FParticleSpriteVertex) * MaxParticles);

		HRESULT Hr = Device->CreateBuffer(&Sbd, nullptr, &ParticleStructuredBuffer);
		assert(SUCCEEDED(Hr) && "Failed to create particle structured buffer");

		D3D11_SHADER_RESOURCE_VIEW_DESC Srvd{};
		Srvd.Format = DXGI_FORMAT_UNKNOWN;
		Srvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		Srvd.Buffer.FirstElement = 0;
		Srvd.Buffer.NumElements = MaxParticles;

		Hr = Device->CreateShaderResourceView(ParticleStructuredBuffer, &Srvd, &ParticleStructuredBufferSRV);
		assert(SUCCEEDED(Hr) && "Failed to create particle structured buffer SRV");
	}

	// 2. 인스턴싱을 위한 정적 정점 버퍼 (단일 쿼드)
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

	// 3. 인스턴싱을 위한 정적 인덱스 버퍼 (단일 쿼드)
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

void FDynamicSpriteEmitterData::GetDynamicMeshElementsEmitter(TArray<FMeshBatchElement>& Collector, const FSceneView* View) const
{
	const FDynamicSpriteEmitterReplayDataBase* Src = GetSourceData();
	if (!Src) return;
	const int32 ParticleCount = Src->ActiveParticleCount;
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

			for (int32 i = 0; i < ParticleCount; ++i)
			{
				int32 SrcIndex = bHasIndices ? Indices[i] : i;
				FBaseParticle* P = reinterpret_cast<FBaseParticle*>(BasePtr + SrcIndex * Stride);
				if (!P) continue;

				GpuParticles[i].Position = P->Location;
				GpuParticles[i].Size = FVector2D(2, 2);
				//GpuParticles[i].Size = FVector2D(P->Size.X, P->Size.Y);
				GpuParticles[i].Color = FLinearColor(1, 1, 1, 1);
				//GpuParticles[i].Color = P->Color;
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

FDynamicMeshEmitterData::FDynamicMeshEmitterData(const UParticleModuleRequired* RequiredModule)
	: FDynamicSpriteEmitterDataBase(RequiredModule)
	, StaticMesh(nullptr)
{
	ID3D11Device* Device = GEngine.GetRHIDevice()->GetDevice();
	if (!Device) return;

	// 1. 파티클별 데이터를 위한 동적 구조화 버퍼
	if (!ParticleStructuredBuffer)
	{
		D3D11_BUFFER_DESC Sbd{};
		Sbd.Usage = D3D11_USAGE_DYNAMIC;
		Sbd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		Sbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		Sbd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		Sbd.StructureByteStride = sizeof(FMeshParticleInstanceVertex);
		Sbd.ByteWidth = static_cast<UINT>(sizeof(FMeshParticleInstanceVertex) * MaxParticles);

		HRESULT Hr = Device->CreateBuffer(&Sbd, nullptr, &ParticleStructuredBuffer);
		assert(SUCCEEDED(Hr) && "Failed to create particle structured buffer");

		D3D11_SHADER_RESOURCE_VIEW_DESC Srvd{};
		Srvd.Format = DXGI_FORMAT_UNKNOWN;
		Srvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		Srvd.Buffer.FirstElement = 0;
		Srvd.Buffer.NumElements = MaxParticles;

		Hr = Device->CreateShaderResourceView(ParticleStructuredBuffer, &Srvd, &ParticleStructuredBufferSRV);
		assert(SUCCEEDED(Hr) && "Failed to create particle structured buffer SRV");
	}
}

void FDynamicMeshEmitterData::Init(bool bInSelected, const FParticleMeshEmitterInstance* InEmitterInstance, UStaticMesh* InStaticMesh, bool InUseStaticMeshLODs, float InLODSizeScale)
{
	bSelected = bInSelected;

	StaticMesh = InStaticMesh;
	assert(StaticMesh);

	// @todo
}

void FDynamicMeshEmitterData::GetDynamicMeshElementsEmitter(TArray<FMeshBatchElement>& Collector, const FSceneView* View) const
{
	const FDynamicSpriteEmitterReplayDataBase* Src = GetSourceData();
	if (!Src) return;
	const int32 ParticleCount = Src->ActiveParticleCount;
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
				FBaseParticle* P = reinterpret_cast<FBaseParticle*>(BasePtr + SrcIndex * Stride);
				if (!P) continue;

				// NOTE: 크기 하드 코딩
				P->Size = FVector::One();

				FTransform Transform(P->Location, FQuat::MakeFromEulerZYX(FVector(P->Rotation, P->Rotation, P->Rotation)), P->Size);
				GpuParticles[i].Transform = Transform.ToMatrix();
				GpuParticles[i].Color = FLinearColor(1, 1, 1, 1);
				//GpuParticles[i].Color = P->Color;
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
