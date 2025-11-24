#include "pch.h"
#include "ParticleHelper.h"

#include "ParticleSpriteEmitter.h"

FDynamicEmitterDataBase::FDynamicEmitterDataBase(const class UParticleModuleRequired* RequiredModule)
	: bSelected(false)
	, EmitterIndex(-1)
{
}

FDynamicSpriteEmitterData::FDynamicSpriteEmitterData(const UParticleModuleRequired* RequiredModule) : FDynamicSpriteEmitterDataBase(RequiredModule)
{
	constexpr uint32 MaxParticles = 4096u;
	const uint32 MaxVerts = MaxParticles * 4u;
	const uint32 MaxIndices = MaxParticles * 6u;

	ID3D11Device* Device = GEngine.GetRHIDevice()->GetDevice();
	if (!Device) return;

	// Vertex buffer (dynamic, CPU write)
	if (!VertexBuffer)
	{
		D3D11_BUFFER_DESC Vbd{};
		Vbd.Usage = D3D11_USAGE_DYNAMIC;
		Vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		Vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		Vbd.MiscFlags = 0;
		Vbd.ByteWidth = static_cast<UINT>(sizeof(FBillboardVertexInfo_GPU) * MaxVerts);
		HRESULT Hr = Device->CreateBuffer(&Vbd, nullptr, &VertexBuffer);
		assert(SUCCEEDED(Hr) && "Failed to create particle dynamic vertex buffer");
	}

	// Index buffer (static, precomputed quad Indices)
	if (!IndexBuffer)
	{
		TArray<uint32> Indices;
		Indices.reserve(MaxIndices);
		for (uint32 Index = 0; Index < MaxParticles; ++Index)
		{
			uint32 base = Index * 4;
			// two triangles: 0,1,2 and 0,2,3
			Indices.push_back(base + 0);
			Indices.push_back(base + 1);
			Indices.push_back(base + 2);

			Indices.push_back(base + 0);
			Indices.push_back(base + 2);
			Indices.push_back(base + 3);
		}

		D3D11_BUFFER_DESC Ibd{};
		Ibd.Usage = D3D11_USAGE_DEFAULT;
		Ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		Ibd.CPUAccessFlags = 0;
		Ibd.MiscFlags = 0;
		Ibd.ByteWidth = static_cast<UINT>(sizeof(uint32) * Indices.size());

		D3D11_SUBRESOURCE_DATA SourceData{};
		SourceData.pSysMem = Indices.data();

		HRESULT Hr = Device->CreateBuffer(&Ibd, &SourceData, &IndexBuffer);
		assert(SUCCEEDED(Hr) && "Failed to create particle index buffer");
	}
}

FDynamicSpriteEmitterData::~FDynamicSpriteEmitterData()
{
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
	if (!VertexBuffer || !IndexBuffer) return;

	// Prepare vertices from CPU particle data
	const int32 Stride = Src->ParticleStride;
	uint8* BasePtr = Src->DataContainer.ParticleData;

	TArray<FVertexDynamic> Vertices;
	Vertices.SetNum(ParticleCount * 4);

	const uint16* Indices = Src->DataContainer.ParticleIndices;
	bool bHasIndices = (Indices != nullptr);

	for (int32 i = 0; i < ParticleCount; ++i)
	{
		int32 SrcIndex = bHasIndices ? Indices[i] : i;
		uint8* ParticlePtr = BasePtr + SrcIndex * Stride;
		FBaseParticle* P = reinterpret_cast<FBaseParticle*>(ParticlePtr);
		if (!P) continue;

		const FVector& Pos = P->Location;
		const float SizeX = 2;	// NOTE: 크기 하드 코딩 (추후 아래 코드로 대체 필요)
		const float SizeY = 2;	// NOTE: 크기 하드 코딩 (추후 아래 코드로 대체 필요)
		//const float SizeX = P->Size.X;
		//const float SizeY = P->Size.Y;

		// 색상, 노멀, 탄젠트 설정
		FVector Normal = FVector(1,0,0);
		FVector4 Tangent = FVector4(0, 1, 0, 1); //FVector4(Right.X, Right.Y, Right.Z, 1.0f);
		FVector4 Color = FVector4(P->Color.R, P->Color.G, P->Color.B, P->Color.A);

		FVector LB = Pos; // (0) -> UV (0,1)
		LB.Y -= (SizeX * 0.5f);
		LB.Z -= (SizeY * 0.5f);

		FVector LT = Pos; // (1) -> UV (0,0)
		LT.Y -= (SizeX * 0.5f);
		LT.Z += (SizeY * 0.5f);

		FVector RT = Pos; // (2) -> UV (1,0)
		RT.Y += (SizeX * 0.5f);
		RT.Z += (SizeY * 0.5f);

		FVector RB = Pos; // (3) -> UV (1,1)
		RB.Y += (SizeX * 0.5f);
		RB.Z -= (SizeY * 0.5f);

		{
			FVertexDynamic V0; V0.Position = LB;V0.UV = FVector2D(0.0f, 1.0f); V0.Normal = Normal; V0.Tangent = Tangent; V0.Color = Color;
			FVertexDynamic V1; V1.Position = LT;V1.UV = FVector2D(0.0f, 0.0f); V1.Normal = Normal; V1.Tangent = Tangent; V1.Color = Color;
			FVertexDynamic V2; V2.Position = RT;V2.UV = FVector2D(1.0f, 0.0f); V2.Normal = Normal; V2.Tangent = Tangent; V2.Color = Color;
			FVertexDynamic V3; V3.Position = RB; V3.UV = FVector2D(1.0f, 1.0f); V3.Normal = Normal; V3.Tangent = Tangent; V3.Color = Color;

			Vertices[i * 4 + 0] = V0;
			Vertices[i * 4 + 1] = V1;
			Vertices[i * 4 + 2] = V2;
			Vertices[i * 4 + 3] = V3;
		}
	}

	// Upload vertex data to dynamic vertex buffer
	ID3D11DeviceContext* Context = GEngine.GetRHIDevice()->GetDeviceContext();
	if (Context)
	{
		D3D11_MAPPED_SUBRESOURCE Mapped{};
		HRESULT Hr = Context->Map(VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped);
		if (SUCCEEDED(Hr))
		{
			const size_t CopySize = sizeof(FVertexDynamic) * Vertices.Num();
			memcpy(Mapped.pData, Vertices.GetData(), CopySize);
			Context->Unmap(VertexBuffer, 0);
		}
	}

	// Create batch element referencing the uploaded buffers
	FMeshBatchElement BatchElement;
	if (Src->MaterialInterface && Src->MaterialInterface->GetShader())
	{
		UShader* Shader = Src->MaterialInterface->GetShader();
		FShaderVariant* Variant = Shader->GetOrCompileShaderVariant(View->ViewShaderMacros);
		if (Variant)
		{
			BatchElement.VertexShader = Variant->VertexShader;
			BatchElement.PixelShader = Variant->PixelShader;
			BatchElement.InputLayout = Variant->InputLayout;
		}
		BatchElement.Material = Src->MaterialInterface;
	}
	// NOTE: 임시로 기본 머티리얼 사용
	else
	{
		BatchElement.Material = UResourceManager::GetInstance().GetDefaultMaterial();
		UShader* Shader = BatchElement.Material->GetShader();
		FShaderVariant* Variant = Shader->GetOrCompileShaderVariant(View->ViewShaderMacros);
		if (Variant)
		{
			BatchElement.VertexShader = Variant->VertexShader;
			BatchElement.PixelShader = Variant->PixelShader;
			BatchElement.InputLayout = Variant->InputLayout;
		}
	}

	BatchElement.PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	BatchElement.VertexBuffer = VertexBuffer;
	BatchElement.IndexBuffer = IndexBuffer;
	BatchElement.IndexCount = static_cast<uint32>(ParticleCount * 6);
	BatchElement.StartIndex = 0;
	BatchElement.BaseVertexIndex = 0;
	BatchElement.VertexStride = sizeof(FVertexDynamic);
	BatchElement.WorldMatrix = FMatrix::Identity();	// TODO: 여기 파티클 컴포넌트의 월드 트랜스폼 매트릭스 넣기
	BatchElement.ObjectID = 0;	// TODO: 여기 파티클 컴포넌트의 InternalIndex 넣기

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
