#pragma once
#include "SCompoundWidget.h"
#include "Delegates.h"

/**
 * SColorPicker - 색상 선택 위젯
 *
 * 특징:
 * - RGB/HSV 색상 선택
 * - 알파 채널 지원
 * - 컴팩트/전체 모드
 * - 델리게이트 이벤트
 *
 * 사용 예시:
 * auto ColorPicker = new SColorPicker();
 * ColorPicker->SetColor(0xFF0000FF); // Red
 * ColorPicker->SetAlphaEnabled(true);
 * ColorPicker->OnColorChanged.Add([](uint32_t NewColor) {
 *     // 색상 변경 처리
 * });
 */
class SColorPicker : public SCompoundWidget
{
public:
    enum EPickerMode
    {
        Compact,    // 작은 색상 버튼
        Full        // 전체 색상 피커
    };

    SColorPicker();
    SColorPicker(uint32_t InColor);
    virtual ~SColorPicker();

    // ===== 색상 설정 =====
    void SetColor(uint32_t InColor);
    uint32_t GetColor() const { return Color; }

    // RGB 개별 설정
    void SetColorRGB(uint8_t R, uint8_t G, uint8_t B, uint8_t A = 255);
    void GetColorRGB(uint8_t& OutR, uint8_t& OutG, uint8_t& OutB, uint8_t& OutA) const;

    // ===== 옵션 =====
    void SetAlphaEnabled(bool bEnabled) { bAlphaEnabled = bEnabled; Invalidate(); }
    bool IsAlphaEnabled() const { return bAlphaEnabled; }

    void SetPickerMode(EPickerMode Mode) { PickerMode = Mode; Invalidate(); }
    EPickerMode GetPickerMode() const { return PickerMode; }

    void SetLabel(const FString& InLabel) { Label = InLabel; Invalidate(); }
    FString GetLabel() const { return Label; }

    // ===== 델리게이트 =====
    TDelegate<uint32_t> OnColorChanged;    // 색상 변경 중
    TDelegate<uint32_t> OnColorCommitted;  // 색상 확정

    // ===== 크기 =====
    void SetSize(float Width, float Height) { PickerWidth = Width; PickerHeight = Height; Invalidate(); }
    virtual float GetWidth() const override
    {
        float Width = Rect.GetWidth();
        return Width > 0.0f ? Width : PickerWidth;
    }
    virtual float GetHeight() const override
    {
        float Height = Rect.GetHeight();
        return Height > 0.0f ? Height : PickerHeight;
    }

    // ===== 렌더링 =====
    virtual void RenderContent() override;

private:
    uint32_t Color = 0xFFFFFFFF;  // RGBA
    bool bAlphaEnabled = true;
    EPickerMode PickerMode = Compact;
    FString Label;
    float PickerWidth = 250.0f;
    float PickerHeight = 250.0f;

    // ImVec4로 변환 (ImGui 호환)
    ImVec4 ColorToImVec4() const;
    void ImVec4ToColor(const ImVec4& Vec);

    FString GetImGuiID() const;
};
