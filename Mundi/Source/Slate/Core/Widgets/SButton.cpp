#include "pch.h"
#include "SButton.h"
#include "ImGui/imgui.h"

SButton::SButton()
	: bIsHovered(false)
	, bIsPressed(false)
{
}

SButton::~SButton()
{
}

void SButton::RenderContent()
{
	if (!bIsVisible)
		return;

	ImDrawList* DrawList = ImGui::GetWindowDrawList();

	// Rect에서 크기 자동 계산
	float Width = Rect.GetWidth();
	float Height = Rect.GetHeight();

	ImVec2 ButtonMin(Rect.Left, Rect.Top);
	ImVec2 ButtonMax(Rect.Right, Rect.Bottom);

	// 상태에 따른 색상 선택
	uint32_t BgColor;
	if (!bIsEnabled)
	{
		BgColor = Style.DisabledColor;
	}
	else if (bIsPressed)
	{
		BgColor = Style.PressedColor;
	}
	else if (bIsHovered)
	{
		BgColor = Style.HoveredColor;
	}
	else
	{
		BgColor = Style.NormalColor;
	}

	// 배경 렌더링
	DrawList->AddRectFilled(ButtonMin, ButtonMax, BgColor, Style.Rounding);

	// 테두리 렌더링
	DrawList->AddRect(ButtonMin, ButtonMax, 0xFF646464, Style.Rounding);

	// 텍스트 렌더링
	if (!Text.empty())
	{
		ImVec2 TextSize = ImGui::CalcTextSize(Text.c_str());
		float Width = Rect.GetWidth();
		float Height = Rect.GetHeight();
		ImVec2 TextPos(
			ButtonMin.x + (Width - TextSize.x) * 0.5f,
			ButtonMin.y + (Height - TextSize.y) * 0.5f
		);
		DrawList->AddText(TextPos, Style.TextColor, Text.c_str());
	}
}

void SButton::OnMouseDown(FVector2D MousePos, uint32_t Button)
{
	// 디버깅용 로그
	bool bHover = IsHover(MousePos);
	printf("[SButton] MouseDown - Pos(%.1f, %.1f) | Rect(%.1f, %.1f, %.1f, %.1f) | Hover: %d | Text: %s\n",
		MousePos.X, MousePos.Y,
		Rect.Left, Rect.Top, Rect.Right, Rect.Bottom,
		bHover, Text.c_str());

	if (bHover && bIsEnabled)
	{
		bIsPressed = true;
		Invalidate();
	}
}

void SButton::OnMouseUp(FVector2D MousePos, uint32_t Button)
{
	if (bIsPressed && IsHover(MousePos) && bIsEnabled)
	{
		// 클릭 이벤트 발생 (기존 델리게이트 사용)
		OnClicked.Broadcast();
	}

	bIsPressed = false;
	Invalidate();
}

void SButton::OnMouseMove(FVector2D MousePos)
{
	bool bWasHovered = bIsHovered;
	bIsHovered = IsHover(MousePos);

	if (bIsHovered != bWasHovered)
	{
		if (bIsHovered)
		{
			OnHovered.Broadcast();
		}
		else
		{
			OnUnhovered.Broadcast();
		}

		Invalidate();
	}
}
