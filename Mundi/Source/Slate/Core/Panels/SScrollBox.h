#pragma once
#include "SPanel.h"

/**
 * SScrollBox - 스크롤 가능한 컨테이너 패널
 *
 * 특징:
 * - 자식 위젯들이 영역을 벗어나면 스크롤바 표시
 * - 수직/수평 스크롤 지원
 * - 자동 크기 조절
 *
 * 사용 예시:
 * auto ScrollBox = new SScrollBox();
 * ScrollBox->SetScrollbarVisibility(true, false); // 수직만
 * ScrollBox->AddChild(new STextBlock("Item 1"));
 * ScrollBox->AddChild(new STextBlock("Item 2"));
 * // ... 많은 아이템들
 */
class SScrollBox : public SPanel
{
public:
    SScrollBox();
    virtual ~SScrollBox();

    // ===== 스크롤바 설정 =====
    void SetScrollbarVisibility(bool bVertical, bool bHorizontal);
    void SetAlwaysShowScrollbar(bool bAlways) { bAlwaysShowScrollbar = bAlways; }

    // ===== 스크롤 위치 =====
    void ScrollToTop();
    void ScrollToBottom();
    void SetScrollY(float Y);
    float GetScrollY() const;

    // ===== 렌더링 =====
    virtual void RenderContent() override;
    virtual void ArrangeChildren() override;

private:
    bool bEnableVerticalScroll = true;
    bool bEnableHorizontalScroll = false;
    bool bAlwaysShowScrollbar = false;

    // ImGui ID (고유 식별자)
    FString GetImGuiID() const;
};
