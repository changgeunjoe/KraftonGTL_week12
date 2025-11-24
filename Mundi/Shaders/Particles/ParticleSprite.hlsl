//================================================================================================
// Filename:      ParticleSprite.hlsl
// Description:   GPU 인스턴싱을 사용하여 스프라이트 파티클을 렌더링하는 셰이더입니다.
//================================================================================================

// b0: ModelBuffer (VS) - ModelBufferType과 정확히 일치 (128 bytes)
cbuffer ModelBuffer : register(b0)
{
	row_major float4x4 WorldMatrix; // 64 bytes
	row_major float4x4 WorldInverseTranspose; // 64 bytes - 올바른 노멀 변환을 위함
};

// b1: ViewProjBuffer (VS) - ViewProjBufferType과 일치
cbuffer ViewProjBuffer : register(b1)
{
	row_major float4x4 ViewMatrix;
	row_major float4x4 ProjectionMatrix;
	row_major float4x4 InverseViewMatrix;
	row_major float4x4 InverseProjectionMatrix;
};

// C++의 FParticleForGPU 구조체와 일치해야 합니다 (패딩 포함).
struct FParticleForGPU
{
	float3 Location;
	float _pad0;
	float2 Size;
	float2 _pad1;
	float4 Color;
};

// 파티클 데이터를 담는 구조화 버퍼 (VS에서만 사용)
StructuredBuffer<FParticleForGPU> ParticleBuffer : register(t10);

// 텍스처 및 샘플러 (PS에서 사용)
Texture2D DiffuseTexture : register(t0);
SamplerState DefaultSampler : register(s0);

// 인스턴싱에 사용될 정적 쿼드의 정점 입력 구조
struct VS_INPUT
{
	float3 Position : POSITION0; // 모서리 오프셋 (예: -0.5, 0.5, 0)
	float2 UV : TEXCOORD0;
	uint InstanceID : SV_InstanceID;
};

// VS -> PS로 전달되는 데이터 구조
struct PS_INPUT
{
	float4 Position   : SV_POSITION;
	float2 UV         : TEXCOORD0;
	float4 Color      : COLOR0;
};

// PS -> Render Target으로 나가는 최종 출력 구조
struct PS_OUTPUT
{
    float4 Color : SV_Target0;
    uint UUID    : SV_Target1;
};


//================================================================================================
// 버텍스 셰이더 (Vertex Shader)
//================================================================================================
PS_INPUT mainVS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;

	// 현재 인스턴스에 해당하는 파티클 데이터 가져오기
	FParticleForGPU particle = ParticleBuffer[input.InstanceID];

	// 카메라를 마주보도록 쿼드를 회전시키는 빌보드 로직
	// ViewMatrix에서 카메라의 Right, Up 벡터를 추출
	float3 camRight = float3(ViewMatrix._11, ViewMatrix._21, ViewMatrix._31);
	float3 camUp = float3(ViewMatrix._12, ViewMatrix._22, ViewMatrix._32);

	// 정점의 월드 위치 계산
	// 파티클 위치를 기준으로, 카메라의 Right/Up 벡터와 파티클 크기를 사용하여 오프셋 적용
	float3 worldPosition = particle.Location;
	worldPosition += (camRight * input.Position.x * particle.Size.x);
	worldPosition += (camUp * input.Position.y * particle.Size.y);

	// 최종 화면 좌표로 변환
	output.Position = mul(float4(worldPosition, 1.0f), mul(ViewMatrix, ProjectionMatrix));
	
	// UV와 색상 정보를 픽셀 셰이더로 전달
	output.UV = input.UV;
	output.Color = particle.Color;

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
	
	// 텍스처 색상과 파티클의 색상을 곱하여 최종 색상 결정
	Output.Color = texColor * input.Color;

	return Output;
}
