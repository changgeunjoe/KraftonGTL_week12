#pragma once
#include "SCompoundWidget.h"

/**
 * SSeparator - 구분선 위젯
 *
 * 특징:
 * - 수평 또는 수직 구분선
 * - 색상 및 두께 조절 가능
 * - UI 요소들을 시각적으로 분리
 *
 * 사용 예시:
 * auto Separator = new SSeparator();
 * Separator->SetOrientation(SSeparator::Horizontal);
 * Separator->SetThickness(2.0f);
 * Separator->SetColor(0xFF808080);
 */
class SSeparator : public SCompoundWidget
{
public:
    enum EOrientation
    {
        Horizontal,
        Vertical
    };

    SSeparator();
    SSeparator(EOrientation InOrientation);
    virtual ~SSeparator();

    // ===== 방향 설정 =====
    void SetOrientation(EOrientation InOrientation) { Orientation = InOrientation; Invalidate(); }
    EOrientation GetOrientation() const { return Orientation; }

    // ===== 두께 설정 =====
    void SetThickness(float InThickness) { Thickness = InThickness; Invalidate(); }
    float GetThickness() const { return Thickness; }

    // ===== 색상 설정 =====
    void SetColor(uint32_t InColor) { Color = InColor; Invalidate(); }
    uint32_t GetColor() const { return Color; }

    // ===== SWindow 오버라이드 =====
    virtual float GetHeight() const override;

    // ===== 렌더링 =====
    virtual void RenderContent() override;

private:
    EOrientation Orientation = Horizontal;
    float Thickness = 1.0f;
    uint32_t Color = 0xFF808080;  // 기본 회색
};
