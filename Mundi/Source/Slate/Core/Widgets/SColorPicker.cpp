#include "pch.h"
#include "SColorPicker.h"
#include "ImGui/imgui.h"

SColorPicker::SColorPicker()
    : Color(0xFFFFFFFF)
    , bAlphaEnabled(true)
    , PickerMode(Compact)
    , Label("")
{
}

SColorPicker::SColorPicker(uint32_t InColor)
    : Color(InColor)
    , bAlphaEnabled(true)
    , PickerMode(Compact)
    , Label("")
{
}

SColorPicker::~SColorPicker()
{
}

void SColorPicker::SetColor(uint32_t InColor)
{
    if (Color != InColor)
    {
        Color = InColor;

        if (OnColorChanged.IsBound())
        {
            OnColorChanged.Broadcast(Color);
        }

        Invalidate();
    }
}

void SColorPicker::SetColorRGB(uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
    uint32_t NewColor = (R << 24) | (G << 16) | (B << 8) | A;
    SetColor(NewColor);
}

void SColorPicker::GetColorRGB(uint8_t& OutR, uint8_t& OutG, uint8_t& OutB, uint8_t& OutA) const
{
    OutR = (Color >> 24) & 0xFF;
    OutG = (Color >> 16) & 0xFF;
    OutB = (Color >> 8) & 0xFF;
    OutA = Color & 0xFF;
}

ImVec4 SColorPicker::ColorToImVec4() const
{
    float r = ((Color >> 24) & 0xFF) / 255.0f;
    float g = ((Color >> 16) & 0xFF) / 255.0f;
    float b = ((Color >> 8) & 0xFF) / 255.0f;
    float a = (Color & 0xFF) / 255.0f;
    return ImVec4(r, g, b, a);
}

void SColorPicker::ImVec4ToColor(const ImVec4& Vec)
{
    uint8_t r = static_cast<uint8_t>(Vec.x * 255.0f);
    uint8_t g = static_cast<uint8_t>(Vec.y * 255.0f);
    uint8_t b = static_cast<uint8_t>(Vec.z * 255.0f);
    uint8_t a = static_cast<uint8_t>(Vec.w * 255.0f);
    Color = (r << 24) | (g << 16) | (b << 8) | a;
}

FString SColorPicker::GetImGuiID() const
{
    return "##ColorPicker_" + std::to_string(reinterpret_cast<uintptr_t>(this));
}

void SColorPicker::RenderContent()
{
    if (!bIsVisible)
        return;

    // 라벨 표시
    if (!Label.empty())
    {
        ImGui::Text("%s", Label.c_str());
        ImGui::SameLine();
    }

    ImVec4 ColorVec = ColorToImVec4();
    ImVec4 PreviousColor = ColorVec;

    // 색상 피커 플래그
    ImGuiColorEditFlags Flags = ImGuiColorEditFlags_None;

    if (!bAlphaEnabled)
        Flags |= ImGuiColorEditFlags_NoAlpha;

    bool bColorChanged = false;

    if (PickerMode == Compact)
    {
        // 컴팩트 모드: 색상 박스 + RGB 슬라이더 (팝업 없이)
        Flags |= ImGuiColorEditFlags_NoLabel;
        if (bAlphaEnabled)
        {
            bColorChanged = ImGui::ColorEdit4(GetImGuiID().c_str(), &ColorVec.x, Flags);
        }
        else
        {
            bColorChanged = ImGui::ColorEdit3(GetImGuiID().c_str(), &ColorVec.x, Flags);
        }
    }
    else
    {
        // 전체 모드: 전체 색상 피커 표시
        Flags |= ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_DisplayHex;
        Flags |= ImGuiColorEditFlags_PickerHueWheel; // Wheel 스타일 사용

        // ColorPicker3/4 사용 (알파 채널 선택적)
        if (bAlphaEnabled)
        {
            bColorChanged = ImGui::ColorPicker4(GetImGuiID().c_str(), &ColorVec.x, Flags);
        }
        else
        {
            bColorChanged = ImGui::ColorPicker3(GetImGuiID().c_str(), &ColorVec.x, Flags);
        }
    }

    // 색상 변경 감지
    if (bColorChanged)
    {
        ImVec4ToColor(ColorVec);

        if (OnColorChanged.IsBound())
        {
            OnColorChanged.Broadcast(Color);
        }
    }

    // 색상 확정 (마우스 릴리즈)
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        if (OnColorCommitted.IsBound())
        {
            OnColorCommitted.Broadcast(Color);
        }
    }
}
