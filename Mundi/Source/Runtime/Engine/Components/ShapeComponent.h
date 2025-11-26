#pragma once

#include "PrimitiveComponent.h"
#include "BoxSphereBounds.h"
#include "Collision/OverlapInfo.h"
#include "Delegate.h"
#include "UShapeComponent.generated.h"

// Forward declaration
class UCollisionManager;

enum class EShapeKind : uint8
{
	Box = 0,
	Sphere,
	Capsule,
};

struct FBoxShape { FVector BoxExtent; };
struct FSphereShape { float SphereRadius; };
struct FCapsuleSphere { float CapsuleRadius, CapsuleHalfHeight; };

struct FShape
{
	FShape() {}

	EShapeKind Kind;
	union {
		FBoxShape Box;
		FSphereShape Sphere;
		FCapsuleSphere Capsule;
	};
};

/**
 * FComponentOverlapSignature
 *
 * Overlap 이벤트를 처리하는 델리게이트 시그니처입니다.
 *
 * @param OverlappedComponent - Overlap이 발생한 자신의 컴포넌트
 * @param OtherActor - 충돌한 상대방 액터
 * @param OtherComp - 충돌한 상대방 컴포넌트
 * @param ContactPoint - 충돌 지점 (World Space)
 * @param PenetrationDepth - 침투 깊이
 */
DECLARE_MULTICAST_DELEGATE_FiveParams(
	FComponentOverlapSignature,
	UShapeComponent*,      // OverlappedComponent
	AActor*,               // OtherActor
	UShapeComponent*,      // OtherComp
	const FVector&,        // ContactPoint
	float                  // PenetrationDepth
);

UCLASS(DisplayName="셰이프 컴포넌트", Description="충돌 모양 기본 컴포넌트입니다")
class UShapeComponent : public UPrimitiveComponent
{
public:

	GENERATED_REFLECTION_BODY();

public:

    // ===== Lua-Bindable Properties (Auto-moved from protected/private) =====

	UPROPERTY(EditAnywhere, Category="Shape")
	bool bShapeIsVisible;

	UPROPERTY(EditAnywhere, Category="Shape")
	bool bShapeHiddenInGame;

	UShapeComponent();
	virtual ~UShapeComponent();

	virtual void TickComponent(float DeltaSeconds) override;

	virtual void GetShape(FShape& OutShape) const {};
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason Reason) override;
    virtual void OnRegister(UWorld* InWorld) override;
    virtual void OnTransformUpdated() override;

	// ────────────────────────────────────────────────
	// Overlap 관련 함수
	// ────────────────────────────────────────────────

	/**
	 * 다른 Shape 컴포넌트와 겹쳐있는지 확인합니다.
	 *
	 * @param Other - 확인할 상대방 Shape 컴포넌트
	 * @return 겹쳐있으면 true, 아니면 false
	 */
	virtual bool IsOverlappingComponent(const UShapeComponent* Other) const;

	/**
	 * 현재 Overlap 상태를 업데이트하고 이벤트를 발생시킵니다.
	 * CollisionManager에서 매 프레임 호출됩니다.
	 *
	 * @param OtherComponents - 확인할 다른 Shape 컴포넌트들의 배열
	 */
	virtual void UpdateOverlaps(const TArray<UShapeComponent*>& OtherComponents);

	/**
	 * 특정 컴포넌트와의 Overlap 정보를 찾습니다.
	 *
	 * @param OtherComponent - 찾을 컴포넌트
	 * @return Overlap 정보 포인터 (없으면 nullptr)
	 */
	FOverlapInfo* FindOverlapInfo(const UShapeComponent* OtherComponent);

	/**
	 * 특정 컴포넌트와의 Overlap 정보를 제거합니다.
	 *
	 * @param OtherComponent - 제거할 컴포넌트
	 * @return 제거 성공 여부
	 */
	bool RemoveOverlapInfo(const UShapeComponent* OtherComponent);

	// ────────────────────────────────────────────────
	// Bounds 관련 함수
	// ────────────────────────────────────────────────

	/**
	 * Bounds를 업데이트합니다.
	 * Transform 변경 시 호출되어야 합니다.
	 */
	virtual void UpdateBounds();

	/**
	 * 스케일이 적용된 Bounds를 반환합니다.
	 *
	 * @return FBoxSphereBounds 구조체 (Box와 Sphere 정보 포함)
	 */
	virtual FBoxSphereBounds GetScaledBounds() const;

    FAABB GetWorldAABB() const override;
	virtual const TArray<FOverlapInfo>& GetOverlapInfos() const override { return OverlapInfos; }

	// Duplication
	virtual void DuplicateSubObjects() override;

	// ────────────────────────────────────────────────
	// Overlap 델리게이트
	// ────────────────────────────────────────────────

	/** Overlap 시작 시 호출되는 멀티캐스트 델리게이트 */
	FComponentOverlapSignature OnComponentBeginOverlap;

	/** Overlap 종료 시 호출되는 멀티캐스트 델리게이트 */
	FComponentOverlapSignature OnComponentEndOverlap;

	/** 현재 충돌 중인지 여부 (디버그 시각화용) */
	bool bIsOverlapping = false;

protected:
	/**
	 * Transform이 변경될 때 호출됩니다.
	 * CollisionManager에 Dirty 마킹합니다.
	 */
	virtual void OnTransformChanged();

protected:
	mutable FAABB WorldAABB; //브로드 페이즈 용
	TSet<UShapeComponent*> OverlapNow; // 이번 프레임에서 overlap 된 Shap Comps
	TSet<UShapeComponent*> OverlapPrev; // 지난 프레임에서 overlap 됐으면 Cache

	/** Cached scaled bounds */
	FBoxSphereBounds CachedBounds;

	FVector4 ShapeColor ;
	bool bDrawOnlyIfSelected;

	TArray<FOverlapInfo> OverlapInfos;
	//TODO: float LineThickness;

};
