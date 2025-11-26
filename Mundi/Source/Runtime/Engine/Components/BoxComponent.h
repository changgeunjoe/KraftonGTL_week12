#pragma once

#include "ShapeComponent.h"
#include "UBoxComponent.generated.h"

UCLASS(DisplayName="박스 컴포넌트", Description="박스 모양 충돌 컴포넌트입니다")
class UBoxComponent : public UShapeComponent
{
public:

	GENERATED_REFLECTION_BODY();

	UBoxComponent();
	void OnRegister(UWorld* InWorld) override;

	void SetBoxExtent(const FVector& InExtent) { BoxExtent = InExtent; UpdateBounds(); }
	FVector GetBoxExtent() const { return BoxExtent; }
	FVector GetScaledBoxExtent() const;

	// Bounds override
	virtual void UpdateBounds() override;
	virtual FBoxSphereBounds GetScaledBounds() const override;

	// Duplication
	virtual void DuplicateSubObjects() override;

private:

	void GetShape(FShape& Out) const override;

public:

    // ===== Lua-Bindable Properties (Auto-moved from protected/private) =====

	UPROPERTY(EditAnywhere, Category="BoxExtent")
	FVector BoxExtent; // Half Extent

	void RenderDebugVolume(class URenderer* Renderer) const override;
};