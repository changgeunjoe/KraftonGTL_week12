#include "pch.h"
#include "SSeparator.h"
#include "ImGui/imgui.h"

SSeparator::SSeparator()
    : Orientation(Horizontal)
    , Thickness(1.0f)
    , Color(0xFF808080)
{
}

SSeparator::SSeparator(EOrientation InOrientation)
    : Orientation(InOrientation)
    , Thickness(1.0f)
    , Color(0xFF808080)
{
}

SSeparator::~SSeparator()
{
}

float SSeparator::GetHeight() const
{
    // 수평 구분선의 높이는 두께
    if (Orientation == Horizontal)
        return Thickness;
    else
        return 0.0f; // 수직 구분선은 부모가 결정
}

void SSeparator::RenderContent()
{
    if (!bIsVisible)
        return;

    if (Orientation == Horizontal)
    {
        // ImGui Separator 사용
        ImGui::Separator();
    }
    else
    {
        // 수직 구분선은 ImGui::SameLine() + ImGui::Separator()
        // 또는 직접 DrawList로 그리기
        ImDrawList* DrawList = ImGui::GetWindowDrawList();

        ImVec2 Pos = ImGui::GetCursorScreenPos();
        float Height = ImGui::GetContentRegionAvail().y;

        // 색상 변환 (RGBA -> ABGR for ImGui)
        ImU32 ImColor = IM_COL32(
            (Color >> 24) & 0xFF,  // R
            (Color >> 16) & 0xFF,  // G
            (Color >> 8) & 0xFF,   // B
            Color & 0xFF           // A
        );

        DrawList->AddLine(
            ImVec2(Pos.x, Pos.y),
            ImVec2(Pos.x, Pos.y + Height),
            ImColor,
            Thickness
        );

        // 커서 이동
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(Thickness, Height));
    }
}
