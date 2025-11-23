#pragma once
#include "SCompoundWidget.h"

/**
 * SImage - 이미지 표시 위젯
 *
 * 특징:
 * - 텍스처를 화면에 표시
 * - 크기 조절 지원 (원본, 늘이기, 맞추기)
 * - Tint 색상 지원
 *
 * 사용 예시:
 * auto Image = new SImage();
 * Image->SetTexture(MyTexture);
 * Image->SetSize(FVector2D(100, 100));
 * Image->SetTint(0xFFFFFFFF);
 */
class SImage : public SCompoundWidget
{
public:
    enum class EImageScaling
    {
        None,           // 원본 크기
        Stretch,        // 늘이기
        Fill,           // 영역 채우기 (비율 유지)
        Fit             // 영역에 맞추기 (비율 유지)
    };

    SImage();
    SImage(void* InTexture);
    virtual ~SImage();

    // ===== 텍스처 설정 =====
    void SetTexture(void* InTexture) { Texture = InTexture; Invalidate(); }
    void* GetTexture() const { return Texture; }

    // ===== 크기 설정 =====
    void SetSize(const FVector2D& InSize) { Size = InSize; Invalidate(); }
    FVector2D GetSize() const { return Size; }

    // ===== 스케일링 모드 =====
    void SetScaling(EImageScaling InScaling) { Scaling = InScaling; Invalidate(); }
    EImageScaling GetScaling() const { return Scaling; }

    // ===== Tint 색상 =====
    void SetTint(uint32_t InTint) { TintColor = InTint; Invalidate(); }
    uint32_t GetTint() const { return TintColor; }

    // ===== SWindow 오버라이드 =====
    virtual float GetWidth() const override { return Size.X; }
    virtual float GetHeight() const override { return Size.Y; }

    // ===== 렌더링 =====
    virtual void RenderContent() override;

private:
    void* Texture = nullptr;        // ImTextureID (void*)
    FVector2D Size = FVector2D(100, 100);
    EImageScaling Scaling = EImageScaling::Stretch;
    uint32_t TintColor = 0xFFFFFFFF;  // RGBA

    ImVec4 GetTintColorVec4() const;
};
