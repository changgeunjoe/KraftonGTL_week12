#include "pch.h"
#include "SHorizontalBox.h"
#include <algorithm>

SHorizontalBox::SHorizontalBox()
{
}

SHorizontalBox::~SHorizontalBox()
{
	ClearSlots();
}

SHorizontalBox::FSlot& SHorizontalBox::AddSlot()
{
	FSlot NewSlot;
	Slots.push_back(NewSlot);
	Invalidate();
	return Slots.back();
}

void SHorizontalBox::RemoveSlot(int32_t Index)
{
	if (Index >= 0 && Index < static_cast<int32_t>(Slots.size()))
	{
		// 자식 위젯도 제거
		if (Slots[Index].Widget)
		{
			RemoveChild(Slots[Index].Widget);
		}

		Slots.erase(Slots.begin() + Index);
		Invalidate();
	}
}

void SHorizontalBox::ClearSlots()
{
	// 모든 자식 위젯 제거
	for (FSlot& Slot : Slots)
	{
		if (Slot.Widget)
		{
			RemoveChild(Slot.Widget);
		}
	}

	Slots.clear();
	ComputedWidths.clear();
	Invalidate();
}

void SHorizontalBox::OnPanelResized()
{
	// 크기가 변경되면 재배치
	ArrangeChildren();
}

float SHorizontalBox::CalculateAutoWidth(const FSlot& Slot) const
{
	if (!Slot.Widget)
		return 0.0f;

	return Slot.Widget->GetWidth() + Slot.Padding.Left + Slot.Padding.Right;
}

void SHorizontalBox::CalculateSlotSizes()
{
	ComputedWidths.clear();
	ComputedWidths.resize(Slots.size(), 0.0f);

	float TotalWidth = Rect.GetWidth();
	float UsedWidth = 0.0f;
	float TotalFillWeight = 0.0f;

	// 1단계: Auto와 Fixed 크기 계산
	for (size_t i = 0; i < Slots.size(); ++i)
	{
		const FSlot& Slot = Slots[i];

		if (Slot.SizeRule == SizeRule_Auto)
		{
			float Width = CalculateAutoWidth(Slot);
			ComputedWidths[i] = Width;
			UsedWidth += Width;
		}
		else if (Slot.SizeRule == SizeRule_Fixed)
		{
			float Width = Slot.SizeValue + Slot.Padding.Left + Slot.Padding.Right;
			ComputedWidths[i] = Width;
			UsedWidth += Width;
		}
		else if (Slot.SizeRule == SizeRule_Fill)
		{
			TotalFillWeight += Slot.SizeValue;
		}
	}

	// 2단계: Fill 크기 계산
	float RemainingWidth = std::max(0.0f, TotalWidth - UsedWidth);

	for (size_t i = 0; i < Slots.size(); ++i)
	{
		const FSlot& Slot = Slots[i];

		if (Slot.SizeRule == SizeRule_Fill)
		{
			if (TotalFillWeight > 0.0f)
			{
				float Ratio = Slot.SizeValue / TotalFillWeight;
				ComputedWidths[i] = RemainingWidth * Ratio;
			}
			else
			{
				ComputedWidths[i] = 0.0f;
			}
		}
	}
}

void SHorizontalBox::ArrangeChildren()
{
	if (Slots.empty())
		return;

	CalculateSlotSizes();

	float CurrentX = Rect.Left;

	for (size_t i = 0; i < Slots.size(); ++i)
	{
		FSlot& Slot = Slots[i];
		if (!Slot.Widget)
			continue;

		// 자식으로 등록 (아직 안 되어있으면)
		auto it = std::find(Children.begin(), Children.end(), Slot.Widget);
		if (it == Children.end())
		{
			AddChild(Slot.Widget);
		}

		float SlotWidth = ComputedWidths[i];
		float ContentWidth = SlotWidth - Slot.Padding.Left - Slot.Padding.Right;

		// HAlignment 적용
		float ChildX = CurrentX + Slot.Padding.Left;
		float ChildWidth = 0.0f;

		if (Slot.HAlign == HAlign_Fill || Slot.SizeRule == SizeRule_Fill)
		{
			// Fill인 경우 전체 너비 사용
			ChildWidth = ContentWidth;
		}
		else
		{
			// 위젯의 실제 너비 사용
			ChildWidth = Slot.Widget->GetWidth();

			// 정렬 적용
			if (Slot.HAlign == HAlign_Center)
			{
				ChildX += (ContentWidth - ChildWidth) * 0.5f;
			}
			else if (Slot.HAlign == HAlign_Right)
			{
				ChildX += ContentWidth - ChildWidth;
			}
		}

		// VAlignment 적용
		float ChildY = Rect.Top + Slot.Padding.Top;
		float ChildHeight = 0.0f;
		float ContentHeight = Rect.GetHeight() - Slot.Padding.Top - Slot.Padding.Bottom;

		if (Slot.VAlign == VAlign_Fill)
		{
			// Fill인 경우 전체 높이 사용
			ChildHeight = ContentHeight;
		}
		else
		{
			// 위젯의 실제 높이 사용
			ChildHeight = Slot.Widget->GetHeight();

			// 정렬 적용
			if (Slot.VAlign == VAlign_Center)
			{
				ChildY += (ContentHeight - ChildHeight) * 0.5f;
			}
			else if (Slot.VAlign == VAlign_Bottom)
			{
				ChildY += ContentHeight - ChildHeight;
			}
		}

		// 위젯 배치
		Slot.Widget->SetRect(
			ChildX,
			ChildY,
			ChildX + ChildWidth,
			ChildY + ChildHeight
		);

		CurrentX += SlotWidth;
	}
}
