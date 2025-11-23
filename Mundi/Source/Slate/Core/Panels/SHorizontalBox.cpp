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
	Slots.Add(NewSlot);
	Invalidate();
	return Slots.back();
}

void SHorizontalBox::RemoveSlot(uint32 Index)
{
	if (Index < static_cast<uint32>(Slots.Num()))
	{
		// 자식 위젯도 제거
		if (Slots[Index].Widget)
		{
			RemoveChild(Slots[Index].Widget);
		}

		Slots.RemoveAt(Index);
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

	Slots.Empty();
	ComputedWidths.Empty();
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

	float Width = Slot.Widget->GetWidth() + Slot.Padding.Left + Slot.Padding.Right;

	// 최소 크기 보장 (SGridPanel과 동일하게)
	if (Width < 50.0f)
		Width = 50.0f;

	return Width;
}

void SHorizontalBox::CalculateSlotSizes()
{
	ComputedWidths.Empty();
	ComputedWidths.SetNum(Slots.Num(), 0.0f);

	float TotalWidth = Rect.GetWidth();
	float UsedWidth = 0.0f;
	float TotalFillWeight = 0.0f;

	// 디버그
	static int debugCount = 0;
	if (debugCount < 3)
	{
		UE_LOG("HBox CalculateSlotSizes: TotalWidth=%.1f, Slots=%d", TotalWidth, (int)Slots.Num());
		debugCount++;
	}

	// 1단계: Auto와 Fixed 크기 계산
	for (size_t i = 0; i < Slots.Num(); ++i)
	{
		const FSlot& Slot = Slots[i];

		if (Slot.SizeRule == SizeRule_Auto)
		{
			float Width = CalculateAutoWidth(Slot);
			ComputedWidths[i] = Width;
			UsedWidth += Width;
			if (debugCount <= 3)
			{
				UE_LOG("  HBox Slot[%d] Auto: Width=%.1f", (int)i, Width);
			}
		}
		else if (Slot.SizeRule == SizeRule_Fixed)
		{
			float Width = Slot.SizeValue + Slot.Padding.Left + Slot.Padding.Right;
			ComputedWidths[i] = Width;
			UsedWidth += Width;
			if (debugCount <= 3)
			{
				UE_LOG("  HBox Slot[%d] Fixed: Width=%.1f (SizeValue=%.1f)", (int)i, Width, Slot.SizeValue);
			}
		}
		else if (Slot.SizeRule == SizeRule_Fill)
		{
			TotalFillWeight += Slot.SizeValue;
			if (debugCount <= 3)
			{
				UE_LOG("  HBox Slot[%d] Fill: Weight=%.1f", (int)i, Slot.SizeValue);
			}
		}
	}

	// 2단계: Fill 크기 계산
	float RemainingWidth = std::max(0.0f, TotalWidth - UsedWidth);

	for (size_t i = 0; i < Slots.Num(); ++i)
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

	// 디버그 로그
	static int callCount = 0;
	if (callCount < 5)
	{
		float Width = Rect.GetWidth();
		float Height = Rect.GetHeight();
		UE_LOG("SHorizontalBox::ArrangeChildren() - Rect: (%.1f, %.1f) to (%.1f, %.1f), Size: (%.1f x %.1f), Slots: %d",
			Rect.Left, Rect.Top, Rect.Right, Rect.Bottom, Width, Height, (int)Slots.Num());
		callCount++;
	}

	for (size_t i = 0; i < Slots.Num(); ++i)
	{
		FSlot& Slot = Slots[i];
		if (!Slot.Widget)
			continue;

		// 자식으로 등록 (아직 안 되어있으면)
		if (Children.Find(Slot.Widget) == -1)
		
		{
			AddChild(Slot.Widget);
		}

		float SlotWidth = ComputedWidths[i];
		float ContentWidth = SlotWidth - Slot.Padding.Left - Slot.Padding.Right;

		// HAlignment 적용
		float ChildX = CurrentX + Slot.Padding.Left;
		float ChildWidth = 0.0f;

		if (Slot.HAlign == HAlign_Fill || Slot.SizeRule == SizeRule_Fill || Slot.SizeRule == SizeRule_Fixed)
		{
			// Fill 또는 Fixed인 경우 전체 너비 사용
			ChildWidth = ContentWidth;
		}
		else
		{
			// 위젯의 실제 너비 사용 (Auto인 경우)
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

void SHorizontalBox::RenderContent()
{
	// HorizontalBox는 자식들만 렌더링
}
