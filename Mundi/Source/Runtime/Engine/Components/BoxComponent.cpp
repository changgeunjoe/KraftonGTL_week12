#include "pch.h"
#include "BoxComponent.h"
// IMPLEMENT_CLASS is now auto-generated in .generated.cpp
//BEGIN_PROPERTIES(UBoxComponent)
//MARK_AS_COMPONENT("박스 충돌 컴포넌트", "박스 모양의 충돌체를 생성하는 컴포넌트입니다.")
//	ADD_PROPERTY(FVector, BoxExtent, "BoxExtent", true, "박스 충돌체의 크기입니다.")
//END_PROPERTIES()

UBoxComponent::UBoxComponent()
{
	///BoxExtent = WorldAABB.GetHalfExtent(); 

	BoxExtent = FVector(0.5f, 0.5f, 0.5f); 
}

void UBoxComponent::OnRegister(UWorld* InWorld)
{ 
	Super::OnRegister(InWorld);

	// Owner의 실제 바운드를 가져옴
	if (AActor* Owner = GetOwner())
	{
		FAABB ActorBounds = Owner->GetBounds();
		FVector WorldHalfExtent = ActorBounds.GetHalfExtent();

		// World scale로 나눠서 local extent 계산
		const FTransform WordTransform = GetWorldTransform();
		const FVector S = FVector(
			std::fabs(WordTransform.Scale3D.X),
			std::fabs(WordTransform.Scale3D.Y),
			std::fabs(WordTransform.Scale3D.Z)
		);

		constexpr float Eps = 1e-6f;
		BoxExtent = FVector(
			S.X > Eps ? WorldHalfExtent.X / S.X : WorldHalfExtent.X,
			S.Y > Eps ? WorldHalfExtent.Y / S.Y : WorldHalfExtent.Y,
			S.Z > Eps ? WorldHalfExtent.Z / S.Z : WorldHalfExtent.Z
		);

	}
}

void UBoxComponent::DuplicateSubObjects()
{
	Super::DuplicateSubObjects();
}

// ────────────────────────────────────────────────────────────────────────────
// Bounds 관련 함수
// ────────────────────────────────────────────────────────────────────────────

void UBoxComponent::UpdateBounds()
{
	FVector ScaledExtent = GetScaledBoxExtent();
	FVector Center = GetWorldLocation();

	// 회전을 고려한 AABB 계산
	FQuat Rotation = GetWorldRotation();

	// 회전이 없으면 기존 방식 사용
	if (Rotation.IsIdentity())
	{
		CachedBounds = FBoxSphereBounds(Center, ScaledExtent);
		return;
	}

	// 회전된 Box의 8개 꼭짓점을 계산하여 AABB 구하기
	FVector Corners[8];
	Corners[0] = FVector(-ScaledExtent.X, -ScaledExtent.Y, -ScaledExtent.Z);
	Corners[1] = FVector(+ScaledExtent.X, -ScaledExtent.Y, -ScaledExtent.Z);
	Corners[2] = FVector(+ScaledExtent.X, +ScaledExtent.Y, -ScaledExtent.Z);
	Corners[3] = FVector(-ScaledExtent.X, +ScaledExtent.Y, -ScaledExtent.Z);
	Corners[4] = FVector(-ScaledExtent.X, -ScaledExtent.Y, +ScaledExtent.Z);
	Corners[5] = FVector(+ScaledExtent.X, -ScaledExtent.Y, +ScaledExtent.Z);
	Corners[6] = FVector(+ScaledExtent.X, +ScaledExtent.Y, +ScaledExtent.Z);
	Corners[7] = FVector(-ScaledExtent.X, +ScaledExtent.Y, +ScaledExtent.Z);

	// 회전 적용
	FVector Min = FVector(FLT_MAX, FLT_MAX, FLT_MAX);
	FVector Max = FVector(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (int32 i = 0; i < 8; ++i)
	{
		FVector RotatedCorner = Rotation.RotateVector(Corners[i]) + Center;
		Min.X = FMath::Min(Min.X, RotatedCorner.X);
		Min.Y = FMath::Min(Min.Y, RotatedCorner.Y);
		Min.Z = FMath::Min(Min.Z, RotatedCorner.Z);
		Max.X = FMath::Max(Max.X, RotatedCorner.X);
		Max.Y = FMath::Max(Max.Y, RotatedCorner.Y);
		Max.Z = FMath::Max(Max.Z, RotatedCorner.Z);
	}

	// AABB의 중심과 Extent 계산
	FVector AABBCenter = (Min + Max) * 0.5f;
	FVector AABBExtent = (Max - Min) * 0.5f;

	CachedBounds = FBoxSphereBounds(AABBCenter, AABBExtent);
}

FBoxSphereBounds UBoxComponent::GetScaledBounds() const
{
	return CachedBounds;
}

FVector UBoxComponent::GetScaledBoxExtent() const
{
	FVector Scale = GetWorldScale();
	return FVector(
		BoxExtent.X * FMath::Abs(Scale.X),
		BoxExtent.Y * FMath::Abs(Scale.Y),
		BoxExtent.Z * FMath::Abs(Scale.Z)
	);
}

void UBoxComponent::GetShape(FShape& Out) const
{
	Out.Kind = EShapeKind::Box;
	Out.Box.BoxExtent = BoxExtent;
}

void UBoxComponent::RenderDebugVolume(URenderer* Renderer) const
{
	// visible = 에디터용
	// hiddeningame = 파이용
	if (!GetOwner()) return;
	if (!GetOwner()->GetWorld()->bPie)
	{
		if (!bShapeIsVisible)
			return;
	}
	if (GetOwner()->GetWorld()->bPie)
	{
		if (bShapeHiddenInGame)
			return;
	}

	const FVector Extent = BoxExtent;
	const FTransform WorldTransform = GetWorldTransform();

	TArray<FVector> StartPoints;
	TArray<FVector> EndPoints;
	TArray<FVector4> Colors;

	FVector local[8] = {
		{-Extent.X, -Extent.Y, -Extent.Z}, {+Extent.X, -Extent.Y, -Extent.Z},
		{-Extent.X, +Extent.Y, -Extent.Z}, {+Extent.X, +Extent.Y, -Extent.Z},
		{-Extent.X, -Extent.Y, +Extent.Z}, {+Extent.X, -Extent.Y, +Extent.Z},
		{-Extent.X, +Extent.Y, +Extent.Z}, {+Extent.X, +Extent.Y, +Extent.Z},
	};

	//월드 space로 변환
	FVector WorldSpace[8]; 
	for (int i = 0; i < 8; i++)
	{ 
		WorldSpace[i] = WorldTransform.TransformPosition(local[i]);
	}

	static const int Edge[12][2] = {
		{0,1},{1,3},{3,2},{2,0}, // bottom
		{4,5},{5,7},{7,6},{6,4}, // top
		{0,4},{1,5},{2,6},{3,7}  // verticals
	};
	for (int i = 0; i < 12; ++i)
	{
		StartPoints.Add(WorldSpace[Edge[i][0]]);
		EndPoints.Add(WorldSpace[Edge[i][1]]);
		Colors.Add(ShapeColor); // 동일 색으로 라인 렌더
	}

	Renderer->AddLines(StartPoints, EndPoints, Colors);
}

