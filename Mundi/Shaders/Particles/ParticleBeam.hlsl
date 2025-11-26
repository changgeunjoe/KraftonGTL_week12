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

	// Taper (굵기 변화)
	float SourceTaper;
	float TargetTaper;
	// 노이즈
	float NoiseStrength;
	float NoiseTime;
	// 텍스처 스크롤 속도
	float TextureScrollSpeed;
	// 두께 펄스
	float PulseSpeed;
	float PulseScale;
	// 노이즈 옥타브 (1~4)
	float NoiseOctaves;
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
	float BeamT : TEXCOORD1;  // 빔 위치 (0=Source, 1=Target)
	float TextureScroll : TEXCOORD2;  // 텍스처 스크롤 오프셋
};

// PS -> Render Target으로 나가는 최종 출력 구조
struct PS_OUTPUT
{
	float4 Color : SV_Target0;
	uint UUID : SV_Target1;
};


// 간단한 해시 기반 노이즈 함수
float hash(float n)
{
	return frac(sin(n) * 43758.5453f);
}

float noise(float x)
{
	float i = floor(x);
	float f = frac(x);
	float u = f * f * (3.0f - 2.0f * f);  // smoothstep
	return lerp(hash(i), hash(i + 1.0f), u) * 2.0f - 1.0f;  // -1 ~ 1 범위
}

// 프랙탈 브라운 운동 (FBM) - 여러 옥타브의 노이즈 합성
float fbm(float x, float octaves)
{
	float value = 0.0f;
	float amplitude = 0.5f;
	float frequency = 1.0f;

	for (int i = 0; i < 4; i++)  // 최대 4 옥타브
	{
		if (i >= (int)octaves) break;
		value += amplitude * noise(x * frequency);
		frequency *= 2.0f;
		amplitude *= 0.5f;
	}
	return value;
}

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

	// Taper 계산: 빔 위치에 따라 굵기 보간
	// t = 0 (Source), t = 1 (Target)
	float t = input.Position.x + 0.5f;  // -0.5~0.5 -> 0~1
	float taperScale = lerp(beam.SourceTaper, beam.TargetTaper, t);

	// 두께 펄스 적용 (PulseScale이 0보다 크면 활성화)
	float pulseMultiplier = 1.0f;
	if (beam.PulseScale > 0.001f)
	{
		// sin 기반 펄스: 1.0 ~ (1.0 + PulseScale) 범위
		pulseMultiplier = 1.0f + beam.PulseScale * (0.5f + 0.5f * sin(beam.NoiseTime * beam.PulseSpeed));
	}

	float currentWidth = beam.Width * taperScale * pulseMultiplier;

	float3 worldPosition = beamCenter;
	worldPosition += beamDir * input.Position.x * beamLength;      // 길이 방향
	worldPosition += right * input.Position.y * currentWidth;      // 너비 방향 (Taper + Pulse 적용)

	// 노이즈 적용 (양 끝은 고정, 중간은 흔들림)
	if (beam.NoiseStrength > 0.001f)
	{
		// 양 끝 페이드: sin 곡선으로 양 끝에서는 노이즈 0
		float noiseFade = sin(t * 3.14159f);

		// 인스턴스별 오프셋 (각 빔마다 다른 노이즈 패턴)
		float instanceOffset = hash((float)input.InstanceID * 127.1f) * 100.0f;

		// 노이즈 계산 (시간 + 위치 + 인스턴스 오프셋)
		// 옥타브 수에 따라 FBM 또는 단순 노이즈 사용
		float noiseInput = t * 5.0f + beam.NoiseTime * 3.0f + instanceOffset;
		float noiseValue = (beam.NoiseOctaves > 1.0f)
			? fbm(noiseInput, beam.NoiseOctaves)
			: noise(noiseInput);

		// right 방향으로 오프셋 적용
		worldPosition += right * noiseValue * beam.NoiseStrength * noiseFade;
	}

	// 최종 화면 좌표로 변환
	output.Position = mul(float4(worldPosition, 1.0f), mul(ViewMatrix, ProjectionMatrix));

	// UV 좌표 - X는 빔 길이 방향 (타일링), Y는 너비 방향 (0~1)
	float tileCount = max(beam.TextureTile, 1.0f);
	output.UV.x = (input.Position.x + 0.5f) * tileCount;  // -0.5~0.5 -> 0~tileCount
	output.UV.y = input.UV.y;  // 0~1

	// 색상
	output.Color = beam.Color;

	// 빔 위치 (페이드용)
	output.BeamT = t;

	// 텍스처 스크롤 오프셋 전달
	output.TextureScroll = beam.NoiseTime * beam.TextureScrollSpeed;

	return output;
}


//================================================================================================
// 픽셀 셰이더 (Pixel Shader)
//================================================================================================
PS_OUTPUT mainPS(PS_INPUT input)
{
	PS_OUTPUT Output;
	Output.UUID = 0; // 파티클은 피킹 대상이 아니므로 0으로 설정

	// 텍스처 스크롤 적용된 UV
	float2 scrolledUV = input.UV;
	scrolledUV.x -= input.TextureScroll;  // 시간에 따라 UV 이동

	// 디퓨즈 텍스처 샘플링
	float4 texColor = DiffuseTexture.Sample(DefaultSampler, scrolledUV);

	// 텍스처 색상과 파티클의 색상을 곱하여 최종 색상 결정
	Output.Color = texColor * input.Color;

	// 양 끝 페이드: sin 곡선으로 부드럽게 (0->1->0)
	float edgeFade = sin(input.BeamT * 3.14159f);
	Output.Color.a *= edgeFade;

	return Output;
}
