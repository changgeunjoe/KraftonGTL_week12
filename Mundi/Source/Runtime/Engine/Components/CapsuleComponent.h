#pragma once

#include "ShapeComponent.h"
#include "UCapsuleComponent.generated.h"

UCLASS(DisplayName="캡슐 컴포넌트", Description="캡슐 모양 충돌 컴포넌트입니다")
class UCapsuleComponent : public UShapeComponent
{
public:

	GENERATED_REFLECTION_BODY();

	UCapsuleComponent();
	void OnRegister(UWorld* World) override;

	void SetCapsuleSize(float InRadius, float InHalfHeight) { CapsuleRadius = InRadius; CapsuleHalfHeight = InHalfHeight; UpdateBounds(); }
	float GetCapsuleRadius() const { return CapsuleRadius; }
	float GetCapsuleHalfHeight() const { return CapsuleHalfHeight; }
	float GetScaledCapsuleRadius() const;
	float GetScaledCapsuleHalfHeight() const;
	FVector GetCapsuleCenter() const;
	void GetCapsuleSegment(FVector& OutStart, FVector& OutEnd) const;

	// Bounds override
	virtual void UpdateBounds() override;
	virtual FBoxSphereBounds GetScaledBounds() const override;

	// Duplication
	virtual void DuplicateSubObjects() override;

protected:


	void GetShape(FShape& Out) const override; 

public:

    // ===== Lua-Bindable Properties (Auto-moved from protected/private) =====

	UPROPERTY(EditAnywhere, Category="CapsuleHalfHeight")
	float CapsuleHalfHeight;

	UPROPERTY(EditAnywhere, Category="CapsuleHalfHeight")
	float CapsuleRadius;
	void RenderDebugVolume(class URenderer* Renderer) const override;
};