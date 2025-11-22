#pragma once
#include "Source/Slate/Core/Panels/SPanel.h"
#include <functional>

/**
 * STextBlock - 텍스트 표시 위젯
 * 정적 텍스트 또는 델리게이트를 통한 동적 텍스트 표시 가능
 */
class STextBlock : public SPanel
{
public:
	enum ETextJustify
	{
		Left,
		Center,
		Right
	};

	STextBlock();
	STextBlock(const FString& InText);
	virtual ~STextBlock();

	// ===== 텍스트 설정 =====
	void SetText(const FString& InText);
	void SetText(std::function<FString()> InTextDelegate);
	FString GetText() const;

	// ===== 스타일 설정 =====
	void SetColor(uint32_t InColor) { Color = InColor; Invalidate(); }
	uint32_t GetColor() const { return Color; }

	void SetJustification(ETextJustify InJustify) { Justification = InJustify; Invalidate(); }
	ETextJustify GetJustification() const { return Justification; }

	void SetFontSize(float InSize) { FontSize = InSize; Invalidate(); }
	float GetFontSize() const { return FontSize; }

	// ===== 렌더링 =====
	virtual void RenderContent() override;

private:
	FString GetDisplayText() const;

	FString Text;
	std::function<FString()> TextDelegate;
	uint32_t Color = 0xFFFFFFFF;  // ABGR 형식
	ETextJustify Justification = Left;
	float FontSize = 0.0f;  // 0.0f = 기본 크기
};
