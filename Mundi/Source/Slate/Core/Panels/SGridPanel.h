#pragma once
#include "SPanel.h"
#include <vector>

/**
 * SGridPanel - 그리드 레이아웃 패널
 *
 * 특징:
 * - 행(Row)과 열(Column) 기반 레이아웃
 * - 각 셀에 위젯 배치 가능
 * - RowSpan, ColumnSpan 지원 (셀 병합)
 * - 행/열 크기 자동 조절
 *
 * 사용 예시:
 * auto Grid = new SGridPanel();
 * Grid->AddSlot(0, 0)  // Row 0, Column 0
 *     .AttachWidget(new STextBlock("Label:"));
 * Grid->AddSlot(0, 1)  // Row 0, Column 1
 *     .AttachWidget(new SEditableText());
 * Grid->AddSlot(1, 0)  // Row 1, Column 0
 *     .ColumnSpan(2)    // 2개 열 병합
 *     .AttachWidget(new SButton());
 */
class SGridPanel : public SPanel
{
public:
    struct FSlot
    {
        SWindow* Widget = nullptr;
        uint32 Row = 0;
        uint32 Column = 0;
        uint32 RowSpan = 1;
        uint32 ColumnSpan = 1;
        FMargin Padding;

        // HAlignment
        enum EHorizontalAlignment
        {
            HAlign_Fill,
            HAlign_Left,
            HAlign_Center,
            HAlign_Right
        };

        // VAlignment
        enum EVerticalAlignment
        {
            VAlign_Fill,
            VAlign_Top,
            VAlign_Center,
            VAlign_Bottom
        };

        EHorizontalAlignment HAlign = HAlign_Fill;
        EVerticalAlignment VAlign = VAlign_Fill;

        // Fluent API
        FSlot& AttachWidget(SWindow* InWidget)
        {
            Widget = InWidget;
            return *this;
        }

        FSlot& SetPadding(float Uniform)
        {
            Padding = FMargin(Uniform);
            return *this;
        }

        FSlot& SetPadding(float Horizontal, float Vertical)
        {
            Padding = FMargin(Horizontal, Vertical);
            return *this;
        }

        FSlot& SetPadding(float Left, float Top, float Right, float Bottom)
        {
            Padding = FMargin(Left, Top, Right, Bottom);
            return *this;
        }

        FSlot& SetRowSpan(uint32 InRowSpan)
        {
            RowSpan = InRowSpan;
            return *this;
        }

        FSlot& SetColumnSpan(uint32 InColumnSpan)
        {
            ColumnSpan = InColumnSpan;
            return *this;
        }

        FSlot& HAlignment(EHorizontalAlignment InHAlign)
        {
            HAlign = InHAlign;
            return *this;
        }

        FSlot& VAlignment(EVerticalAlignment InVAlign)
        {
            VAlign = InVAlign;
            return *this;
        }
    };

    SGridPanel();
    virtual ~SGridPanel();

    // ===== 슬롯 관리 =====
    FSlot& AddSlot(uint32 Row, uint32 Column);
    void RemoveSlot(uint32 Row, uint32 Column);
    void ClearSlots();

    // ===== 행/열 크기 설정 =====
    void SetColumnWidth(uint32 Column, float Width);
    void SetRowHeight(uint32 Row, float Height);

    // ===== 렌더링 =====
    virtual void RenderContent() override;
    virtual void ArrangeChildren() override;

private:
    TArray<FSlot> Slots;
    TArray<float> ColumnWidths;  // 각 열의 계산된 너비
    TArray<float> RowHeights;    // 각 행의 계산된 높이

    void CalculateGridSizes();
    uint32 GetMaxRow() const;
    uint32 GetMaxColumn() const;
};
