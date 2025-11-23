#pragma once
#include "Source/Slate/Core/Windows/SWindow.h"

/**
 * SCompoundWidget - 리프 위젯의 베이스 클래스
 *
 * 특징:
 * - 자식 위젯을 가지지 않음 (리프 노드)
 * - SPanel과 달리 자식 관리 기능 없음
 * - 단일 기능 위젯 (버튼, 텍스트, 체크박스 등)
 *
 * 사용 예시:
 * - SButton
 * - STextBlock
 * - SEditableText
 * - SCheckBox
 * - SImage
 */
class SCompoundWidget : public SWindow
{
public:
    SCompoundWidget() = default;
    virtual ~SCompoundWidget() = default;

    // ===== 렌더링 =====
    virtual void OnRender() override
    {
        if (!bIsVisible)
            return;

        // 자식 위젯이 없으므로 자신만 렌더링
        RenderContent();
    }

    // ===== 파생 클래스에서 구현할 렌더링 =====
    virtual void RenderContent() {}

    // ===== 가시성 =====
    void SetVisible(bool bInVisible) { bIsVisible = bInVisible; }
    bool IsVisible() const { return bIsVisible; }

    // ===== 활성화 =====
    void SetEnabled(bool bInEnabled) { bIsEnabled = bInEnabled; }
    bool IsEnabled() const { return bIsEnabled; }

    // ===== Invalidation =====
    void Invalidate() { bNeedsRepaint = true; }
    bool NeedsRepaint() const { return bNeedsRepaint; }

protected:
    bool bIsVisible = true;
    bool bIsEnabled = true;
    bool bNeedsRepaint = true;
};
