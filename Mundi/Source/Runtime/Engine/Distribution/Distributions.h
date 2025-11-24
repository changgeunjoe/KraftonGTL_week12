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
