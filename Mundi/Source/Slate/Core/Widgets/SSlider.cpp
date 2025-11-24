#include "pch.h"
#include "SSlider.h"
#include "ImGui/imgui.h"
#include <string>
#include <algorithm>

SSlider::SSlider()
    : Value(0.0f)
    , MinValue(0.0f)
    , MaxValue(100.0f)
    , Width(200.0f)
    , Orientation(Horizontal)
    , Label("")
    , Format("%.1f")
{
}

SSlider::SSlider(float InMinValue, float InMaxValue)
    : Value(InMinValue)
    , MinValue(InMinValue)
    , MaxValue(InMaxValue)
    , Width(200.0f)
    , Orientation(Horizontal)
    , Label("")
    , Format("%.1f")
{
}

SSlider::~SSlider()
{
}

void SSlider::SetValue(float InValue)
{
    // 범위 클램핑
    float ClampedValue = std::max(MinValue, std::min(MaxValue, InValue));

    if (Value != ClampedValue)
    {
        Value = ClampedValue;

        // 델리게이트 호출
        if (OnValueChanged.IsBound())
        {
            OnValueChanged.Broadcast(Value);
        }

        Invalidate();
    }
}

FString SSlider::GetImGuiID() const
{
    // 고유 ID 생성 (메모리 주소 기반)
    return "##Slider_" + std::to_string(reinterpret_cast<uintptr_t>(this));
}

void SSlider::RenderContent()
{
    if (!bIsVisible)
        return;

    // 라벨 표시 (있으면)
    if (!Label.empty())
    {
        ImGui::Text("%s", Label.c_str());
        if (Orientation == Horizontal)
        {
            ImGui::SameLine();
        }
    }

    // 이전 값 저장 (변경 감지용)
    float PreviousValue = Value;

    // ImGui 슬라이더 렌더링
    bool bValueChanged = false;

    if (Orientation == Horizontal)
    {
        ImGui::SetNextItemWidth(Width);
        bValueChanged = ImGui::SliderFloat(GetImGuiID().c_str(), &Value, MinValue, MaxValue, Format.c_str());
    }
    else
    {
        // 수직 슬라이더
        bValueChanged = ImGui::VSliderFloat(GetImGuiID().c_str(), ImVec2(50, Width), &Value, MinValue, MaxValue, Format.c_str());
    }

    // 값이 변경된 경우
    if (bValueChanged && Value != PreviousValue)
    {
        if (OnValueChanged.IsBound())
        {
            OnValueChanged.Broadcast(Value);
        }
    }

    // 마우스 릴리즈 시 (값 확정)
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        if (OnValueCommitted.IsBound())
        {
            OnValueCommitted.Broadcast(Value);
        }
    }
}
