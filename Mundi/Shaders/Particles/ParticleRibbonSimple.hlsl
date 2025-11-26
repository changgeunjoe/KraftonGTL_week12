//================================================================================================
// Filename:      ParticleRibbonSimple.hlsl
// Description:   CPU에서 빌보드 계산이 완료된 리본 정점을 렌더링하는 셰이더
//                연속된 메시로 렌더링되어 이음새가 없음
//================================================================================================

// b0: ModelBuffer (VS)
cbuffer ModelBuffer : register(b0)
{
	row_major float4x4 WorldMatrix;
	row_major float4x4 WorldInverseTranspose;
};

// b1: ViewProjBuffer (VS)
cbuffer ViewProjBuffer : register(b1)
{
	row_major float4x4 ViewMatrix;
	row_major float4x4 ProjectionMatrix;
	row_major float4x4 InverseViewMatrix;
	row_major float4x4 InverseProjectionMatrix;
};

// 리본 정점 데이터 (구조화 버퍼)
struct FRibbonVertex
{
	float3 Position;    // 월드 위치 (빌보드 오프셋 적용됨)
	float2 UV;          // 텍스처 좌표
	float4 Color;       // 색상 (알파 포함)
};

// 구조화 버퍼
StructuredBuffer<FRibbonVertex> RibbonVertexBuffer : register(t10);

// 텍스처 및 샘플러 (PS에서 사용)
Texture2D DiffuseTexture : register(t0);
SamplerState DefaultSampler : register(s0);

// 정점 입력 (인덱스만 사용)
struct VS_INPUT
{
	uint VertexID : SV_VertexID;
};

// VS -> PS로 전달되는 데이터 구조
struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
	float4 Color : COLOR0;
};

// PS -> Render Target으로 나가는 최종 출력 구조
struct PS_OUTPUT
{
	float4 Color : SV_Target0;
	uint UUID : SV_Target1;
};


//================================================================================================
// 버텍스 셰이더 (Vertex Shader)
//================================================================================================
PS_INPUT mainVS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;

	// 구조화 버퍼에서 정점 데이터 가져오기
	FRibbonVertex vertex = RibbonVertexBuffer[input.VertexID];

	// 이미 월드 좌표이므로 바로 뷰/프로젝션 변환
	output.Position = mul(float4(vertex.Position, 1.0f), mul(ViewMatrix, ProjectionMatrix));
	output.UV = vertex.UV;
	output.Color = vertex.Color;

	return output;
}


//================================================================================================
// 픽셀 셰이더 (Pixel Shader)
//================================================================================================
PS_OUTPUT mainPS(PS_INPUT input)
{
	PS_OUTPUT Output;
	Output.UUID = 0; // 파티클은 피킹 대상이 아니므로 0으로 설정

	// 디퓨즈 텍스처 샘플링
	float4 texColor = DiffuseTexture.Sample(DefaultSampler, input.UV);

	// 텍스처 색상과 정점 색상을 곱하여 최종 색상 결정
	Output.Color = texColor * input.Color;

	return Output;
}
