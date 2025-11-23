#include "pch.h"
#include "SImage.h"
#include "ImGui/imgui.h"

SImage::SImage()
    : Texture(nullptr)
    , Size(100, 100)
    , Scaling(EImageScaling::Stretch)
    , TintColor(0xFFFFFFFF)
{
}

SImage::SImage(void* InTexture)
    : Texture(InTexture)
    , Size(100, 100)
    , Scaling(EImageScaling::Stretch)
    , TintColor(0xFFFFFFFF)
{
}

SImage::~SImage()
{
}

ImVec4 SImage::GetTintColorVec4() const
{
    // RGBA -> ImVec4 (0.0 ~ 1.0)
    float r = ((TintColor >> 24) & 0xFF) / 255.0f;
    float g = ((TintColor >> 16) & 0xFF) / 255.0f;
    float b = ((TintColor >> 8) & 0xFF) / 255.0f;
    float a = (TintColor & 0xFF) / 255.0f;
    return ImVec4(r, g, b, a);
}

void SImage::RenderContent()
{
    if (!bIsVisible || !Texture)
        return;

    // ImGui::ImageWithBg 사용 (ImGui 1.91.9+)
    ImVec2 ImageSize(Size.X, Size.Y);
    ImVec4 TintVec = GetTintColorVec4();

    // UV 좌표 (전체 이미지 사용)
    ImVec2 UV0(0, 0);
    ImVec2 UV1(1, 1);

    // ImageWithBg: (texture, size, uv0, uv1, bg_col, tint_col)
    ImVec4 BgCol(0, 0, 0, 0); // 투명 배경

    ImGui::ImageWithBg(
        Texture,
        ImageSize,
        UV0,
        UV1,
        BgCol,
        TintVec
    );
}
