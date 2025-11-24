#pragma once
#include "SPanel.h"
#include <vector>
#include <algorithm>

/**
 * SVerticalBox - 수직 방향으로 위젯을 배치하는 레이아웃 패널
 * 각 슬롯은 Auto, Fill, Fixed 크기 규칙을 가질 수 있습니다.
 */
class SVerticalBox : public SPanel
{
public:
	enum EVerticalAlignment
	{
		VAlign_Top,
		VAlign_Center,
		VAlign_Bottom,
		VAlign_Fill
	};

	enum ESizeRule
	{
		SizeRule_Auto,      // 자식 크기에 맞춤
		SizeRule_Fill,      // 남은 공간 채움 (비율)
		SizeRule_Fixed      // 고정 픽셀 크기
	};

	/**
	 * FSlot - 각 자식 위젯의 배치 정보를 담는 슬롯
	 */
	struct FSlot
	{
		SWindow* Widget = nullptr;  // SWindow*로 변경 (SPanel, SCompoundWidget 모두 수용)
		ESizeRule SizeRule = SizeRule_Auto;
		float SizeValue = 1.0f;
		EVerticalAlignment VAlign = VAlign_Top;
		FMargin Padding = FMargin(0, 0, 0, 0);

		// Fluent API - 메서드 체이닝을 위한 빌더 패턴
		FSlot& AutoHeight()
		{
			SizeRule = SizeRule_Auto;
			return *this;
		}

		FSlot& FillHeight(float Ratio = 1.0f)
		{
			SizeRule = SizeRule_Fill;
			SizeValue = Ratio;
			return *this;
		}

		FSlot& FixedHeight(float Height)
		{
			SizeRule = SizeRule_Fixed;
			SizeValue = Height;
			return *this;
		}

		FSlot& SetPadding(float Uniform)
		{
			Padding = FMargin(Uniform, Uniform, Uniform, Uniform);
			return *this;
		}

		FSlot& SetPadding(float Horizontal, float Vertical)
		{
			Padding = FMargin(Horizontal, Vertical, Horizontal, Vertical);
			return *this;
		}

		FSlot& SetPadding(float Left, float Top, float Right, float Bottom)
		{
			Padding = FMargin(Left, Top, Right, Bottom);
			return *this;
		}

		FSlot& VAlignment(EVerticalAlignment InAlign)
		{
			VAlign = InAlign;
			return *this;
		}

		FSlot& AttachWidget(SWindow* InWidget)  // SWindow*로 변경
		{
			Widget = InWidget;
			return *this;
		}

		FSlot& operator[](SWindow* InWidget)  // SWindow*로 변경
		{
			Widget = InWidget;
			return *this;
		}
	};

	SVerticalBox();
	virtual ~SVerticalBox();

	// ===== Slot 관리 =====
	FSlot& AddSlot();
	void RemoveSlot(uint32 Index);
	void ClearSlots();
	uint32 GetSlotCount() const { return static_cast<uint32>(Slots.Num()); }

	// ===== 레이아웃 =====
	virtual void ArrangeChildren() override;
	virtual void OnPanelResized() override;
	virtual void RenderContent() override;

private:
	void CalculateSlotSizes();
	float CalculateAutoHeight(const FSlot& Slot) const;
	TArray<FSlot> Slots;
	TArray<float> ComputedHeights;  // 각 슬롯의 계산된 높이
};
