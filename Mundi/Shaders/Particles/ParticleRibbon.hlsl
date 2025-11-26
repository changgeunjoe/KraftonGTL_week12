//================================================================================================
// Filename:      ParticleRibbon.hlsl
// Description:   GPU 인스턴싱을 사용하여 리본/트레일 파티클을 렌더링하는 셰이더입니다.
//                파티클의 이동 경로를 따라 연결하는 띠(ribbon) 형태
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

// 리본 세그먼트 데이터 (구조화 버퍼)
struct FRibbonSegmentData
{
	// 세그먼트 시작 위치
	float3 Position0;
	float Width0;

	// 세그먼트 끝 위치
	float3 Position1;
	float Width1;

	// 색상
	float4 Color;

	// UV 좌표
	float TexCoord0;
	float TexCoord1;
	// 알파 페이드
	float Alpha0;
	float Alpha1;
};

// 파티클 데이터를 담는 구조화 버퍼 (VS에서만 사용)
StructuredBuffer<FRibbonSegmentData> RibbonBuffer : register(t10);

// 텍스처 및 샘플러 (PS에서 사용)
Texture2D DiffuseTexture : register(t0);
SamplerState DefaultSampler : register(s0);

// 인스턴싱에 사용될 정적 쿼드의 정점 입력 구조
struct VS_INPUT
{
	float3 Position : POSITION0; // 쿼드 정점 (-0.5~0.5)
	float2 UV : TEXCOORD0;
	uint InstanceID : SV_InstanceID;
};

// VS -> PS로 전달되는 데이터 구조
struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
	float4 Color : COLOR0;
	float Alpha : TEXCOORD1;  // 알파 페이드 값
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

	// 현재 인스턴스에 해당하는 리본 데이터 가져오기
	FRibbonSegmentData ribbon = RibbonBuffer[input.InstanceID];

	// 리본 방향 계산 (Position0 -> Position1)
	float3 ribbonDir = ribbon.Position1 - ribbon.Position0;
	float ribbonLength = length(ribbonDir);

	if (ribbonLength < 0.001f)
	{
		// 길이가 너무 짧으면 렌더링하지 않음
		output.Position = float4(0, 0, 0, 0);
		return output;
	}

	ribbonDir = normalize(ribbonDir);

	// 카메라 위치 (InverseViewMatrix에서 추출)
	float3 cameraPos = float3(InverseViewMatrix._41, InverseViewMatrix._42, InverseViewMatrix._43);

	// t = 0 (Position0), t = 1 (Position1)
	float t = input.Position.x + 0.5f;  // -0.5~0.5 -> 0~1

	// 위치 보간
	float3 worldPosition = lerp(ribbon.Position0, ribbon.Position1, t);

	// 각 정점 위치에서 카메라 방향 계산 (중심이 아닌 실제 정점 위치 기준)
	float3 toCamera = normalize(cameraPos - worldPosition);

	// 리본에 수직이면서 카메라를 향하는 방향 (리본의 "옆" 방향)
	float3 right = cross(ribbonDir, toCamera);
	if (length(right) < 0.001f)
	{
		right = cross(ribbonDir, float3(0, 0, 1));
	}
	right = normalize(right);

	// 너비 보간
	float currentWidth = lerp(ribbon.Width0, ribbon.Width1, t);

	// 너비 방향 오프셋 적용
	worldPosition += right * input.Position.y * currentWidth;

	// 최종 화면 좌표로 변환
	output.Position = mul(float4(worldPosition, 1.0f), mul(ViewMatrix, ProjectionMatrix));

	// UV 좌표 보간
	output.UV.x = lerp(ribbon.TexCoord0, ribbon.TexCoord1, t);
	output.UV.y = input.UV.y;

	// 색상
	output.Color = ribbon.Color;

	// 알파 보간
	output.Alpha = lerp(ribbon.Alpha0, ribbon.Alpha1, t);

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

	// 알파 페이드 적용
	Output.Color.a *= input.Alpha;

	return Output;
}
