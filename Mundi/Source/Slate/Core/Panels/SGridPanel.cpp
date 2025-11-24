#include "pch.h"
#include "SGridPanel.h"

SGridPanel::SGridPanel()
{
}

SGridPanel::~SGridPanel()
{
    ClearSlots();
}

SGridPanel::FSlot& SGridPanel::AddSlot(uint32 Row, uint32 Column)
{
    FSlot NewSlot;
    NewSlot.Row = Row;
    NewSlot.Column = Column;
    Slots.Add(NewSlot);
    Invalidate();
    return Slots.back();
}

void SGridPanel::RemoveSlot(uint32 Row, uint32 Column)
{
    // 해당 Row, Column을 가진 슬롯 찾기 및 제거
    for (int32 i = Slots.Num() - 1; i >= 0; --i)
    {
        if (Slots[i].Row == Row && Slots[i].Column == Column)
        {
            if (Slots[i].Widget)
            {
                RemoveChild(Slots[i].Widget);
            }
            Slots.RemoveAt(i);
        }
    }
    Invalidate();
}

void SGridPanel::ClearSlots()
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
    ColumnWidths.Empty();
    RowHeights.Empty();
    Invalidate();
}

void SGridPanel::SetColumnWidth(uint32 Column, float Width)
{
    if (Column >= 0 && Column < static_cast<uint32>(ColumnWidths.Num()))
    {
        ColumnWidths[Column] = Width;
        Invalidate();
    }
}

void SGridPanel::SetRowHeight(uint32 Row, float Height)
{
    if (Row >= 0 && Row < static_cast<uint32>(RowHeights.Num()))
    {
        RowHeights[Row] = Height;
        Invalidate();
    }
}

uint32 SGridPanel::GetMaxRow() const
{
    uint32 MaxRow = 0;
    for (const FSlot& Slot : Slots)
    {
        uint32 EndRow = Slot.Row + Slot.RowSpan - 1;
        if (EndRow > MaxRow)
            MaxRow = EndRow;
    }
    return MaxRow;
}

uint32 SGridPanel::GetMaxColumn() const
{
    uint32 MaxColumn = 0;
    for (const FSlot& Slot : Slots)
    {
        uint32 EndColumn = Slot.Column + Slot.ColumnSpan - 1;
        if (EndColumn > MaxColumn)
            MaxColumn = EndColumn;
    }
    return MaxColumn;
}

void SGridPanel::CalculateGridSizes()
{
    uint32 NumRows = GetMaxRow() + 1;
    uint32 NumColumns = GetMaxColumn() + 1;

    ColumnWidths.Empty();
    RowHeights.Empty();

    ColumnWidths.SetNum(NumColumns, 0.0f);
    RowHeights.SetNum(NumRows, 0.0f);

    // 각 열/행의 최대 크기 계산
    for (const FSlot& Slot : Slots)
    {
        if (!Slot.Widget)
            continue;

        // ColumnSpan/RowSpan이 1인 경우에만 자동 크기 계산
        if (Slot.ColumnSpan == 1)
        {
            float WidgetWidth = Slot.Widget->GetWidth() + Slot.Padding.Left + Slot.Padding.Right;
            if (WidgetWidth > ColumnWidths[Slot.Column])
                ColumnWidths[Slot.Column] = WidgetWidth;
        }

        if (Slot.RowSpan == 1)
        {
            float WidgetHeight = Slot.Widget->GetHeight() + Slot.Padding.Top + Slot.Padding.Bottom;
            if (WidgetHeight > RowHeights[Slot.Row])
                RowHeights[Slot.Row] = WidgetHeight;
        }
    }

    // 최소 크기 보장
    for (float& Width : ColumnWidths)
    {
        if (Width < 50.0f)
            Width = 50.0f;
    }

    for (float& Height : RowHeights)
    {
        if (Height < 30.0f)
            Height = 30.0f;
    }

    // 전체 크기가 Panel 크기를 초과하면 비율 조정
    float TotalWidth = 0.0f;
    for (float Width : ColumnWidths)
        TotalWidth += Width;

    float TotalHeight = 0.0f;
    for (float Height : RowHeights)
        TotalHeight += Height;

    float PanelWidth = Rect.GetWidth();
    float PanelHeight = Rect.GetHeight();

    if (TotalWidth > PanelWidth && PanelWidth > 0.0f)
    {
        float Scale = PanelWidth / TotalWidth;
        for (float& Width : ColumnWidths)
            Width *= Scale;
    }

    if (TotalHeight > PanelHeight && PanelHeight > 0.0f)
    {
        float Scale = PanelHeight / TotalHeight;
        for (float& Height : RowHeights)
            Height *= Scale;
    }
}

void SGridPanel::ArrangeChildren()
{
    if (Slots.empty())
        return;

    CalculateGridSizes();

    // 각 슬롯의 위젯 배치
    for (FSlot& Slot : Slots)
    {
        if (!Slot.Widget)
            continue;

        // 자식으로 등록 (아직 안 되어있으면)
        if (Children.Find(Slot.Widget) == -1)
        
        {
            AddChild(Slot.Widget);
        }

        // 셀 위치 계산
        float CellX = Rect.Left;
        for (uint32 c = 0; c < Slot.Column; ++c)
        {
            if (c < static_cast<uint32>(ColumnWidths.Num()))
                CellX += ColumnWidths[c];
        }

        float CellY = Rect.Top;
        for (uint32 r = 0; r < Slot.Row; ++r)
        {
            if (r < static_cast<uint32>(RowHeights.Num()))
                CellY += RowHeights[r];
        }

        // 셀 크기 계산 (Span 고려)
        float CellWidth = 0.0f;
        for (uint32 c = Slot.Column; c < Slot.Column + Slot.ColumnSpan; ++c)
        {
            if (c < static_cast<uint32>(ColumnWidths.Num()))
                CellWidth += ColumnWidths[c];
        }

        float CellHeight = 0.0f;
        for (uint32 r = Slot.Row; r < Slot.Row + Slot.RowSpan; ++r)
        {
            if (r < static_cast<uint32>(RowHeights.Num()))
                CellHeight += RowHeights[r];
        }

        // Padding 적용
        float ContentX = CellX + Slot.Padding.Left;
        float ContentY = CellY + Slot.Padding.Top;
        float ContentWidth = CellWidth - Slot.Padding.Left - Slot.Padding.Right;
        float ContentHeight = CellHeight - Slot.Padding.Top - Slot.Padding.Bottom;

        // Alignment 적용
        float WidgetX = ContentX;
        float WidgetY = ContentY;
        float WidgetWidth = 0.0f;
        float WidgetHeight = 0.0f;

        // 수평 정렬
        if (Slot.HAlign == FSlot::HAlign_Fill)
        {
            WidgetWidth = ContentWidth;
        }
        else
        {
            WidgetWidth = Slot.Widget->GetWidth();
            if (WidgetWidth > ContentWidth)
                WidgetWidth = ContentWidth;

            if (Slot.HAlign == FSlot::HAlign_Center)
            {
                WidgetX += (ContentWidth - WidgetWidth) * 0.5f;
            }
            else if (Slot.HAlign == FSlot::HAlign_Right)
            {
                WidgetX += ContentWidth - WidgetWidth;
            }
        }

        // 수직 정렬
        if (Slot.VAlign == FSlot::VAlign_Fill)
        {
            WidgetHeight = ContentHeight;
        }
        else
        {
            WidgetHeight = Slot.Widget->GetHeight();
            if (WidgetHeight > ContentHeight)
                WidgetHeight = ContentHeight;

            if (Slot.VAlign == FSlot::VAlign_Center)
            {
                WidgetY += (ContentHeight - WidgetHeight) * 0.5f;
            }
            else if (Slot.VAlign == FSlot::VAlign_Bottom)
            {
                WidgetY += ContentHeight - WidgetHeight;
            }
        }

        // 위젯 배치
        Slot.Widget->SetRect(
            WidgetX,
            WidgetY,
            WidgetX + WidgetWidth,
            WidgetY + WidgetHeight
        );
    }
}

void SGridPanel::RenderContent()
{
    // 그리드 패널은 자식들만 렌더링
    // 필요시 여기에 디버그용 그리드 라인 그리기 추가 가능
}
