#include "pch.h"
#include "SComboBox.h"
#include "ImGui/imgui.h"
#include <string>

SComboBox::SComboBox()
    : SelectedIndex(-1)
    , Label("")
{
}

SComboBox::SComboBox(const TArray<FString>& InOptions)
    : Options(InOptions)
    , SelectedIndex(InOptions.IsEmpty() ? -1 : 0)
    , Label("")
{
}

SComboBox::~SComboBox()
{
}

void SComboBox::AddOption(const FString& Option)
{
    Options.Add(Option);

    // 첫 번째 옵션이면 자동 선택
    if (Options.Num() == 1)
    {
        SetSelectedIndex(0);
    }

    Invalidate();
}

void SComboBox::SetOptions(const TArray<FString>& InOptions)
{
    Options = InOptions;

    // 선택된 인덱스가 범위를 벗어나면 조정
    if (SelectedIndex >= static_cast<uint32>(Options.Num()))
    {
        SelectedIndex = Options.IsEmpty() ? -1 : 0;
    }

    Invalidate();
}

void SComboBox::ClearOptions()
{
    Options.Empty();
    SelectedIndex = -1;
    Invalidate();
}

void SComboBox::SetSelectedIndex(uint32 Index)
{
    if (Index >= 0 && Index < static_cast<uint32>(Options.Num()) && SelectedIndex != Index)
    {
        uint32 OldIndex = SelectedIndex;
        SelectedIndex = Index;

        // 델리게이트 호출
        if (OnSelectionChanged.IsBound())
        {
            OnSelectionChanged.Broadcast(SelectedIndex, GetSelectedValue());
        }

        Invalidate();
    }
}

FString SComboBox::GetSelectedValue() const
{
    if (SelectedIndex >= 0 && SelectedIndex < static_cast<uint32>(Options.Num()))
    {
        return Options[SelectedIndex];
    }
    return "";
}

FString SComboBox::GetImGuiID() const
{
    // 고유 ID 생성 (메모리 주소 기반)
    return "##ComboBox_" + std::to_string(reinterpret_cast<uintptr_t>(this));
}

void SComboBox::RenderContent()
{
    if (!bIsVisible)
        return;

    // 라벨 표시 (있으면)
    if (!Label.empty())
    {
        ImGui::Text("%s", Label.c_str());
        ImGui::SameLine();
    }

    // 현재 선택된 값 표시
    const char* PreviewValue = (SelectedIndex >= 0 && SelectedIndex < static_cast<uint32>(Options.Num()))
        ? Options[SelectedIndex].c_str()
        : "Select...";

    // ComboBox 시작
    if (ImGui::BeginCombo(GetImGuiID().c_str(), PreviewValue))
    {
        // 각 옵션 표시
        for (uint32 i = 0; i < static_cast<uint32>(Options.Num()); ++i)
        {
            bool bIsSelected = (i == SelectedIndex);

            if (ImGui::Selectable(Options[i].c_str(), bIsSelected))
            {
                SetSelectedIndex(i);
            }

            // 선택된 항목에 포커스
            if (bIsSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }
}
