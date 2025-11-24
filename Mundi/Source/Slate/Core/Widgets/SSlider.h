#pragma once
#include "SCompoundWidget.h"
#include "Delegates.h"

/**
 * SSlider - 슬라이더 위젯
 *
 * 특징:
 * - 최소값 ~ 최대값 범위에서 값 선택
 * - 수평/수직 방향 지원
 * - 델리게이트를 통한 값 변경 이벤트
 * - 포맷 문자열 지원 (표시 형식 커스터마이징)
 *
 * 사용 예시:
 * auto Slider = new SSlider(0.0f, 100.0f);
 * Slider->SetValue(50.0f);
 * Slider->SetOrientation(SSlider::Horizontal);
 * Slider->OnValueChanged.Add([](float NewValue) {
 *     // 값 변경 처리
 * });
 */
class SSlider : public SCompoundWidget
{
public:
    enum EOrientation
    {
        Horizontal,
        Vertical
    };

    SSlider();
    SSlider(float InMinValue, float InMaxValue);
    virtual ~SSlider();

    // ===== 값 설정 =====
    void SetValue(float InValue);
    float GetValue() const { return Value; }

    // ===== 범위 설정 =====
    void SetMinValue(float InMin) { MinValue = InMin; Invalidate(); }
    void SetMaxValue(float InMax) { MaxValue = InMax; Invalidate(); }
    void SetRange(float InMin, float InMax) { MinValue = InMin; MaxValue = InMax; Invalidate(); }
    float GetMinValue() const { return MinValue; }
    float GetMaxValue() const { return MaxValue; }

    // ===== 방향 설정 =====
    void SetOrientation(EOrientation InOrientation) { Orientation = InOrientation; Invalidate(); }
    EOrientation GetOrientation() const { return Orientation; }

    // ===== 라벨 및 포맷 =====
    void SetLabel(const FString& InLabel) { Label = InLabel; Invalidate(); }
    FString GetLabel() const { return Label; }

    void SetFormat(const FString& InFormat) { Format = InFormat; Invalidate(); }
    FString GetFormat() const { return Format; }

    // ===== 크기 설정 =====
    void SetWidth(float InWidth) { Width = InWidth; Invalidate(); }
    float GetWidth() const { return Width; }

    // ===== 델리게이트 (이벤트) =====
    TDelegate<float> OnValueChanged;       // 값이 변경될 때 (드래그 중)
    TDelegate<float> OnValueCommitted;     // 값 변경 완료 시 (마우스 릴리즈)

    // ===== 렌더링 =====
    virtual void RenderContent() override;

private:
    float Value = 0.0f;
    float MinValue = 0.0f;
    float MaxValue = 100.0f;
    float Width = 200.0f;

    EOrientation Orientation = Horizontal;
    FString Label;
    FString Format = "%.1f";  // 기본 포맷: 소수점 1자리

    // ImGui ID (고유 식별자)
    FString GetImGuiID() const;
};
