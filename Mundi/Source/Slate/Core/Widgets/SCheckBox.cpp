#include "pch.h"
#include "SCheckBox.h"
#include "ImGui/imgui.h"
#include <string>

SCheckBox::SCheckBox()
    : bIsChecked(false)
    , Label("")
{
}

SCheckBox::SCheckBox(const FString& InLabel, bool bInChecked)
    : bIsChecked(bInChecked)
    , Label(InLabel)
{
}

SCheckBox::~SCheckBox()
{
}

void SCheckBox::SetChecked(bool bInChecked)
{
    if (bIsChecked != bInChecked)
    {
        bIsChecked = bInChecked;

        // 델리게이트 호출
        if (OnCheckStateChanged.IsBound())
        {
            OnCheckStateChanged.Broadcast(bIsChecked);
        }

        Invalidate();
    }
}

FString SCheckBox::GetImGuiID() const
{
    // 고유 ID 생성 (메모리 주소 기반)
    return "##CheckBox_" + std::to_string(reinterpret_cast<uintptr_t>(this));
}

void SCheckBox::RenderContent()
{
    if (!bIsVisible)
        return;

    // ImGui 체크박스 렌더링
    bool bCurrentState = bIsChecked;

    FString DisplayLabel = Label.empty() ? GetImGuiID() : (Label + GetImGuiID());

    if (ImGui::Checkbox(DisplayLabel.c_str(), &bCurrentState))
    {
        // 사용자가 체크박스를 클릭한 경우
        SetChecked(bCurrentState);
    }
}
