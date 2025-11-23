#include "pch.h"
#include "SVerticalBox.h"
#include <algorithm>

SVerticalBox::SVerticalBox()
{
}

SVerticalBox::~SVerticalBox()
{
	ClearSlots();
}

SVerticalBox::FSlot& SVerticalBox::AddSlot()
{
	FSlot NewSlot;
	Slots.Add(NewSlot);
	Invalidate();
	return Slots.Last();
}

void SVerticalBox::RemoveSlot(uint32 Index)
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

void SVerticalBox::ClearSlots()
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
	ComputedHeights.Empty();
	Invalidate();
}

void SVerticalBox::OnPanelResized()
{
	// 크기가 변경되면 재배치
	ArrangeChildren();
}

float SVerticalBox::CalculateAutoHeight(const FSlot& Slot) const
{
	if (!Slot.Widget)
		return 0.0f;

	float Height = Slot.Widget->GetHeight() + Slot.Padding.Top + Slot.Padding.Bottom;

	// 최소 크기 보장 (SGridPanel과 동일하게)
	if (Height < 30.0f)
		Height = 30.0f;

	return Height;
}

void SVerticalBox::CalculateSlotSizes()
{
	ComputedHeights.Empty();
	ComputedHeights.SetNum(Slots.Num(), 0.0f);

	float TotalHeight = Rect.GetHeight();
	float UsedHeight = 0.0f;
	float TotalFillWeight = 0.0f;

	// 디버그
	static int debugCount = 0;
	if (debugCount < 3)
	{
		UE_LOG("CalculateSlotSizes: TotalHeight=%.1f, Slots=%d", TotalHeight, (int)Slots.Num());
		debugCount++;
	}

	// 1단계: Auto와 Fixed 크기 계산
	for (uint32 i = 0; i < static_cast<uint32>(Slots.Num()); ++i)
	{
		const FSlot& Slot = Slots[i];

		if (Slot.SizeRule == SizeRule_Auto)
		{
			float Height = CalculateAutoHeight(Slot);
			ComputedHeights[i] = Height;
			UsedHeight += Height;
			if (debugCount <= 3)
			{
				UE_LOG("  Slot[%d] Auto: Height=%.1f", (int)i, Height);
			}
		}
		else if (Slot.SizeRule == SizeRule_Fixed)
		{
			float Height = Slot.SizeValue + Slot.Padding.Top + Slot.Padding.Bottom;
			ComputedHeights[i] = Height;
			UsedHeight += Height;
			if (debugCount <= 3)
			{
				UE_LOG("  Slot[%d] Fixed: Height=%.1f (SizeValue=%.1f)", (int)i, Height, Slot.SizeValue);
			}
		}
		else if (Slot.SizeRule == SizeRule_Fill)
		{
			TotalFillWeight += Slot.SizeValue;
			if (debugCount <= 3)
			{
				UE_LOG("  Slot[%d] Fill: Weight=%.1f", (int)i, Slot.SizeValue);
			}
		}
	}

	// 2단계: Fill 크기 계산
	float RemainingHeight = std::max(0.0f, TotalHeight - UsedHeight);

	if (debugCount <= 3)
	{
		UE_LOG("  RemainingHeight=%.1f (Total=%.1f - Used=%.1f), TotalFillWeight=%.1f",
			RemainingHeight, TotalHeight, UsedHeight, TotalFillWeight);
	}

	// 공간이 부족하면 Fixed/Auto 모두 동일하게 비율적으로 축소
	if (UsedHeight > TotalHeight && TotalHeight > 0.0f)
	{
		float Scale = TotalHeight / UsedHeight;
		for (uint32 i = 0; i < static_cast<uint32>(Slots.Num()); ++i)
		{
			if (Slots[i].SizeRule != SizeRule_Fill)
			{
				ComputedHeights[i] *= Scale;
			}
		}
		RemainingHeight = 0.0f;
	}

	for (uint32 i = 0; i < static_cast<uint32>(Slots.Num()); ++i)
	{
		const FSlot& Slot = Slots[i];

		if (Slot.SizeRule == SizeRule_Fill)
		{
			if (TotalFillWeight > 0.0f)
			{
				float Ratio = Slot.SizeValue / TotalFillWeight;
				ComputedHeights[i] = RemainingHeight * Ratio;
				if (debugCount <= 3)
				{
					UE_LOG("  Slot[%d] Fill Result: Height=%.1f (Ratio=%.2f)", (int)i, ComputedHeights[i], Ratio);
				}
			}
			else
			{
				ComputedHeights[i] = 0.0f;
			}
		}
	}
}

void SVerticalBox::ArrangeChildren()
{
	if (Slots.empty())
		return;

	CalculateSlotSizes();

	float CurrentY = Rect.Top;

	// 디버그 로그
	static int callCount = 0;
	if (callCount < 5) // 처음 5번만 로그
	{
		float Width = Rect.GetWidth();
		float Height = Rect.GetHeight();
		UE_LOG("SVerticalBox::ArrangeChildren() - Rect: (%.1f, %.1f) to (%.1f, %.1f), Size: (%.1f x %.1f), Slots: %d",
			Rect.Left, Rect.Top, Rect.Right, Rect.Bottom, Width, Height, (int)Slots.Num());
		callCount++;
	}

	for (uint32 i = 0; i < static_cast<uint32>(Slots.Num()); ++i)
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

		float SlotHeight = ComputedHeights[i];
		float ContentHeight = SlotHeight - Slot.Padding.Top - Slot.Padding.Bottom;

		// VAlignment 적용
		float ChildY = CurrentY + Slot.Padding.Top;
		float ChildHeight = 0.0f;

		if (Slot.VAlign == VAlign_Fill || Slot.SizeRule == SizeRule_Fill || Slot.SizeRule == SizeRule_Fixed)
		{
			// Fill 또는 Fixed인 경우 전체 높이 사용
			ChildHeight = ContentHeight;
		}
		else
		{
			// 위젯의 실제 높이 사용 (Auto인 경우)
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
		float ChildX = Rect.Left + Slot.Padding.Left;
		float ChildWidth = Rect.GetWidth() - Slot.Padding.Left - Slot.Padding.Right;

		Slot.Widget->SetRect(
			ChildX,
			ChildY,
			ChildX + ChildWidth,
			ChildY + ChildHeight
		);

		// 디버그 로그
		if (callCount <= 5)
		{
			UE_LOG("  Slot[%d]: Rect(%.1f, %.1f, %.1f, %.1f), Height: %.1f",
				(int)i, ChildX, ChildY, ChildX + ChildWidth, ChildY + ChildHeight, SlotHeight);
		}

		CurrentY += SlotHeight;
	}
}

void SVerticalBox::RenderContent()
{
	// VerticalBox는 자식들만 렌더링
}
