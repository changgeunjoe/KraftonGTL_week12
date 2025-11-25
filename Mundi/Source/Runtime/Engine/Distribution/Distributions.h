#pragma once
#include <random>
#include "Vector.h"

struct FRandomStream
{
	std::mt19937 Generator;

	FRandomStream()
	{
		std::random_device Rd;
		Generator.seed(Rd());
	}

	FRandomStream(int32 Seed)
	{
		Generator.seed(Seed);
	}

	/** 0.0 ~ 1.0 사이의 랜덤 float 반환 */
	float GetFraction()
	{
		std::uniform_real_distribution<float> Dist(0.0f, 1.0f);
		return Dist(Generator);
	}

	/** Min ~ Max 사이의 랜덤 float 반환 */
	float GetRange(float Min, float Max)
	{
		return Min + (Max - Min) * GetFraction();
	}

	/** Min ~ Max 사이의 랜덤 Vector 반환 */
	FVector GetVectorRange(const FVector& Min, const FVector& Max)
	{
		return FVector(
			GetRange(Min.X, Max.X),
			GetRange(Min.Y, Max.Y),
			GetRange(Min.Z, Max.Z)
		);
	}

	/** 단위 벡터 반환 */
	FVector GetUnitVector()
	{
		FVector V;
		V.X = GetRange(-1.0f, 1.0f);
		V.Y = GetRange(-1.0f, 1.0f);
		V.Z = GetRange(-1.0f, 1.0f);
		return V.GetSafeNormal();
	}
};

UCLASS()
class UDistribution : public UObject
{
	DECLARE_CLASS(UDistribution, UObject)
public:
	UDistribution() = default;
	virtual ~UDistribution() = default;
};

// ------------------------------------------------------------
// Distribution Float
// ------------------------------------------------------------
UCLASS()
class UDistributionFloat : public UDistribution
{
	DECLARE_CLASS(UDistributionFloat, UDistribution)
public:
	/**
	 * 시간 F에 따른 값을 반환한다.
	 * @param F              시간 (0.0 ~ 1.0)
	 * @param Data           (옵션) 외부 데이터
	 * @param InRandomStream (옵션) 난수 생성기. 없으면 전역 난수 사용.
	 */
	virtual float GetValue(float F = 0.0f, void* Data = nullptr, FRandomStream* InRandomStream = nullptr) const
	{
		return 0.0f;
	}
};

UCLASS()
class UDistributionFloatConstant : public UDistributionFloat
{
	DECLARE_CLASS(UDistributionFloatConstant, UDistributionFloat)
public:
	float Constant;

	UDistributionFloatConstant() : Constant(0.0f) {}

	virtual float GetValue(float F = 0.0f, void* Data = nullptr, FRandomStream* InRandomStream = nullptr) const override
	{
		return Constant;
	}
};

UCLASS()
class UDistributionFloatUniform : public UDistributionFloat
{
	DECLARE_CLASS(UDistributionFloatUniform, UDistributionFloat)
public:
	float Min;
	float Max;

	UDistributionFloatUniform() : Min(0.0f), Max(0.0f) {}

	virtual float GetValue(float F = 0.0f, void* Data = nullptr, FRandomStream* InRandomStream = nullptr) const override
	{
		if (InRandomStream)
		{
			return InRandomStream->GetRange(Min, Max);
		}
		else
		{
			// Stream이 없으면 임시 생성 (혹은 전역 싱글톤 사용 권장)
			static FRandomStream GlobalStream;
			return GlobalStream.GetRange(Min, Max);
		}
	}
};

UCLASS()
class UDistributionFloatBezier : public UDistributionFloat
{
	DECLARE_CLASS(UDistributionFloatBezier, UDistributionFloat)
public:
	// 입력(시간) 범위 설정
	float MinInput;
	float MaxInput;

	// 4개의 고정 제어점 (출력값)
	float P0; // Start Value
	float P1; // Start Tangent
	float P2; // End Tangent
	float P3; // End Value

	UDistributionFloatBezier()
		: MinInput(0.0f), MaxInput(1.0f)
		, P0(0.0f), P1(0.0f), P2(0.0f), P3(0.0f)
	{}

	UDistributionFloatBezier(float InMinInput, float InMaxInput, float InP0, float InP1, float InP2, float InP3)
		: MinInput(InMinInput), MaxInput(InMaxInput)
		, P0(InP0), P1(InP1), P2(InP2), P3(InP3)
	{}

	virtual float GetValue(float F = 0.0f, void* Data = nullptr, FRandomStream* InRandomStream = nullptr) const override
	{
		// 1. 입력 범위(MinInput ~ MaxInput)를 0 ~ 1 로 정규화
		float Range = MaxInput - MinInput;
		float T = 0.0f;

		// 분모가 0에 가까우면(범위가 없으면) 0이나 1로 처리 (안전장치)
		if (std::abs(Range) < 1.e-6f) // 아주 작은 값(Epsilon)
		{
			T = (F >= MaxInput) ? 1.0f : 0.0f;
		}
		else
		{
			T = (F - MinInput) / Range;
		}

		// 2. 0.0 ~ 1.0 클램핑 (커브 범위를 벗어난 입력은 양 끝값 유지)
		if (T < 0.0f) T = 0.0f;
		if (T > 1.0f) T = 1.0f;

		// 3. 3차 베지어 계산
		float U = 1.0f - T;
		float TT = T * T;
		float UU = U * U;
		float UUU = UU * U;
		float TTT = TT * T;

		// P0~P3는 Y값(출력값)임
		float Result = (UUU * P0) + (3 * UU * T * P1) + (3 * U * TT * P2) + (TTT * P3);
		return Result;
	}
};

// ------------------------------------------------------------
// Distribution Vector
// ------------------------------------------------------------
UCLASS()
class UDistributionVector : public UDistribution
{
	DECLARE_CLASS(UDistributionVector, UDistribution)
public:
	/** 파티클 시스템 등에서 호출하는 메인 함수이다. */
	virtual FVector GetValue(float F = 0.0f, void* Data = nullptr, int32 LastExtreme = 0, FRandomStream* InRandomStream = nullptr) const
	{
		return FVector::Zero();
	}
};

UCLASS()
class UDistributionVectorConstant : public UDistributionVector
{
	DECLARE_CLASS(UDistributionVectorConstant, UDistributionVector)
public:
	FVector Constant;

	UDistributionVectorConstant() : Constant(FVector::Zero()) {}

	virtual FVector GetValue(float F = 0.0f, void* Data = nullptr, int32 LastExtreme = 0, FRandomStream* InRandomStream = nullptr) const override
	{
		return Constant;
	}
};

UCLASS()
class UDistributionVectorUniform : public UDistributionVector
{
	DECLARE_CLASS(UDistributionVectorUniform, UDistributionVector)
public:
	FVector Min;
	FVector Max;

	UDistributionVectorUniform() : Min(0.0f), Max(0.0f) {}

	virtual FVector GetValue(float F = 0.0f, void* Data = nullptr, int32 LastExtreme = 0, FRandomStream* InRandomStream = nullptr) const override
	{
		if (InRandomStream)
		{
			return InRandomStream->GetVectorRange(Min, Max);
		}
		else
		{
			static FRandomStream GlobalStream;
			return GlobalStream.GetVectorRange(Min, Max);
		}
	}
};

// ─────────────────────────────────────────────────────────────
// FRawDistributionFloat
// ─────────────────────────────────────────────────────────────

struct FRawDistributionFloat
{
	UDistributionFloat* Distribution;

	FRawDistributionFloat() : Distribution(nullptr) {}

	~FRawDistributionFloat()
	{
		if (Distribution)
		{
			DeleteObject(Distribution);
		}
	}

	bool IsCreated() const { return Distribution != nullptr; }

	float GetValue(float F = 0.0f, void* Data = nullptr, FRandomStream* InRandomStream = nullptr)
	{
		if (Distribution)
		{
			return Distribution->GetValue(F, Data, InRandomStream);
		}
		return 0.0f;
	}

	void GetOutRange(float& MinOut, float& MaxOut)
	{
		MinOut = 0.0f;
		MaxOut = 0.0f;

		if (Distribution)
		{
			if (auto* DistConst = Cast<UDistributionFloatConstant>(Distribution))
			{
				MinOut = DistConst->Constant;
				MaxOut = DistConst->Constant;
			}
			else if (auto* DistUniform = Cast<UDistributionFloatUniform>(Distribution))
			{
				MinOut = DistUniform->Min;
				MaxOut = DistUniform->Max;
			}
		}
	}
};

// ------------------------------------------------------------
// FRawDistributionVector
// ------------------------------------------------------------

struct FRawDistributionVector
{
	UDistributionVector* Distribution;

	FRawDistributionVector() : Distribution(nullptr) {}

	~FRawDistributionVector()
	{
		if (Distribution)
		{
			DeleteObject(Distribution);
		}
	}

	bool IsCreated() const { return Distribution != nullptr; }

	FVector GetValue(float F = 0.0f, void* Data = nullptr, int32 LastExtreme = 0, FRandomStream* InRandomStream = nullptr)
	{
		if (Distribution)
		{
			return Distribution->GetValue(F, Data, LastExtreme, InRandomStream);
		}
		return FVector::Zero();
	}

	void GetOutRange(FVector& MinOut, FVector& MaxOut)
	{
		MinOut = FVector::Zero();
		MaxOut = FVector::Zero();

		if (Distribution)
		{
			if (auto* DistConst = Cast<UDistributionVectorConstant>(Distribution))
			{
				MinOut = DistConst->Constant;
				MaxOut = DistConst->Constant;
			}
			else if (auto* DistUniform = Cast<UDistributionVectorUniform>(Distribution))
			{
				MinOut = DistUniform->Min;
				MaxOut = DistUniform->Max;
			}
		}
	}
};

UCLASS()
class UDistributionVectorBezier : public UDistributionVector
{
	DECLARE_CLASS(UDistributionVectorBezier, UDistributionVector)
public:
	// 입력 범위
	float MinInput;
	float MaxInput;

	// 제어점 (출력 벡터)
	FVector P0;
	FVector P1;
	FVector P2;
	FVector P3;

	UDistributionVectorBezier()
		: MinInput(0.0f), MaxInput(1.0f)
		, P0(FVector::Zero()), P1(FVector::Zero()), P2(FVector::Zero()), P3(FVector::Zero())
	{}

	virtual FVector GetValue(float F = 0.0f, void* Data = nullptr, int32 LastExtreme = 0, FRandomStream* InRandomStream = nullptr) const override
	{
		// 1. 정규화
		float Range = MaxInput - MinInput;
		float T = 0.0f;

		if (std::abs(Range) < 1.e-6f)
		{
			T = (F >= MaxInput) ? 1.0f : 0.0f;
		}
		else
		{
			T = (F - MinInput) / Range;
		}

		// 2. 클램핑
		if (T < 0.0f) T = 0.0f;
		if (T > 1.0f) T = 1.0f;

		// 3. 베지어 계산
		float U = 1.0f - T;
		float TT = T * T;
		float UU = U * U;
		float UUU = UU * U;
		float TTT = TT * T;

		// Vector 연산
		FVector Result = (P0 * UUU) + (P1 * (3 * UU * T)) + (P2 * (3 * U * TT)) + (P3 * TTT);
		return Result;
	}
};
