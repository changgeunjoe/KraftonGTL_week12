#include "pch.h"
#include "SScrollBox.h"
#include "ImGui/imgui.h"
#include <string>

SScrollBox::SScrollBox()
    : bEnableVerticalScroll(true)
    , bEnableHorizontalScroll(false)
    , bAlwaysShowScrollbar(false)
{
}

SScrollBox::~SScrollBox()
{
}

void SScrollBox::SetScrollbarVisibility(bool bVertical, bool bHorizontal)
{
    bEnableVerticalScroll = bVertical;
    bEnableHorizontalScroll = bHorizontal;
    Invalidate();
}

void SScrollBox::ScrollToTop()
{
    // ImGui의 스크롤 설정은 다음 프레임에 적용됨
    // RenderContent에서 ImGui::SetScrollY(0) 호출 필요
}

void SScrollBox::ScrollToBottom()
{
    // RenderContent에서 ImGui::SetScrollY(ImGui::GetScrollMaxY()) 호출 필요
}

void SScrollBox::SetScrollY(float Y)
{
    // RenderContent에서 ImGui::SetScrollY(Y) 호출 필요
}

float SScrollBox::GetScrollY() const
{
    return ImGui::GetScrollY();
}

FString SScrollBox::GetImGuiID() const
{
    // 고유 ID 생성 (메모리 주소 기반)
    return "##ScrollBox_" + std::to_string(reinterpret_cast<uintptr_t>(this));
}

void SScrollBox::RenderContent()
{
    if (!bIsVisible)
        return;

    // ImGui BeginChild로 스크롤 영역 생성
    ImGuiWindowFlags Flags = 0;

    if (bEnableHorizontalScroll)
        Flags |= ImGuiWindowFlags_HorizontalScrollbar;

    if (bAlwaysShowScrollbar)
        Flags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;

    // 스크롤 영역 시작
    ImVec2 Size(Rect.GetWidth(), Rect.GetHeight());
    ImGui::SetCursorScreenPos(ImVec2(Rect.Left, Rect.Top));

    if (ImGui::BeginChild(GetImGuiID().c_str(), Size, false, Flags))
    {
        // 자식 위젯들을 먼저 배치
        ArrangeChildren();

        // 자식들 렌더링
        for (SWindow* Child : Children)
        {
            if (Child)
            {
                // SPanel인 경우에만 가시성 체크
                SPanel* ChildPanel = dynamic_cast<SPanel*>(Child);
                if (!ChildPanel || ChildPanel->IsVisible())
                {
                    Child->OnRender();
                }
            }
        }
    }
    ImGui::EndChild();
}

void SScrollBox::ArrangeChildren()
{
    if (Children.empty())
        return;

    // 스크롤박스는 자식들을 수직으로 배치
    float CurrentY = Rect.Top;
    float MaxWidth = Rect.GetWidth();

    for (SWindow* Child : Children)
    {
        if (!Child)
            continue;

        float ChildHeight = Child->GetHeight();
        if (ChildHeight == 0.0f)
            ChildHeight = 30.0f; // 기본 높이

        float ChildWidth = Child->GetWidth();
        if (ChildWidth == 0.0f || ChildWidth > MaxWidth)
            ChildWidth = MaxWidth;

        Child->SetRect(
            Rect.Left,
            CurrentY,
            Rect.Left + ChildWidth,
            CurrentY + ChildHeight
        );

        CurrentY += ChildHeight;
    }
}
