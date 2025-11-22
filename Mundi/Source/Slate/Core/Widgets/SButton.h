#pragma once
#include "Source/Slate/Core/Panels/SPanel.h"
#include "Delegates.h"

/**
 * SButton - 클릭 가능한 버튼 위젯
 */
class SButton : public SPanel
{
public:
	struct FButtonStyle
	{
		uint32_t NormalColor = 0xFF3C3C3C;      // ABGR 형식
		uint32_t HoveredColor = 0xFF505050;
		uint32_t PressedColor = 0xFF282828;
		uint32_t DisabledColor = 0xFF1E1E1E;
		uint32_t TextColor = 0xFFFFFFFF;
		float Rounding = 4.0f;
	};

	SButton();
	virtual ~SButton();

	// ===== 설정 =====
	void SetText(const FString& InText) { Text = InText; Invalidate(); }
	FString GetText() const { return Text; }

	void SetSize(FVector2D InSize) { Size = InSize; Invalidate(); }
	FVector2D GetSize() const { return Size; }

	// Size를 기준으로 크기 반환
	virtual float GetWidth() const override { return Size.X; }
	virtual float GetHeight() const override { return Size.Y; }

	void SetButtonStyle(const FButtonStyle& InStyle) { Style = InStyle; Invalidate(); }
	const FButtonStyle& GetButtonStyle() const { return Style; }

	// ===== 이벤트 =====
	TDelegate<> OnClicked;
	TDelegate<> OnHovered;
	TDelegate<> OnUnhovered;

	// ===== 렌더링 =====
	virtual void RenderContent() override;

	// ===== 입력 =====
	virtual void OnMouseDown(FVector2D MousePos, uint32_t Button) override;
	virtual void OnMouseUp(FVector2D MousePos, uint32_t Button) override;
	virtual void OnMouseMove(FVector2D MousePos) override;

private:
	FString Text;
	FVector2D Size = FVector2D(100, 30);
	FButtonStyle Style;

	bool bIsHovered = false;
	bool bIsPressed = false;
};
