#include "pch.h"
#include "STextBlock.h"
#include "ImGui/imgui.h"

STextBlock::STextBlock()
	: Color(0xFFFFFFFF)
	, Justification(Left)
	, FontSize(0.0f)
{
}

STextBlock::STextBlock(const FString& InText)
	: Text(InText)
	, Color(0xFFFFFFFF)
	, Justification(Left)
	, FontSize(0.0f)
{
}

STextBlock::~STextBlock()
{
}

void STextBlock::SetText(const FString& InText)
{
	Text = InText;
	TextDelegate = nullptr;
	Invalidate();
}

void STextBlock::SetText(std::function<FString()> InTextDelegate)
{
	TextDelegate = InTextDelegate;
	Text.clear();
	Invalidate();
}

FString STextBlock::GetText() const
{
	return GetDisplayText();
}

FString STextBlock::GetDisplayText() const
{
	if (TextDelegate)
	{
		return TextDelegate();
	}
	return Text;
}

void STextBlock::RenderContent()
{
	if (!bIsVisible)
		return;

	FString DisplayText = GetDisplayText();
	if (DisplayText.empty())
		return;

	ImDrawList* DrawList = ImGui::GetWindowDrawList();

	// 폰트 크기 설정 (옵션)
	ImFont* Font = nullptr;
	if (FontSize > 0.0f)
	{
		// 커스텀 폰트 사이즈 (현재는 기본 폰트 사용)
		// 나중에 폰트 시스템 확장 가능
	}

	// 텍스트 크기 계산
	ImVec2 TextSize = ImGui::CalcTextSize(DisplayText.c_str());

	// 정렬에 따른 위치 계산
	ImVec2 TextPos;
	TextPos.y = Rect.Top;

	switch (Justification)
	{
	case Left:
		TextPos.x = Rect.Left;
		break;

	case Center:
		TextPos.x = Rect.Left + (Rect.GetWidth() - TextSize.x) * 0.5f;
		break;

	case Right:
		TextPos.x = Rect.Right - TextSize.x;
		break;
	}

	// 텍스트 렌더링
	DrawList->AddText(TextPos, Color, DisplayText.c_str());
}
