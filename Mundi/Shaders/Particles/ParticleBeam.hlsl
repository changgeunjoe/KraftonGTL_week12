//================================================================================================
// Filename:      ParticleBeam.hlsl
// Description:   GPU 인스턴싱을 사용하여 빔 파티클을 렌더링하는 셰이더입니다.
//                시작점에서 끝점까지 연결하는 띠(ribbon) 형태
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

// 빔 파티클 데이터 (구조화 버퍼)
struct FBeamParticleData
{
	// 시작점
	float3 SourcePoint;
	float Width;

	// 끝점
	float3 TargetPoint;
	float TextureTile;  // 텍스처 타일링 횟수

	// 색상
	float4 Color;
};

// 파티클 데이터를 담는 구조화 버퍼 (VS에서만 사용)
StructuredBuffer<FBeamParticleData> BeamBuffer : register(t10);

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

	// 현재 인스턴스에 해당하는 빔 데이터 가져오기
	FBeamParticleData beam = BeamBuffer[input.InstanceID];

	// 빔 방향 계산
	float3 beamDir = beam.TargetPoint - beam.SourcePoint;
	float beamLength = length(beamDir);
	beamDir = normalize(beamDir);

	// 빔 중심점
	float3 beamCenter = (beam.SourcePoint + beam.TargetPoint) * 0.5f;

	// 카메라 위치 (InverseViewMatrix에서 추출)
	float3 cameraPos = float3(InverseViewMatrix._41, InverseViewMatrix._42, InverseViewMatrix._43);

	// 카메라를 향하는 방향
	float3 toCamera = normalize(cameraPos - beamCenter);

	// 빔에 수직이면서 카메라를 향하는 방향 (빔의 "위" 방향)
	float3 right = cross(beamDir, toCamera);
	if (length(right) < 0.001f)
	{
		right = cross(beamDir, float3(0, 0, 1));
	}
	right = normalize(right);

	// 정점 위치 계산
	// input.Position.x: -0.5 ~ 0.5 (빔 길이 방향)
	// input.Position.y: -0.5 ~ 0.5 (빔 너비 방향)
	float3 worldPosition = beamCenter;
	worldPosition += beamDir * input.Position.x * beamLength;      // 길이 방향
	worldPosition += right * input.Position.y * beam.Width;        // 너비 방향

	// 최종 화면 좌표로 변환
	output.Position = mul(float4(worldPosition, 1.0f), mul(ViewMatrix, ProjectionMatrix));

	// UV 좌표 - X는 빔 길이 방향 (타일링), Y는 너비 방향 (0~1)
	float tileCount = max(beam.TextureTile, 1.0f);
	output.UV.x = (input.Position.x + 0.5f) * tileCount;  // -0.5~0.5 -> 0~tileCount
	output.UV.y = input.UV.y;  // 0~1

	// 색상
	output.Color = beam.Color;

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
