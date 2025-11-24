#include "pch.h"
#include "SListView.h"
#include "ImGui/imgui.h"
#include <algorithm>

SListView::SListView()
    : SelectedIndex(-1)
    , SelectionMode(Single)
    , ItemHeight(20.0f)
{
}

SListView::~SListView()
{
}

void SListView::AddItem(const FString& Item)
{
    Items.Add(Item);
    Invalidate();
}

void SListView::RemoveItem(uint32 Index)
{
    if (Index < static_cast<uint32>(Items.Num()))
    {
        Items.RemoveAt(Index);

        // 선택 인덱스 조정
        if (SelectedIndex == static_cast<int32>(Index))
        {
            SelectedIndex = -1;
        }
        else if (SelectedIndex > static_cast<int32>(Index))
        {
            SelectedIndex--;
        }

        // 다중 선택 인덱스 조정
        int32 FoundIdx = SelectedIndices.Find(Index);
        if (FoundIdx != -1)
        {
            SelectedIndices.RemoveAt(FoundIdx);
        }

        for (uint32& Idx : SelectedIndices)
        {
            if (Idx > Index)
                Idx--;
        }

        Invalidate();
    }
}

void SListView::ClearItems()
{
    Items.Empty();
    SelectedIndex = -1;
    SelectedIndices.Empty();
    Invalidate();
}

void SListView::SetSelectedIndex(uint32 Index)
{
    if (SelectionMode == None)
        return;

    if (Index >= -1 && Index < static_cast<uint32>(Items.Num()) && SelectedIndex != Index)
    {
        SelectedIndex = Index;

        if (SelectionMode == Single)
        {
            SelectedIndices.Empty();
            if (Index >= 0)
            {
                SelectedIndices.Add(Index);
            }
        }

        if (OnSelectionChanged.IsBound() && Index >= 0)
        {
            OnSelectionChanged.Broadcast(Index, Items[Index]);
        }

        Invalidate();
    }
}

FString SListView::GetSelectedItem() const
{
    if (SelectedIndex >= 0 && SelectedIndex < static_cast<uint32>(Items.Num()))
    {
        return Items[SelectedIndex];
    }
    return "";
}

void SListView::AddSelection(uint32 Index)
{
    if (SelectionMode != Multi)
        return;

    if (Index >= 0 && Index < static_cast<uint32>(Items.Num()))
    {
        if (!IsSelected(Index))
        {
            SelectedIndices.Add(Index);
            Invalidate();
        }
    }
}

void SListView::RemoveSelection(uint32 Index)
{
    int32 FoundIdx = SelectedIndices.Find(Index);
    if (FoundIdx != -1)
    {
        SelectedIndices.RemoveAt(FoundIdx);
        Invalidate();
    }
}

bool SListView::IsSelected(uint32 Index) const
{
    return SelectedIndices.Find(Index) != -1;
}

FString SListView::GetImGuiID() const
{
    return "##ListView_" + std::to_string(reinterpret_cast<uintptr_t>(this));
}

void SListView::RenderContent()
{
    if (!bIsVisible)
        return;

    // 리스트박스로 렌더링 (부모가 설정한 Rect 사용, 없으면 기본 크기)
    float Width = Rect.GetWidth();
    float Height = Rect.GetHeight();

    if (Width <= 0.0f) Width = ListWidth;
    if (Height <= 0.0f) Height = ListHeight;

    ImVec2 ListSize(Width, Height);

    // Rect 위치로 커서 이동
    ImGui::SetCursorScreenPos(ImVec2(Rect.Left, Rect.Top));

    if (ImGui::BeginListBox(GetImGuiID().c_str(), ListSize))
    {
        for (uint32 i = 0; i < static_cast<uint32>(Items.Num()); ++i)
        {
            bool bIsSelected = IsSelected(i);
            FString ItemLabel = Items[i];

            // Selectable 렌더링
            if (ImGui::Selectable(ItemLabel.c_str(), bIsSelected))
            {
                if (SelectionMode == Single)
                {
                    SetSelectedIndex(i);
                }
                else if (SelectionMode == Multi)
                {
                    // Ctrl 클릭: 토글
                    ImGuiIO& IO = ImGui::GetIO();
                    if (IO.KeyCtrl)
                    {
                        if (bIsSelected)
                            RemoveSelection(i);
                        else
                            AddSelection(i);
                    }
                    else
                    {
                        // 일반 클릭: 단일 선택
                        SelectedIndices.Empty();
                        AddSelection(i);
                        SelectedIndex = i;

                        if (OnSelectionChanged.IsBound())
                        {
                            OnSelectionChanged.Broadcast(i, Items[i]);
                        }
                    }
                }
            }

            // 더블 클릭 감지
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                if (OnItemDoubleClicked.IsBound())
                {
                    OnItemDoubleClicked.Broadcast(i, Items[i]);
                }
            }

            // 선택된 항목에 포커스
            if (bIsSelected && i == SelectedIndex)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndListBox();
    }
}
