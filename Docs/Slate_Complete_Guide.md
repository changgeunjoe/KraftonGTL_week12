# 🎨 Mundi Engine - Slate UI 시스템 완전 활용 가이드

> **작성일**: 2025-01-21
> **대상**: Mundi 엔진 개발자
> **목적**: 언리얼 Slate 스타일 UI 시스템의 완전한 활용 방법

---

## 📚 목차

1. [Slate 기초 개념](#1-slate-기초-개념)
2. [기본 위젯 라이브러리](#2-기본-위젯-라이브러리)
3. [레이아웃 시스템](#3-레이아웃-시스템)
4. [고급 위젯](#4-고급-위젯)
5. [실전 에디터 제작](#5-실전-에디터-제작)
6. [최적화 및 고급 기법](#6-최적화-및-고급-기법)
7. [단계별 구현 로드맵](#7-단계별-구현-로드맵)

---

## 1. Slate 기초 개념

### 1.1 Slate란?

**Slate**는 언리얼 엔진의 UI 프레임워크로, 다음 특징을 가집니다:

- ✅ **컴포넌트 기반 아키텍처**: 재사용 가능한 UI 블록
- ✅ **계층 구조**: 부모-자식 관계로 UI 구성
- ✅ **선언적 문법**: 코드로 UI 구조 정의 (언리얼)
- ✅ **즉시 모드**: 매 프레임 렌더링 (Mundi - ImGui 기반)
- ✅ **이벤트 기반**: 델리게이트로 상호작용

### 1.2 Mundi의 Slate 구조

```
SWidget (베이스)
├── SPanel (컨테이너)
│   ├── Layout Panels (레이아웃)
│   │   ├── SVerticalBox
│   │   ├── SHorizontalBox
│   │   ├── SGridPanel
│   │   ├── SScrollBox
│   │   └── SSplitter ✅ (이미 구현됨)
│   └── Custom Panels (커스텀)
│       ├── SParticleEmitterPanel
│       ├── SGraphPanel (Blueprint용)
│       └── STimelinePanel
└── SCompoundWidget (복합 위젯)
    ├── SButton
    ├── STextBlock
    ├── SEditableText
    ├── SCheckBox
    ├── SComboBox
    ├── STreeView
    └── SListView
```

---

## 2. 기본 위젯 라이브러리

### 2.1 SPanel - 베이스 클래스

모든 Slate 위젯의 기반이 되는 클래스입니다.

#### 구현 예시

```cpp
// SPanel.h
#pragma once
#include "SWindow.h"

class SPanel : public SWindow
{
public:
    SPanel();
    virtual ~SPanel();

    // ===== 자식 관리 =====
    void AddChild(SPanel* Child);
    void RemoveChild(SPanel* Child);
    void ClearChildren();
    const TArray<SPanel*>& GetChildren() const { return Children; }

    // ===== 가시성 =====
    void SetVisible(bool bInVisible) { bIsVisible = bInVisible; }
    bool IsVisible() const { return bIsVisible; }

    // ===== 활성화 =====
    void SetEnabled(bool bInEnabled) { bIsEnabled = bInEnabled; }
    bool IsEnabled() const { return bIsEnabled; }

    // ===== 렌더링 =====
    virtual void OnRender() override;
    virtual void RenderContent() {}  // 파생 클래스에서 구현

protected:
    TArray<SPanel*> Children;
    bool bIsVisible = true;
    bool bIsEnabled = true;
};
```

#### 활용 방법

1. **커스텀 패널 만들기**
```cpp
class SMyCustomPanel : public SPanel
{
protected:
    void RenderContent() override
    {
        ImGui::Text("My Custom Panel");
        // 커스텀 렌더링 로직
    }
};
```

2. **계층 구조 만들기**
```cpp
auto MainPanel = new SPanel();
auto ChildPanel1 = new SPanel();
auto ChildPanel2 = new SPanel();

MainPanel->AddChild(ChildPanel1);
MainPanel->AddChild(ChildPanel2);
```

---

### 2.2 SButton - 버튼 위젯

클릭 가능한 버튼 위젯입니다.

#### 구현

```cpp
// SButton.h
#pragma once
#include "Panels/SPanel.h"

class SButton : public SPanel
{
public:
    SButton();

    // ===== 설정 =====
    void SetText(const FString& InText) { Text = InText; }
    void SetIcon(class UTexture* InIcon) { Icon = InIcon; }
    void SetSize(FVector2D InSize) { Size = InSize; }

    // ===== 이벤트 =====
    TDelegate<void()> OnClicked;
    TDelegate<void()> OnHovered;
    TDelegate<void()> OnUnhovered;

    // ===== 스타일 =====
    void SetButtonStyle(const FButtonStyle& InStyle) { Style = InStyle; }

protected:
    void RenderContent() override;
    void OnMouseDown(FVector2D MousePos, uint32 Button) override;
    void OnMouseUp(FVector2D MousePos, uint32 Button) override;
    void OnMouseMove(FVector2D MousePos) override;

private:
    FString Text;
    UTexture* Icon = nullptr;
    FVector2D Size = FVector2D(100, 30);
    bool bIsHovered = false;
    bool bIsPressed = false;

    struct FButtonStyle
    {
        ImU32 NormalColor = IM_COL32(60, 60, 60, 255);
        ImU32 HoveredColor = IM_COL32(80, 80, 80, 255);
        ImU32 PressedColor = IM_COL32(40, 40, 40, 255);
        ImU32 TextColor = IM_COL32(255, 255, 255, 255);
    } Style;
};
```

```cpp
// SButton.cpp
void SButton::RenderContent()
{
    ImVec2 ButtonMin(Rect.Left, Rect.Top);
    ImVec2 ButtonMax(Rect.Left + Size.X, Rect.Top + Size.Y);

    // 상태에 따른 색상
    ImU32 BgColor = bIsPressed ? Style.PressedColor :
                    bIsHovered ? Style.HoveredColor :
                    Style.NormalColor;

    ImDrawList* DrawList = ImGui::GetWindowDrawList();

    // 배경
    DrawList->AddRectFilled(ButtonMin, ButtonMax, BgColor, 4.0f);

    // 테두리
    DrawList->AddRect(ButtonMin, ButtonMax, IM_COL32(100, 100, 100, 255), 4.0f);

    // 텍스트
    if (!Text.empty())
    {
        ImVec2 TextSize = ImGui::CalcTextSize(Text.c_str());
        ImVec2 TextPos(
            ButtonMin.x + (Size.X - TextSize.x) * 0.5f,
            ButtonMin.y + (Size.Y - TextSize.y) * 0.5f
        );
        DrawList->AddText(TextPos, Style.TextColor, Text.c_str());
    }

    // 아이콘
    if (Icon)
    {
        // 아이콘 렌더링 로직
    }
}

void SButton::OnMouseDown(FVector2D MousePos, uint32 Button)
{
    if (IsHover(MousePos) && bIsEnabled)
    {
        bIsPressed = true;
    }
}

void SButton::OnMouseUp(FVector2D MousePos, uint32 Button)
{
    if (bIsPressed && IsHover(MousePos))
    {
        if (OnClicked.IsBound())
        {
            OnClicked.Execute();
        }
    }
    bIsPressed = false;
}

void SButton::OnMouseMove(FVector2D MousePos)
{
    bool bWasHovered = bIsHovered;
    bIsHovered = IsHover(MousePos);

    if (bIsHovered && !bWasHovered)
    {
        if (OnHovered.IsBound())
            OnHovered.Execute();
    }
    else if (!bIsHovered && bWasHovered)
    {
        if (OnUnhovered.IsBound())
            OnUnhovered.Execute();
    }
}
```

#### 활용 예시

```cpp
// 1. 기본 버튼
auto SaveButton = new SButton();
SaveButton->SetText("Save");
SaveButton->OnClicked.Bind([this]() {
    SaveParticleSystem();
});

// 2. 아이콘 버튼
auto PlayButton = new SButton();
PlayButton->SetIcon(IconPlay);
PlayButton->SetSize(FVector2D(40, 40));
PlayButton->OnClicked.Bind([this]() {
    PlayParticlePreview();
});

// 3. 토글 버튼
auto ToggleButton = new SButton();
ToggleButton->SetText(bIsPlaying ? "Pause" : "Play");
ToggleButton->OnClicked.Bind([this, ToggleButton]() {
    bIsPlaying = !bIsPlaying;
    ToggleButton->SetText(bIsPlaying ? "Pause" : "Play");
});
```

---

### 2.3 STextBlock - 텍스트 표시 위젯

#### 구현

```cpp
// STextBlock.h
class STextBlock : public SPanel
{
public:
    // ===== 설정 =====
    void SetText(const FString& InText) { Text = InText; }
    void SetText(TDelegate<FString()> InTextDelegate) { TextDelegate = InTextDelegate; }

    void SetFont(const FSlateFontInfo& InFont) { Font = InFont; }
    void SetColor(ImU32 InColor) { Color = InColor; }
    void SetJustification(ETextJustify InJustify) { Justification = InJustify; }

    enum ETextJustify
    {
        Left,
        Center,
        Right
    };

protected:
    void RenderContent() override;

private:
    FString GetDisplayText() const
    {
        return TextDelegate.IsBound() ? TextDelegate.Execute() : Text;
    }

    FString Text;
    TDelegate<FString()> TextDelegate;
    FSlateFontInfo Font;
    ImU32 Color = IM_COL32(255, 255, 255, 255);
    ETextJustify Justification = Left;
};
```

#### 활용 예시

```cpp
// 1. 정적 텍스트
auto TitleText = new STextBlock();
TitleText->SetText("Cascade Particle Editor");
TitleText->SetColor(IM_COL32(255, 200, 100, 255));

// 2. 동적 텍스트 (델리게이트 바인딩)
auto ParticleCountText = new STextBlock();
ParticleCountText->SetText([this]() -> FString {
    return FString::Printf("Active Particles: %d", GetTotalParticles());
});

// 3. 정렬 텍스트
auto CenteredText = new STextBlock();
CenteredText->SetText("Center Aligned");
CenteredText->SetJustification(STextBlock::Center);
```

---

### 2.4 SEditableText - 텍스트 입력 위젯

#### 구현

```cpp
// SEditableText.h
class SEditableText : public SPanel
{
public:
    // ===== 이벤트 =====
    TDelegate<void(const FString&)> OnTextChanged;
    TDelegate<void(const FString&)> OnTextCommitted;

    // ===== 설정 =====
    void SetText(const FString& InText) { Text = InText; }
    FString GetText() const { return Text; }
    void SetHintText(const FString& InHint) { HintText = InHint; }
    void SetIsPassword(bool bInPassword) { bIsPassword = bInPassword; }

protected:
    void RenderContent() override;

private:
    FString Text;
    FString HintText;
    bool bIsPassword = false;
    bool bIsFocused = false;
};
```

```cpp
// SEditableText.cpp
void SEditableText::RenderContent()
{
    char Buffer[256];
    strcpy_s(Buffer, Text.c_str());

    ImGui::SetCursorScreenPos(ImVec2(Rect.Left, Rect.Top));

    int Flags = ImGuiInputTextFlags_None;
    if (bIsPassword)
        Flags |= ImGuiInputTextFlags_Password;

    if (ImGui::InputText("##EditableText", Buffer, 256, Flags))
    {
        Text = Buffer;
        if (OnTextChanged.IsBound())
        {
            OnTextChanged.Execute(Text);
        }
    }

    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        if (OnTextCommitted.IsBound())
        {
            OnTextCommitted.Execute(Text);
        }
    }
}
```

#### 활용 예시

```cpp
// 1. 이름 입력
auto NameInput = new SEditableText();
NameInput->SetHintText("Enter emitter name...");
NameInput->OnTextCommitted.Bind([this](const FString& NewName) {
    RenameEmitter(NewName);
});

// 2. 숫자 입력
auto SpawnRateInput = new SEditableText();
SpawnRateInput->SetText("100");
SpawnRateInput->OnTextChanged.Bind([this](const FString& Value) {
    float Rate = FCString::Atof(Value.c_str());
    UpdateSpawnRate(Rate);
});

// 3. 비밀번호 입력
auto PasswordInput = new SEditableText();
PasswordInput->SetIsPassword(true);
PasswordInput->SetHintText("Password");
```

---

### 2.5 SCheckBox - 체크박스 위젯

#### 구현

```cpp
// SCheckBox.h
class SCheckBox : public SPanel
{
public:
    // ===== 이벤트 =====
    TDelegate<void(bool)> OnCheckStateChanged;

    // ===== 설정 =====
    void SetIsChecked(bool bInChecked) { bIsChecked = bInChecked; }
    bool IsChecked() const { return bIsChecked; }
    void SetLabel(const FString& InLabel) { Label = InLabel; }

protected:
    void RenderContent() override;
    void OnMouseDown(FVector2D MousePos, uint32 Button) override;

private:
    bool bIsChecked = false;
    FString Label;
};
```

#### 활용 예시

```cpp
// 1. 기본 체크박스
auto LoopCheckbox = new SCheckBox();
LoopCheckbox->SetLabel("Loop Animation");
LoopCheckbox->OnCheckStateChanged.Bind([this](bool bChecked) {
    SetAnimationLoop(bChecked);
});

// 2. 옵션 토글
auto ShowGridCheckbox = new SCheckBox();
ShowGridCheckbox->SetLabel("Show Grid");
ShowGridCheckbox->SetIsChecked(true);
ShowGridCheckbox->OnCheckStateChanged.Bind([](bool bChecked) {
    GShowFlags.Grid = bChecked;
});
```

---

## 3. 레이아웃 시스템

### 3.1 SVerticalBox - 수직 레이아웃

세로 방향으로 위젯을 배치합니다.

#### 구현

```cpp
// SVerticalBox.h
#pragma once
#include "Panels/SPanel.h"

class SVerticalBox : public SPanel
{
public:
    enum EVerticalAlignment
    {
        VAlign_Top,
        VAlign_Center,
        VAlign_Bottom,
        VAlign_Fill
    };

    enum ESizeRule
    {
        SizeRule_Auto,      // 자식 크기에 맞춤
        SizeRule_Fill,      // 남은 공간 채움 (비율)
        SizeRule_Fixed      // 고정 픽셀 크기
    };

    struct FSlot
    {
        SPanel* Widget = nullptr;
        ESizeRule SizeRule = SizeRule_Auto;
        float SizeValue = 1.0f;
        EVerticalAlignment VAlign = VAlign_Top;
        FMargin Padding = FMargin(0, 0, 0, 0);

        // Fluent API
        FSlot& AutoHeight()
        {
            SizeRule = SizeRule_Auto;
            return *this;
        }

        FSlot& FillHeight(float Ratio = 1.0f)
        {
            SizeRule = SizeRule_Fill;
            SizeValue = Ratio;
            return *this;
        }

        FSlot& FixedHeight(float Height)
        {
            SizeRule = SizeRule_Fixed;
            SizeValue = Height;
            return *this;
        }

        FSlot& Padding(float Uniform)
        {
            Padding = FMargin(Uniform, Uniform, Uniform, Uniform);
            return *this;
        }

        FSlot& Padding(float Horizontal, float Vertical)
        {
            Padding = FMargin(Horizontal, Vertical, Horizontal, Vertical);
            return *this;
        }

        FSlot& Padding(float Left, float Top, float Right, float Bottom)
        {
            Padding = FMargin(Left, Top, Right, Bottom);
            return *this;
        }

        FSlot& VAlignment(EVerticalAlignment InAlign)
        {
            VAlign = InAlign;
            return *this;
        }

        void AttachWidget(SPanel* InWidget)
        {
            Widget = InWidget;
        }
    };

    SVerticalBox();
    virtual ~SVerticalBox();

    // ===== Slot 관리 =====
    FSlot& AddSlot();
    void RemoveSlot(int32 Index);
    void ClearSlots();

protected:
    void RenderContent() override;
    void OnPanelResized() override;

private:
    void ArrangeChildren();
    void CalculateSlotSizes();

    TArray<FSlot> Slots;
    TArray<float> ComputedHeights;
};
```

```cpp
// SVerticalBox.cpp
void SVerticalBox::CalculateSlotSizes()
{
    ComputedHeights.clear();

    float TotalHeight = Rect.GetHeight();
    float UsedHeight = 0.0f;
    float TotalFillWeight = 0.0f;

    // 1단계: Auto와 Fixed 크기 계산
    for (FSlot& Slot : Slots)
    {
        if (Slot.SizeRule == SizeRule_Auto)
        {
            if (Slot.Widget)
            {
                float Height = Slot.Widget->GetHeight() +
                              Slot.Padding.Top + Slot.Padding.Bottom;
                ComputedHeights.Add(Height);
                UsedHeight += Height;
            }
            else
            {
                ComputedHeights.Add(0.0f);
            }
        }
        else if (Slot.SizeRule == SizeRule_Fixed)
        {
            float Height = Slot.SizeValue + Slot.Padding.Top + Slot.Padding.Bottom;
            ComputedHeights.Add(Height);
            UsedHeight += Height;
        }
        else if (Slot.SizeRule == SizeRule_Fill)
        {
            TotalFillWeight += Slot.SizeValue;
            ComputedHeights.Add(0.0f); // 나중에 계산
        }
    }

    // 2단계: Fill 크기 계산
    float RemainingHeight = FMath::Max(0.0f, TotalHeight - UsedHeight);

    int32 SlotIndex = 0;
    for (FSlot& Slot : Slots)
    {
        if (Slot.SizeRule == SizeRule_Fill)
        {
            if (TotalFillWeight > 0.0f)
            {
                float Ratio = Slot.SizeValue / TotalFillWeight;
                ComputedHeights[SlotIndex] = RemainingHeight * Ratio;
            }
            else
            {
                ComputedHeights[SlotIndex] = 0.0f;
            }
        }
        SlotIndex++;
    }
}

void SVerticalBox::ArrangeChildren()
{
    if (Slots.Num() == 0) return;

    CalculateSlotSizes();

    float CurrentY = Rect.Top;

    for (int32 i = 0; i < Slots.Num(); i++)
    {
        FSlot& Slot = Slots[i];
        if (!Slot.Widget) continue;

        float SlotHeight = ComputedHeights[i];
        float ContentHeight = SlotHeight - Slot.Padding.Top - Slot.Padding.Bottom;

        // VAlignment 적용
        float ChildY = CurrentY + Slot.Padding.Top;

        if (Slot.VAlign == VAlign_Center && Slot.SizeRule != SizeRule_Fill)
        {
            float ChildHeight = Slot.Widget->GetHeight();
            ChildY += (ContentHeight - ChildHeight) * 0.5f;
        }
        else if (Slot.VAlign == VAlign_Bottom && Slot.SizeRule != SizeRule_Fill)
        {
            float ChildHeight = Slot.Widget->GetHeight();
            ChildY += ContentHeight - ChildHeight;
        }

        // 위젯 배치
        float ChildX = Rect.Left + Slot.Padding.Left;
        float ChildWidth = Rect.GetWidth() - Slot.Padding.Left - Slot.Padding.Right;

        Slot.Widget->SetRect(
            ChildX, ChildY,
            ChildX + ChildWidth,
            ChildY + (Slot.VAlign == VAlign_Fill ? ContentHeight : Slot.Widget->GetHeight())
        );

        CurrentY += SlotHeight;
    }
}

void SVerticalBox::RenderContent()
{
    for (FSlot& Slot : Slots)
    {
        if (Slot.Widget && Slot.Widget->IsVisible())
        {
            Slot.Widget->OnRender();
        }
    }
}
```

#### 활용 예시

```cpp
// 1. 기본 수직 레이아웃
auto VBox = new SVerticalBox();

VBox->AddSlot()
    .AutoHeight()
    .SetPadding(5.0f)
    .AttachWidget(new STextBlock("Title"));

VBox->AddSlot()
    .FillHeight(1.0f)
    .SetPadding(5.0f)
    .AttachWidget(new SEditableText());

VBox->AddSlot()
    .AutoHeight()
    .SetPadding(5.0f)
    .AttachWidget(new SButton());

// 2. 툴바 + 콘텐츠 + 상태바 레이아웃
auto MainLayout = new SVerticalBox();

// 툴바 (고정 높이)
MainLayout->AddSlot()
    .FixedHeight(40.0f)
    .AttachWidget(CreateToolbar());

// 메인 콘텐츠 (남은 공간 채움)
MainLayout->AddSlot()
    .FillHeight(1.0f)
    .AttachWidget(CreateMainContent());

// 상태바 (자동 높이)
MainLayout->AddSlot()
    .AutoHeight()
    .AttachWidget(CreateStatusBar());

// 3. 복잡한 레이아웃
auto ComplexLayout = new SVerticalBox();

// 헤더
ComplexLayout->AddSlot()
    .AutoHeight()
    .SetPadding(0, 0, 0, 10)
    .AttachWidget(CreateHeader());

// 중간 영역 (70% 높이)
ComplexLayout->AddSlot()
    .FillHeight(0.7f)
    .SetPadding(5.0f)
    .VAlignment(SVerticalBox::VAlign_Fill)
    .AttachWidget(CreateMiddleSection());

// 하단 영역 (30% 높이)
ComplexLayout->AddSlot()
    .FillHeight(0.3f)
    .SetPadding(5.0f)
    .AttachWidget(CreateBottomSection());
```

---

### 3.2 SHorizontalBox - 수평 레이아웃

가로 방향으로 위젯을 배치합니다.

#### 구현

```cpp
// SHorizontalBox.h
class SHorizontalBox : public SPanel
{
public:
    enum EHorizontalAlignment
    {
        HAlign_Left,
        HAlign_Center,
        HAlign_Right,
        HAlign_Fill
    };

    enum ESizeRule
    {
        SizeRule_Auto,
        SizeRule_Fill,
        SizeRule_Fixed
    };

    struct FSlot
    {
        SPanel* Widget = nullptr;
        ESizeRule SizeRule = SizeRule_Auto;
        float SizeValue = 1.0f;
        EHorizontalAlignment HAlign = HAlign_Left;
        FMargin Padding = FMargin(0, 0, 0, 0);

        FSlot& AutoWidth()
        {
            SizeRule = SizeRule_Auto;
            return *this;
        }

        FSlot& FillWidth(float Ratio = 1.0f)
        {
            SizeRule = SizeRule_Fill;
            SizeValue = Ratio;
            return *this;
        }

        FSlot& FixedWidth(float Width)
        {
            SizeRule = SizeRule_Fixed;
            SizeValue = Width;
            return *this;
        }

        FSlot& Padding(float Uniform) { /* ... */ return *this; }
        FSlot& HAlignment(EHorizontalAlignment InAlign) { /* ... */ return *this; }
        void AttachWidget(SPanel* InWidget) { Widget = InWidget; }
    };

    FSlot& AddSlot();
    // ... 나머지 SVerticalBox와 유사
};
```

#### 활용 예시

```cpp
// 1. 툴바 버튼들
auto Toolbar = new SHorizontalBox();

Toolbar->AddSlot()
    .AutoWidth()
    .SetPadding(2.0f)
    .AttachWidget(CreateButton("New"));

Toolbar->AddSlot()
    .AutoWidth()
    .SetPadding(2.0f)
    .AttachWidget(CreateButton("Open"));

Toolbar->AddSlot()
    .AutoWidth()
    .SetPadding(2.0f)
    .AttachWidget(CreateButton("Save"));

Toolbar->AddSlot()
    .FillWidth(1.0f)
    .AttachWidget(new SPanel()); // Spacer

Toolbar->AddSlot()
    .AutoWidth()
    .SetPadding(2.0f)
    .AttachWidget(CreateButton("Help"));

// 2. 레이블 + 입력 필드
auto LabeledInput = new SHorizontalBox();

LabeledInput->AddSlot()
    .AutoWidth()
    .SetPadding(0, 0, 10, 0)
    .VAlignment(SHorizontalBox::VAlign_Center)
    .AttachWidget(new STextBlock("Name:"));

LabeledInput->AddSlot()
    .FillWidth(1.0f)
    .AttachWidget(new SEditableText());

// 3. 좌우 분할
auto SplitLayout = new SHorizontalBox();

SplitLayout->AddSlot()
    .FillWidth(0.3f)
    .AttachWidget(CreateLeftPanel());

SplitLayout->AddSlot()
    .FillWidth(0.7f)
    .AttachWidget(CreateRightPanel());
```

---

### 3.3 SGridPanel - 그리드 레이아웃

격자 형태로 위젯을 배치합니다.

#### 구현

```cpp
// SGridPanel.h
class SGridPanel : public SPanel
{
public:
    struct FSlot
    {
        SPanel* Widget = nullptr;
        int32 Column = 0;
        int32 Row = 0;
        int32 ColumnSpan = 1;
        int32 RowSpan = 1;
        FMargin Padding = FMargin(0, 0, 0, 0);

        FSlot& SetColumn(int32 InColumn) { Column = InColumn; return *this; }
        FSlot& SetRow(int32 InRow) { Row = InRow; return *this; }
        FSlot& SetColumnSpan(int32 InSpan) { ColumnSpan = InSpan; return *this; }
        FSlot& SetRowSpan(int32 InSpan) { RowSpan = InSpan; return *this; }
        FSlot& Padding(float Uniform) { /* ... */ return *this; }
        void AttachWidget(SPanel* InWidget) { Widget = InWidget; }
    };

    SGridPanel();

    // ===== 열/행 정의 =====
    void SetColumnFill(int32 Column, float Weight);
    void SetRowFill(int32 Row, float Weight);

    FSlot& AddSlot(int32 Column, int32 Row);

protected:
    void RenderContent() override;
    void ArrangeChildren();

private:
    TArray<FSlot> Slots;
    TMap<int32, float> ColumnWeights;
    TMap<int32, float> RowWeights;
    int32 NumColumns = 0;
    int32 NumRows = 0;
};
```

#### 활용 예시

```cpp
// 1. 프로퍼티 그리드
auto PropertyGrid = new SGridPanel();

PropertyGrid->SetColumnFill(0, 0.4f); // 레이블 열 40%
PropertyGrid->SetColumnFill(1, 0.6f); // 값 열 60%

// Row 0
PropertyGrid->AddSlot(0, 0)
    .SetPadding(5.0f)
    .AttachWidget(new STextBlock("Location:"));

PropertyGrid->AddSlot(1, 0)
    .SetPadding(5.0f)
    .AttachWidget(new SEditableText());

// Row 1
PropertyGrid->AddSlot(0, 1)
    .SetPadding(5.0f)
    .AttachWidget(new STextBlock("Rotation:"));

PropertyGrid->AddSlot(1, 1)
    .SetPadding(5.0f)
    .AttachWidget(new SEditableText());

// Row 2
PropertyGrid->AddSlot(0, 2)
    .SetPadding(5.0f)
    .AttachWidget(new STextBlock("Scale:"));

PropertyGrid->AddSlot(1, 2)
    .SetPadding(5.0f)
    .AttachWidget(new SEditableText());

// 2. 버튼 그리드 (계산기)
auto Calculator = new SGridPanel();

for (int32 Row = 0; Row < 4; Row++)
{
    for (int32 Col = 0; Col < 4; Col++)
    {
        Calculator->AddSlot(Col, Row)
            .SetPadding(2.0f)
            .AttachWidget(CreateCalculatorButton(Row, Col));
    }
}

// 3. 복잡한 레이아웃 (Span 사용)
auto ComplexGrid = new SGridPanel();

// 헤더 (전체 너비)
ComplexGrid->AddSlot(0, 0)
    .SetColumnSpan(3)
    .AttachWidget(new STextBlock("Header"));

// 좌측 패널 (2행 병합)
ComplexGrid->AddSlot(0, 1)
    .SetRowSpan(2)
    .AttachWidget(CreateSidePanel());

// 메인 콘텐츠
ComplexGrid->AddSlot(1, 1)
    .SetColumnSpan(2)
    .AttachWidget(CreateMainContent());

// 하단 버튼들
ComplexGrid->AddSlot(1, 2)
    .AttachWidget(new SButton("OK"));

ComplexGrid->AddSlot(2, 2)
    .AttachWidget(new SButton("Cancel"));
```

---

### 3.4 SScrollBox - 스크롤 가능 컨테이너

#### 구현

```cpp
// SScrollBox.h
class SScrollBox : public SPanel
{
public:
    SScrollBox();

    // ===== 콘텐츠 설정 =====
    void SetContent(SPanel* InContent);
    SPanel* GetContent() const { return ContentWidget; }

    // ===== 스크롤 제어 =====
    void ScrollToTop();
    void ScrollToBottom();
    void ScrollToOffset(float Offset);
    float GetScrollOffset() const { return ScrollOffset; }
    float GetMaxScrollOffset() const { return MaxScrollOffset; }

    // ===== 설정 =====
    void SetOrientation(EOrientation InOrientation) { Orientation = InOrientation; }
    void SetScrollbarThickness(float Thickness) { ScrollbarThickness = Thickness; }

    enum EOrientation
    {
        Orient_Vertical,
        Orient_Horizontal
    };

protected:
    void RenderContent() override;
    void OnMouseMove(FVector2D MousePos) override;
    void OnMouseDown(FVector2D MousePos, uint32 Button) override;
    void OnMouseUp(FVector2D MousePos, uint32 Button) override;
    void OnMouseWheel(float Delta) override;

private:
    void UpdateLayout();
    void RenderScrollbar();
    FRect GetScrollbarRect() const;
    FRect GetScrollbarThumbRect() const;

    SPanel* ContentWidget = nullptr;
    EOrientation Orientation = Orient_Vertical;

    float ScrollOffset = 0.0f;
    float MaxScrollOffset = 0.0f;
    float ScrollbarThickness = 12.0f;

    bool bIsDraggingThumb = false;
    float DragStartOffset = 0.0f;
    FVector2D DragStartMousePos;
};
```

```cpp
// SScrollBox.cpp
void SScrollBox::RenderContent()
{
    if (!ContentWidget) return;

    UpdateLayout();

    // 클리핑 영역 설정
    ImGui::PushClipRect(
        ImVec2(Rect.Left, Rect.Top),
        ImVec2(Rect.Right - ScrollbarThickness, Rect.Bottom),
        true
    );

    // 콘텐츠 렌더링 (오프셋 적용)
    ContentWidget->OnRender();

    ImGui::PopClipRect();

    // 스크롤바 렌더링
    RenderScrollbar();
}

void SScrollBox::UpdateLayout()
{
    if (!ContentWidget) return;

    if (Orientation == Orient_Vertical)
    {
        // 콘텐츠 높이 계산
        float ContentHeight = ContentWidget->GetHeight();
        float ViewportHeight = Rect.GetHeight();

        MaxScrollOffset = FMath::Max(0.0f, ContentHeight - ViewportHeight);
        ScrollOffset = FMath::Clamp(ScrollOffset, 0.0f, MaxScrollOffset);

        // 콘텐츠 위치 조정
        ContentWidget->SetRect(
            Rect.Left,
            Rect.Top - ScrollOffset,
            Rect.Right - ScrollbarThickness,
            Rect.Top - ScrollOffset + ContentHeight
        );
    }
    else
    {
        // 수평 스크롤 로직
        // ...
    }
}

void SScrollBox::RenderScrollbar()
{
    if (MaxScrollOffset <= 0.0f) return;

    ImDrawList* DrawList = ImGui::GetWindowDrawList();

    // 스크롤바 배경
    FRect ScrollbarRect = GetScrollbarRect();
    DrawList->AddRectFilled(
        ImVec2(ScrollbarRect.Left, ScrollbarRect.Top),
        ImVec2(ScrollbarRect.Right, ScrollbarRect.Bottom),
        IM_COL32(40, 40, 40, 255)
    );

    // 스크롤바 Thumb
    FRect ThumbRect = GetScrollbarThumbRect();
    ImU32 ThumbColor = bIsDraggingThumb ?
        IM_COL32(120, 120, 120, 255) :
        IM_COL32(80, 80, 80, 255);

    DrawList->AddRectFilled(
        ImVec2(ThumbRect.Left, ThumbRect.Top),
        ImVec2(ThumbRect.Right, ThumbRect.Bottom),
        ThumbColor,
        4.0f
    );
}

FRect SScrollBox::GetScrollbarThumbRect() const
{
    FRect ScrollbarRect = GetScrollbarRect();

    float ViewportHeight = Rect.GetHeight();
    float ContentHeight = ContentWidget ? ContentWidget->GetHeight() : 0.0f;

    // Thumb 높이 (비율)
    float ThumbRatio = ViewportHeight / ContentHeight;
    float ThumbHeight = ScrollbarRect.GetHeight() * ThumbRatio;
    ThumbHeight = FMath::Max(ThumbHeight, 30.0f); // 최소 크기

    // Thumb 위치
    float ScrollRatio = ScrollOffset / MaxScrollOffset;
    float ThumbY = ScrollbarRect.Top +
                   (ScrollbarRect.GetHeight() - ThumbHeight) * ScrollRatio;

    return FRect(
        ScrollbarRect.Left + 2,
        ThumbY,
        ScrollbarRect.Right - 2,
        ThumbY + ThumbHeight
    );
}

void SScrollBox::OnMouseWheel(float Delta)
{
    float ScrollSpeed = 20.0f;
    ScrollOffset -= Delta * ScrollSpeed;
    ScrollOffset = FMath::Clamp(ScrollOffset, 0.0f, MaxScrollOffset);
}
```

#### 활용 예시

```cpp
// 1. 긴 리스트 스크롤
auto ScrollableList = new SScrollBox();

auto ListContent = new SVerticalBox();
for (int32 i = 0; i < 100; i++)
{
    ListContent->AddSlot()
        .AutoHeight()
        .SetPadding(2.0f)
        .AttachWidget(new STextBlock(FString::Printf("Item %d", i)));
}

ScrollableList->SetContent(ListContent);

// 2. Emitter 패널 (스크롤 가능)
auto EmitterListScroll = new SScrollBox();
auto EmitterList = new SParticleEmitterPanel();
EmitterListScroll->SetContent(EmitterList);

// 3. 로그 뷰어
auto LogViewer = new SScrollBox();
LogViewer->SetContent(CreateLogContent());
LogViewer->ScrollToBottom(); // 자동으로 최신 로그로 스크롤
```

---

## 4. 고급 위젯

### 4.1 STreeView - 트리 구조 위젯

계층적 데이터를 표시하는 트리 뷰입니다.

#### 구현

```cpp
// STreeView.h
template<typename ItemType>
class STreeView : public SPanel
{
public:
    // ===== 델리게이트 정의 =====
    TDelegate<void(ItemType)> OnSelectionChanged;
    TDelegate<void(ItemType)> OnItemDoubleClicked;
    TDelegate<void(ItemType, bool)> OnExpansionChanged;

    // 데이터 제공 델리게이트
    TDelegate<TArray<ItemType>(ItemType)> OnGetChildren;
    TDelegate<FString(ItemType)> OnGenerateItemText;
    TDelegate<UTexture*(ItemType)> OnGenerateItemIcon;
    TDelegate<bool(ItemType)> OnIsItemExpanded;

    STreeView();

    // ===== 데이터 =====
    void SetRootItems(const TArray<ItemType>& Items);
    const TArray<ItemType>& GetRootItems() const { return RootItems; }

    // ===== 선택 =====
    void SetSelectedItem(ItemType Item);
    ItemType GetSelectedItem() const { return SelectedItem; }
    void ClearSelection();

    // ===== 확장/축소 =====
    void SetItemExpansion(ItemType Item, bool bExpanded);
    bool IsItemExpanded(ItemType Item) const;
    void ExpandAll();
    void CollapseAll();

    // ===== 스타일 =====
    void SetIndentSize(float Size) { IndentSize = Size; }
    void SetItemHeight(float Height) { ItemHeight = Height; }

protected:
    void RenderContent() override;
    void OnMouseDown(FVector2D MousePos, uint32 Button) override;
    void OnMouseUp(FVector2D MousePos, uint32 Button) override;

private:
    void RenderTreeNode(ItemType Item, int32 Depth, float& CurrentY);
    void RenderExpandButton(FVector2D Pos, bool bExpanded);

    ItemType GetItemAtPosition(FVector2D MousePos, float& OutDepth);

    TArray<ItemType> RootItems;
    ItemType SelectedItem;
    TMap<ItemType, bool> ExpansionStates;

    float IndentSize = 20.0f;
    float ItemHeight = 24.0f;
    float ExpandButtonSize = 16.0f;
};
```

```cpp
// STreeView.cpp
template<typename ItemType>
void STreeView<ItemType>::RenderContent()
{
    float CurrentY = Rect.Top;

    for (ItemType RootItem : RootItems)
    {
        RenderTreeNode(RootItem, 0, CurrentY);
    }
}

template<typename ItemType>
void STreeView<ItemType>::RenderTreeNode(ItemType Item, int32 Depth, float& CurrentY)
{
    if (CurrentY > Rect.Bottom) return;

    ImDrawList* DrawList = ImGui::GetWindowDrawList();

    float Indent = Rect.Left + Depth * IndentSize;
    FRect ItemRect(Indent, CurrentY, Rect.Right, CurrentY + ItemHeight);

    // 배경 (선택 상태)
    if (Item == SelectedItem)
    {
        DrawList->AddRectFilled(
            ImVec2(ItemRect.Left, ItemRect.Top),
            ImVec2(ItemRect.Right, ItemRect.Bottom),
            IM_COL32(0, 120, 215, 100)
        );
    }

    // 자식 있는지 확인
    TArray<ItemType> Children;
    if (OnGetChildren.IsBound())
    {
        Children = OnGetChildren.Execute(Item);
    }

    bool bHasChildren = Children.Num() > 0;
    bool bIsExpanded = IsItemExpanded(Item);

    // 확장 버튼
    if (bHasChildren)
    {
        float ButtonX = Indent;
        float ButtonY = CurrentY + (ItemHeight - ExpandButtonSize) * 0.5f;
        RenderExpandButton(FVector2D(ButtonX, ButtonY), bIsExpanded);
    }

    // 아이콘
    float ContentX = Indent + (bHasChildren ? ExpandButtonSize + 4 : 4);
    if (OnGenerateItemIcon.IsBound())
    {
        if (UTexture* Icon = OnGenerateItemIcon.Execute(Item))
        {
            // 아이콘 렌더링
            ContentX += 20.0f;
        }
    }

    // 텍스트
    FString Text = OnGenerateItemText.IsBound() ?
                   OnGenerateItemText.Execute(Item) :
                   "Item";

    ImVec2 TextPos(ContentX, CurrentY + (ItemHeight - ImGui::GetTextLineHeight()) * 0.5f);
    DrawList->AddText(TextPos, IM_COL32(255, 255, 255, 255), Text.c_str());

    CurrentY += ItemHeight;

    // 자식 렌더링 (확장된 경우)
    if (bHasChildren && bIsExpanded)
    {
        for (ItemType Child : Children)
        {
            RenderTreeNode(Child, Depth + 1, CurrentY);
        }
    }
}

template<typename ItemType>
void STreeView<ItemType>::RenderExpandButton(FVector2D Pos, bool bExpanded)
{
    ImDrawList* DrawList = ImGui::GetWindowDrawList();

    ImVec2 Center(Pos.X + ExpandButtonSize * 0.5f, Pos.Y + ExpandButtonSize * 0.5f);
    float Size = ExpandButtonSize * 0.3f;

    if (bExpanded)
    {
        // 아래 화살표 (▼)
        ImVec2 P1(Center.x - Size, Center.y - Size * 0.3f);
        ImVec2 P2(Center.x + Size, Center.y - Size * 0.3f);
        ImVec2 P3(Center.x, Center.y + Size * 0.7f);
        DrawList->AddTriangleFilled(P1, P2, P3, IM_COL32(200, 200, 200, 255));
    }
    else
    {
        // 오른쪽 화살표 (▶)
        ImVec2 P1(Center.x - Size * 0.3f, Center.y - Size);
        ImVec2 P2(Center.x - Size * 0.3f, Center.y + Size);
        ImVec2 P3(Center.x + Size * 0.7f, Center.y);
        DrawList->AddTriangleFilled(P1, P2, P3, IM_COL32(200, 200, 200, 255));
    }
}

template<typename ItemType>
void STreeView<ItemType>::OnMouseDown(FVector2D MousePos, uint32 Button)
{
    if (!IsHover(MousePos)) return;

    float ClickDepth = 0.0f;
    ItemType ClickedItem = GetItemAtPosition(MousePos, ClickDepth);

    if (ClickedItem != nullptr)
    {
        float Indent = Rect.Left + ClickDepth * IndentSize;

        // 확장 버튼 클릭 체크
        if (MousePos.X >= Indent && MousePos.X <= Indent + ExpandButtonSize)
        {
            bool bExpanded = IsItemExpanded(ClickedItem);
            SetItemExpansion(ClickedItem, !bExpanded);

            if (OnExpansionChanged.IsBound())
            {
                OnExpansionChanged.Execute(ClickedItem, !bExpanded);
            }
        }
        else
        {
            // 아이템 선택
            SetSelectedItem(ClickedItem);

            if (OnSelectionChanged.IsBound())
            {
                OnSelectionChanged.Execute(ClickedItem);
            }
        }
    }
}
```

#### 활용 예시 - 파티클 Emitter 트리

```cpp
// Emitter/Module 트리 구조
struct FParticleTreeItem
{
    enum EType { Emitter, Module };

    EType Type;
    void* Data;  // UParticleEmitter* 또는 UParticleModule*
    FString Name;
    int32 Index;
};

class SParticleEmitterPanel : public SPanel
{
private:
    STreeView<FParticleTreeItem*>* TreeView = nullptr;

    void SetupTreeView()
    {
        TreeView = new STreeView<FParticleTreeItem*>();

        // 자식 가져오기
        TreeView->OnGetChildren.Bind([this](FParticleTreeItem* Item) -> TArray<FParticleTreeItem*> {
            TArray<FParticleTreeItem*> Children;

            if (Item->Type == FParticleTreeItem::Emitter)
            {
                UParticleEmitter* Emitter = (UParticleEmitter*)Item->Data;
                UParticleLODLevel* LOD = Emitter->LODLevels[0];

                // Required Module
                auto RequiredItem = new FParticleTreeItem();
                RequiredItem->Type = FParticleTreeItem::Module;
                RequiredItem->Data = LOD->RequiredModule;
                RequiredItem->Name = "Required";
                Children.Add(RequiredItem);

                // Modules
                for (int32 i = 0; i < LOD->Modules.Num(); i++)
                {
                    auto ModuleItem = new FParticleTreeItem();
                    ModuleItem->Type = FParticleTreeItem::Module;
                    ModuleItem->Data = LOD->Modules[i];
                    ModuleItem->Name = LOD->Modules[i]->GetClass()->Name;
                    ModuleItem->Index = i;
                    Children.Add(ModuleItem);
                }
            }

            return Children;
        });

        // 텍스트 생성
        TreeView->OnGenerateItemText.Bind([](FParticleTreeItem* Item) -> FString {
            return Item->Name;
        });

        // 아이콘 생성
        TreeView->OnGenerateItemIcon.Bind([this](FParticleTreeItem* Item) -> UTexture* {
            if (Item->Type == FParticleTreeItem::Emitter)
                return IconEmitter;
            else
                return IconModule;
        });

        // 선택 변경
        TreeView->OnSelectionChanged.Bind([this](FParticleTreeItem* Item) {
            if (Item->Type == FParticleTreeItem::Module)
            {
                UParticleModule* Module = (UParticleModule*)Item->Data;
                OnModuleSelected.Execute(Module);
            }
        });

        // 더블 클릭
        TreeView->OnItemDoubleClicked.Bind([this](FParticleTreeItem* Item) {
            if (Item->Type == FParticleTreeItem::Module)
            {
                // 모듈 편집 다이얼로그 열기
            }
        });
    }

    void RefreshTreeView()
    {
        TArray<FParticleTreeItem*> RootItems;

        for (int32 i = 0; i < ParticleSystem->Emitters.Num(); i++)
        {
            auto EmitterItem = new FParticleTreeItem();
            EmitterItem->Type = FParticleTreeItem::Emitter;
            EmitterItem->Data = ParticleSystem->Emitters[i];
            EmitterItem->Name = FString::Printf("Emitter %d", i);
            EmitterItem->Index = i;
            RootItems.Add(EmitterItem);
        }

        TreeView->SetRootItems(RootItems);
        TreeView->ExpandAll(); // 모두 펼침
    }
};
```

---

### 4.2 SComboBox - 드롭다운 메뉴

#### 구현

```cpp
// SComboBox.h
template<typename OptionType>
class SComboBox : public SPanel
{
public:
    // ===== 델리게이트 =====
    TDelegate<void(OptionType)> OnSelectionChanged;
    TDelegate<FString(OptionType)> OnGenerateItemText;

    SComboBox();

    // ===== 옵션 설정 =====
    void SetOptions(const TArray<OptionType>& InOptions);
    void AddOption(OptionType Option);
    void ClearOptions();

    // ===== 선택 =====
    void SetSelectedOption(OptionType Option);
    OptionType GetSelectedOption() const { return SelectedOption; }

    // ===== 스타일 =====
    void SetDropdownHeight(float Height) { DropdownHeight = Height; }

protected:
    void RenderContent() override;
    void OnMouseDown(FVector2D MousePos, uint32 Button) override;

private:
    void OpenDropdown();
    void CloseDropdown();
    void RenderDropdownPopup();

    TArray<OptionType> Options;
    OptionType SelectedOption;
    bool bIsDropdownOpen = false;
    float DropdownHeight = 200.0f;
};
```

```cpp
// SComboBox.cpp
template<typename OptionType>
void SComboBox<OptionType>::RenderContent()
{
    ImDrawList* DrawList = ImGui::GetWindowDrawList();

    // 배경
    DrawList->AddRectFilled(
        ImVec2(Rect.Left, Rect.Top),
        ImVec2(Rect.Right, Rect.Bottom),
        IM_COL32(60, 60, 60, 255),
        4.0f
    );

    // 선택된 텍스트
    FString DisplayText = OnGenerateItemText.IsBound() ?
                         OnGenerateItemText.Execute(SelectedOption) :
                         "Select...";

    ImVec2 TextPos(
        Rect.Left + 8,
        Rect.Top + (Rect.GetHeight() - ImGui::GetTextLineHeight()) * 0.5f
    );
    DrawList->AddText(TextPos, IM_COL32(255, 255, 255, 255), DisplayText.c_str());

    // 드롭다운 화살표
    float ArrowX = Rect.Right - 20;
    float ArrowY = Rect.Top + Rect.GetHeight() * 0.5f;
    float ArrowSize = 6.0f;

    ImVec2 P1(ArrowX - ArrowSize, ArrowY - ArrowSize * 0.3f);
    ImVec2 P2(ArrowX + ArrowSize, ArrowY - ArrowSize * 0.3f);
    ImVec2 P3(ArrowX, ArrowY + ArrowSize * 0.7f);
    DrawList->AddTriangleFilled(P1, P2, P3, IM_COL32(200, 200, 200, 255));

    // 드롭다운 팝업
    if (bIsDropdownOpen)
    {
        RenderDropdownPopup();
    }
}

template<typename OptionType>
void SComboBox<OptionType>::RenderDropdownPopup()
{
    ImGui::SetNextWindowPos(ImVec2(Rect.Left, Rect.Bottom));
    ImGui::SetNextWindowSize(ImVec2(Rect.GetWidth(), DropdownHeight));

    if (ImGui::BeginPopup("##ComboDropdown", ImGuiWindowFlags_NoMove))
    {
        for (int32 i = 0; i < Options.Num(); i++)
        {
            OptionType Option = Options[i];
            FString Text = OnGenerateItemText.IsBound() ?
                          OnGenerateItemText.Execute(Option) :
                          FString::Printf("Option %d", i);

            bool bIsSelected = (Option == SelectedOption);
            if (ImGui::Selectable(Text.c_str(), bIsSelected))
            {
                SetSelectedOption(Option);
                CloseDropdown();
            }
        }

        ImGui::EndPopup();
    }
    else
    {
        bIsDropdownOpen = false;
    }
}

template<typename OptionType>
void SComboBox<OptionType>::OnMouseDown(FVector2D MousePos, uint32 Button)
{
    if (IsHover(MousePos))
    {
        if (bIsDropdownOpen)
        {
            CloseDropdown();
        }
        else
        {
            OpenDropdown();
        }
    }
}

template<typename OptionType>
void SComboBox<OptionType>::OpenDropdown()
{
    bIsDropdownOpen = true;
    ImGui::OpenPopup("##ComboDropdown");
}
```

#### 활용 예시

```cpp
// 1. Emitter Type 선택
auto EmitterTypeCombo = new SComboBox<FString>();
EmitterTypeCombo->SetOptions({"Sprite", "Mesh", "Ribbon", "Beam"});
EmitterTypeCombo->OnGenerateItemText.Bind([](FString Type) -> FString {
    return Type + " Emitter";
});
EmitterTypeCombo->OnSelectionChanged.Bind([this](FString Type) {
    CreateEmitterOfType(Type);
});

// 2. Material 선택
auto MaterialCombo = new SComboBox<UMaterial*>();
MaterialCombo->SetOptions(GetAllMaterials());
MaterialCombo->OnGenerateItemText.Bind([](UMaterial* Mat) -> FString {
    return Mat->GetName();
});
MaterialCombo->OnSelectionChanged.Bind([this](UMaterial* Mat) {
    ApplyMaterial(Mat);
});

// 3. Enum 선택
enum class EBlendMode { Opaque, Masked, Translucent };

auto BlendModeCombo = new SComboBox<EBlendMode>();
BlendModeCombo->SetOptions({
    EBlendMode::Opaque,
    EBlendMode::Masked,
    EBlendMode::Translucent
});
BlendModeCombo->OnGenerateItemText.Bind([](EBlendMode Mode) -> FString {
    switch(Mode)
    {
        case EBlendMode::Opaque: return "Opaque";
        case EBlendMode::Masked: return "Masked";
        case EBlendMode::Translucent: return "Translucent";
    }
    return "Unknown";
});
```

---

### 4.3 SColorPicker - 색상 선택 위젯

#### 구현

```cpp
// SColorPicker.h
class SColorPicker : public SPanel
{
public:
    // ===== 델리게이트 =====
    TDelegate<void(const FLinearColor&)> OnColorChanged;
    TDelegate<void(const FLinearColor&)> OnColorCommitted;

    SColorPicker();

    // ===== 색상 =====
    void SetColor(const FLinearColor& InColor);
    FLinearColor GetColor() const { return CurrentColor; }

    // ===== 모드 =====
    enum EPickerMode
    {
        Mode_RGB,
        Mode_HSV
    };
    void SetMode(EPickerMode InMode) { Mode = InMode; }

protected:
    void RenderContent() override;

private:
    void RenderColorSquare();    // Saturation/Value 선택
    void RenderHueBar();         // Hue 선택
    void RenderAlphaBar();       // Alpha 선택
    void RenderColorPreview();   // 현재 색상 미리보기
    void RenderColorValues();    // RGB/HSV 값 입력

    FLinearColor CurrentColor;
    EPickerMode Mode = Mode_HSV;
};
```

#### 활용 예시

```cpp
// 프로퍼티 편집에서 색상 선택
void SParticleDetailsPanel::RenderColorProperty(const FProperty& Prop, void* ValuePtr)
{
    FLinearColor* Color = (FLinearColor*)ValuePtr;

    ImGui::Text("%s", Prop.Name.c_str());
    ImGui::SameLine();

    // ImGui의 ColorEdit4 사용 (또는 커스텀 SColorPicker)
    float ColorArray[4] = { Color->R, Color->G, Color->B, Color->A };

    if (ImGui::ColorEdit4("##Color", ColorArray))
    {
        Color->R = ColorArray[0];
        Color->G = ColorArray[1];
        Color->B = ColorArray[2];
        Color->A = ColorArray[3];

        if (OnPropertyChanged.IsBound())
        {
            OnPropertyChanged.Execute(SelectedModule, Prop);
        }
    }
}
```

---

## 5. 실전 에디터 제작

### 5.1 Cascade 파티클 에디터 완전 구현

#### 전체 구조

```cpp
// SParticleSystemEditorWindow.h
#pragma once
#include "SWindow.h"

class SParticleSystemEditorWindow : public SWindow
{
public:
    SParticleSystemEditorWindow();
    virtual ~SParticleSystemEditorWindow();

    bool Initialize(
        float X, float Y, float Width, float Height,
        UWorld* World, ID3D11Device* Device);

    void SetParticleSystem(UParticleSystem* System);

    virtual void OnRender() override;
    virtual void OnUpdate(float DeltaSeconds) override;

private:
    // ===== UI 구성 =====
    void BuildLayout();
    SPanel* CreateToolbar();
    SPanel* CreateMainContent();
    SPanel* CreateStatusBar();

    // ===== 패널들 =====
    class SParticleEmitterPanel* EmitterPanel = nullptr;
    class SParticleDetailsPanel* DetailsPanel = nullptr;
    class SParticleViewportPanel* ViewportPanel = nullptr;
    class SParticleCurveEditorPanel* CurvePanel = nullptr;

    // ===== 레이아웃 =====
    SVerticalBox* RootLayout = nullptr;
    SSplitterH* MainSplitter = nullptr;
    SSplitterV* LeftSplitter = nullptr;
    SSplitterV* RightSplitter = nullptr;

    // ===== 데이터 =====
    UParticleSystem* EditingSystem = nullptr;
    UWorld* World = nullptr;
    ID3D11Device* Device = nullptr;

    // ===== UI 상태 =====
    bool bShowCurveEditor = false;
    bool bIsOpen = true;
};
```

#### 구현

```cpp
// SParticleSystemEditorWindow.cpp
bool SParticleSystemEditorWindow::Initialize(
    float X, float Y, float Width, float Height,
    UWorld* InWorld, ID3D11Device* InDevice)
{
    World = InWorld;
    Device = InDevice;

    SetRect(X, Y, X + Width, Y + Height);

    BuildLayout();

    return true;
}

void SParticleSystemEditorWindow::BuildLayout()
{
    // 루트 레이아웃 (수직)
    RootLayout = new SVerticalBox();

    // 1. 툴바 (고정 높이)
    RootLayout->AddSlot()
        .FixedHeight(40.0f)
        .AttachWidget(CreateToolbar());

    // 2. 메인 콘텐츠 (남은 공간 채움)
    RootLayout->AddSlot()
        .FillHeight(1.0f)
        .AttachWidget(CreateMainContent());

    // 3. 상태바 (고정 높이)
    RootLayout->AddSlot()
        .FixedHeight(24.0f)
        .AttachWidget(CreateStatusBar());
}

SPanel* SParticleSystemEditorWindow::CreateToolbar()
{
    auto Toolbar = new SHorizontalBox();

    // Save 버튼
    auto SaveButton = new SButton();
    SaveButton->SetText("Save");
    SaveButton->OnClicked.Bind([this]() {
        SaveParticleSystem();
    });

    Toolbar->AddSlot()
        .AutoWidth()
        .SetPadding(2.0f)
        .AttachWidget(SaveButton);

    // Restart 버튼
    auto RestartButton = new SButton();
    RestartButton->SetText("Restart");
    RestartButton->OnClicked.Bind([this]() {
        if (ViewportPanel)
            ViewportPanel->Restart();
    });

    Toolbar->AddSlot()
        .AutoWidth()
        .SetPadding(2.0f)
        .AttachWidget(RestartButton);

    // Spacer
    Toolbar->AddSlot()
        .FillWidth(1.0f)
        .AttachWidget(new SPanel());

    // Curve Editor 토글
    auto CurveToggle = new SButton();
    CurveToggle->SetText("Curves");
    CurveToggle->OnClicked.Bind([this]() {
        bShowCurveEditor = !bShowCurveEditor;
        CurvePanel->SetVisible(bShowCurveEditor);
    });

    Toolbar->AddSlot()
        .AutoWidth()
        .SetPadding(2.0f)
        .AttachWidget(CurveToggle);

    return Toolbar;
}

SPanel* SParticleSystemEditorWindow::CreateMainContent()
{
    // 메인 스플리터 (좌우 30:70)
    MainSplitter = new SSplitterH();
    MainSplitter->SetSplitRatio(0.3f);

    // 왼쪽 스플리터 (상하 60:40)
    LeftSplitter = new SSplitterV();
    LeftSplitter->SetSplitRatio(0.6f);

    // Emitter 패널
    EmitterPanel = new SParticleEmitterPanel();
    EmitterPanel->OnModuleSelected.Bind([this](UParticleModule* Module) {
        if (DetailsPanel)
        {
            DetailsPanel->SetSelectedModule(Module);
        }
    });

    // Details 패널
    DetailsPanel = new SParticleDetailsPanel();

    LeftSplitter->SetLeftOrTop(EmitterPanel);
    LeftSplitter->SetRightOrBottom(DetailsPanel);

    // 오른쪽 스플리터 (상하 70:30)
    RightSplitter = new SSplitterV();
    RightSplitter->SetSplitRatio(0.7f);

    // Viewport 패널
    ViewportPanel = new SParticleViewportPanel();
    ViewportPanel->Initialize(World, Device);

    // Curve Editor 패널
    CurvePanel = new SParticleCurveEditorPanel();
    CurvePanel->SetVisible(false);

    RightSplitter->SetLeftOrTop(ViewportPanel);
    RightSplitter->SetRightOrBottom(CurvePanel);

    // 조립
    MainSplitter->SetLeftOrTop(LeftSplitter);
    MainSplitter->SetRightOrBottom(RightSplitter);

    return MainSplitter;
}

SPanel* SParticleSystemEditorWindow::CreateStatusBar()
{
    auto StatusBar = new SHorizontalBox();

    // 상태 텍스트
    auto StatusText = new STextBlock();
    StatusText->SetText([this]() -> FString {
        if (!EditingSystem)
            return "No particle system";

        return FString::Printf("Emitters: %d", EditingSystem->Emitters.Num());
    });

    StatusBar->AddSlot()
        .FillWidth(1.0f)
        .SetPadding(5.0f, 0.0f)
        .AttachWidget(StatusText);

    // 파티클 카운트
    auto ParticleCount = new STextBlock();
    ParticleCount->SetText([this]() -> FString {
        int32 Total = 0;
        if (ViewportPanel && ViewportPanel->GetParticleSystemComponent())
        {
            auto PSC = ViewportPanel->GetParticleSystemComponent();
            for (auto& Instance : PSC->EmitterInstances)
            {
                Total += Instance->ActiveParticles;
            }
        }
        return FString::Printf("Particles: %d", Total);
    });

    StatusBar->AddSlot()
        .AutoWidth()
        .SetPadding(5.0f, 0.0f)
        .AttachWidget(ParticleCount);

    return StatusBar;
}

void SParticleSystemEditorWindow::OnRender()
{
    if (!bIsOpen) return;

    ImGui::SetNextWindowPos(ImVec2(Rect.Left, Rect.Top));
    ImGui::SetNextWindowSize(ImVec2(Rect.GetWidth(), Rect.GetHeight()));

    ImGui::Begin("Cascade - Particle System Editor", &bIsOpen,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    // 루트 레이아웃 렌더링
    if (RootLayout)
    {
        RootLayout->SetRect(Rect);
        RootLayout->OnRender();
    }

    ImGui::End();
}

void SParticleSystemEditorWindow::SetParticleSystem(UParticleSystem* System)
{
    EditingSystem = System;

    if (EmitterPanel)
        EmitterPanel->SetParticleSystem(System);

    if (ViewportPanel)
    {
        auto PSC = NewObject<UParticleSystemComponent>();
        PSC->SetTemplate(System);
        ViewportPanel->SetParticleSystemComponent(PSC);
    }
}
```

---

### 5.2 Material Editor (노드 기반 에디터)

#### 노드 그래프 패널

```cpp
// SMaterialGraphPanel.h
class SMaterialGraphPanel : public SPanel
{
public:
    struct FMaterialNode
    {
        FString NodeType;
        FVector2D Position;
        FVector2D Size;
        TArray<FString> InputPins;
        TArray<FString> OutputPins;
        void* NodeData;  // UMaterialExpression*
    };

    struct FWire
    {
        FMaterialNode* StartNode;
        int32 StartPinIndex;
        FMaterialNode* EndNode;
        int32 EndPinIndex;
    };

    SMaterialGraphPanel();

    // ===== 노드 관리 =====
    void AddNode(const FString& NodeType, FVector2D Position);
    void RemoveNode(FMaterialNode* Node);
    void SelectNode(FMaterialNode* Node);

    // ===== 와이어 관리 =====
    void ConnectPins(FMaterialNode* Start, int32 StartPin,
                    FMaterialNode* End, int32 EndPin);
    void RemoveWire(FWire* Wire);

protected:
    void RenderContent() override;
    void OnMouseDown(FVector2D MousePos, uint32 Button) override;
    void OnMouseMove(FVector2D MousePos) override;
    void OnMouseUp(FVector2D MousePos, uint32 Button) override;

private:
    void RenderGrid();
    void RenderWires();
    void RenderNodes();
    void RenderNode(FMaterialNode* Node);
    void RenderPin(FVector2D Pos, bool bIsInput);

    FVector2D ScreenToGraph(FVector2D ScreenPos);
    FVector2D GraphToScreen(FVector2D GraphPos);

    TArray<FMaterialNode*> Nodes;
    TArray<FWire*> Wires;
    FMaterialNode* SelectedNode = nullptr;

    // 카메라
    FVector2D ViewOffset = FVector2D(0, 0);
    float ViewZoom = 1.0f;

    // 드래그 상태
    enum EDragMode
    {
        None,
        PanView,
        DragNode,
        ConnectWire,
        BoxSelect
    };
    EDragMode CurrentDragMode = None;
};
```

---

### 5.3 Animation Timeline Editor

```cpp
// SAnimationTimeline.h
class SAnimationTimeline : public SPanel
{
public:
    struct FAnimationTrack
    {
        FString Name;
        TArray<FKeyframe> Keys;
        bool bIsExpanded = true;
    };

    struct FKeyframe
    {
        float Time;
        FVariant Value;  // float, FVector, FQuat 등
        bool bIsSelected = false;
    };

    SAnimationTimeline();

    // ===== 트랙 관리 =====
    void AddTrack(const FString& Name);
    void RemoveTrack(int32 TrackIndex);

    // ===== 키프레임 =====
    void AddKeyframe(int32 TrackIndex, float Time, const FVariant& Value);
    void RemoveKeyframe(int32 TrackIndex, int32 KeyIndex);
    void MoveKeyframe(int32 TrackIndex, int32 KeyIndex, float NewTime);

    // ===== 재생 =====
    void SetCurrentTime(float Time);
    float GetCurrentTime() const { return CurrentTime; }
    void SetTimeRange(float Start, float End);

protected:
    void RenderContent() override;

private:
    void RenderTimeRuler();
    void RenderTracks();
    void RenderTrack(const FAnimationTrack& Track, int32 Index);
    void RenderKeyframes(const FAnimationTrack& Track);
    void RenderPlayhead();

    TArray<FAnimationTrack> Tracks;
    float CurrentTime = 0.0f;
    float StartTime = 0.0f;
    float EndTime = 5.0f;
    float PixelsPerSecond = 100.0f;
};
```

---

## 6. 최적화 및 고급 기법

### 6.1 Invalidation (선택적 리렌더링)

불필요한 렌더링을 줄여 성능을 향상시킵니다.

```cpp
// SPanel에 Invalidation 추가
class SPanel : public SWindow
{
protected:
    bool bNeedsRepaint = true;

public:
    void Invalidate()
    {
        bNeedsRepaint = true;

        // 부모도 invalidate
        if (ParentPanel)
        {
            ParentPanel->Invalidate();
        }
    }

    void OnRender() override
    {
        // 변경 없으면 스킵
        if (!bNeedsRepaint && !bAlwaysRepaint)
            return;

        RenderBackground();
        RenderContent();
        RenderChildren();

        bNeedsRepaint = false;
    }

    // 프로퍼티 변경 시 Invalidate 호출
    void SetVisible(bool bInVisible)
    {
        if (bIsVisible != bInVisible)
        {
            bIsVisible = bInVisible;
            Invalidate();
        }
    }
};
```

---

### 6.2 Attribute 시스템 (동적 바인딩)

델리게이트를 통해 동적으로 값을 업데이트합니다.

```cpp
// TAttribute 템플릿
template<typename T>
class TAttribute
{
public:
    TAttribute() : bHasValue(false) {}
    TAttribute(const T& InValue) : Value(InValue), bHasValue(true) {}
    TAttribute(TDelegate<T()> InGetter) : Getter(InGetter), bHasValue(false) {}

    T Get() const
    {
        return Getter.IsBound() ? Getter.Execute() : Value;
    }

    bool IsBound() const { return Getter.IsBound(); }

private:
    T Value;
    TDelegate<T()> Getter;
    bool bHasValue;
};

// 사용 예시
class STextBlock : public SPanel
{
public:
    void SetText(const FString& InText)
    {
        TextAttribute = TAttribute<FString>(InText);
    }

    void SetText(TDelegate<FString()> InTextGetter)
    {
        TextAttribute = TAttribute<FString>(InTextGetter);
    }

    FString GetText() const
    {
        return TextAttribute.Get();
    }

private:
    TAttribute<FString> TextAttribute;
};

// 동적 텍스트 예시
auto DynamicText = new STextBlock();
DynamicText->SetText([this]() -> FString {
    return FString::Printf("FPS: %.1f", GetCurrentFPS());
});
```

---

### 6.3 스타일 시스템

일관된 UI 스타일을 적용합니다.

```cpp
// FSlateStyle.h
struct FSlateStyle
{
    // 색상
    ImU32 PrimaryColor = IM_COL32(0, 120, 215, 255);
    ImU32 SecondaryColor = IM_COL32(60, 60, 60, 255);
    ImU32 BackgroundColor = IM_COL32(45, 45, 48, 255);
    ImU32 TextColor = IM_COL32(255, 255, 255, 255);
    ImU32 BorderColor = IM_COL32(80, 80, 80, 255);

    // 폰트
    FSlateFontInfo TitleFont;
    FSlateFontInfo NormalFont;
    FSlateFontInfo SmallFont;

    // 간격
    float DefaultPadding = 5.0f;
    float BorderThickness = 1.0f;

    // 버튼 스타일
    struct FButtonStyle
    {
        ImU32 NormalColor = IM_COL32(60, 60, 60, 255);
        ImU32 HoveredColor = IM_COL32(80, 80, 80, 255);
        ImU32 PressedColor = IM_COL32(40, 40, 40, 255);
    } ButtonStyle;
};

// 전역 스타일 레지스트리
class FSlateStyleRegistry
{
public:
    static void RegisterStyle(const FString& Name, const FSlateStyle& Style)
    {
        Styles[Name] = Style;
    }

    static const FSlateStyle* GetStyle(const FString& Name)
    {
        return Styles.Find(Name);
    }

private:
    static TMap<FString, FSlateStyle> Styles;
};

// 다크 테마
FSlateStyle DarkTheme;
DarkTheme.BackgroundColor = IM_COL32(30, 30, 30, 255);
DarkTheme.TextColor = IM_COL32(220, 220, 220, 255);
FSlateStyleRegistry::RegisterStyle("Dark", DarkTheme);

// 라이트 테마
FSlateStyle LightTheme;
LightTheme.BackgroundColor = IM_COL32(240, 240, 240, 255);
LightTheme.TextColor = IM_COL32(30, 30, 30, 255);
FSlateStyleRegistry::RegisterStyle("Light", LightTheme);
```

---

### 6.4 Undo/Redo 시스템

편집 작업을 되돌릴 수 있는 시스템입니다.

```cpp
// FUndoCommand.h
class FUndoCommand
{
public:
    virtual ~FUndoCommand() {}
    virtual void Undo() = 0;
    virtual void Redo() = 0;
    virtual FString GetDescription() const = 0;
};

// UndoManager.h
class FUndoManager
{
public:
    void PushCommand(FUndoCommand* Command)
    {
        // Redo 스택 클리어
        for (auto* Cmd : RedoStack)
            delete Cmd;
        RedoStack.clear();

        UndoStack.push_back(Command);

        // 최대 크기 제한
        if (UndoStack.Num() > MaxUndoLevels)
        {
            delete UndoStack[0];
            UndoStack.RemoveAt(0);
        }
    }

    void Undo()
    {
        if (UndoStack.Num() == 0) return;

        FUndoCommand* Cmd = UndoStack.back();
        Cmd->Undo();

        UndoStack.pop_back();
        RedoStack.push_back(Cmd);
    }

    void Redo()
    {
        if (RedoStack.Num() == 0) return;

        FUndoCommand* Cmd = RedoStack.back();
        Cmd->Redo();

        RedoStack.pop_back();
        UndoStack.push_back(Cmd);
    }

    bool CanUndo() const { return UndoStack.Num() > 0; }
    bool CanRedo() const { return RedoStack.Num() > 0; }

private:
    TArray<FUndoCommand*> UndoStack;
    TArray<FUndoCommand*> RedoStack;
    int32 MaxUndoLevels = 100;
};

// 사용 예시
class FChangePropertyCommand : public FUndoCommand
{
public:
    FChangePropertyCommand(
        UObject* InObject,
        const FProperty& InProperty,
        const FVariant& InOldValue,
        const FVariant& InNewValue)
        : Object(InObject)
        , Property(InProperty)
        , OldValue(InOldValue)
        , NewValue(InNewValue)
    {}

    void Undo() override
    {
        // 프로퍼티 값을 이전 값으로
        void* PropPtr = (uint8*)Object + Property.Offset;
        memcpy(PropPtr, &OldValue, Property.Size);
    }

    void Redo() override
    {
        // 프로퍼티 값을 새 값으로
        void* PropPtr = (uint8*)Object + Property.Offset;
        memcpy(PropPtr, &NewValue, Property.Size);
    }

    FString GetDescription() const override
    {
        return FString::Printf("Change %s", Property.Name.c_str());
    }

private:
    UObject* Object;
    FProperty Property;
    FVariant OldValue;
    FVariant NewValue;
};
```

---

## 7. 단계별 구현 로드맵

### Phase 1: 기초 인프라 (1주)

```cpp
✅ 완료 목표:
- SPanel 베이스 클래스
- SVerticalBox
- SHorizontalBox
- SButton
- STextBlock

📁 파일 생성:
Mundi/Source/Slate/Panels/
├── SPanel.h
├── SPanel.cpp
├── SVerticalBox.h
├── SVerticalBox.cpp
├── SHorizontalBox.h
├── SHorizontalBox.cpp
├── SButton.h
├── SButton.cpp
├── STextBlock.h
└── STextBlock.cpp

🎯 테스트:
간단한 툴바 UI 만들어보기
```

### Phase 2: 기본 위젯 (1주)

```cpp
✅ 완료 목표:
- SEditableText
- SCheckBox
- SScrollBox
- SGridPanel
- SComboBox

🎯 테스트:
프로퍼티 편집 패널 만들어보기
```

### Phase 3: 고급 위젯 (2주)

```cpp
✅ 완료 목표:
- STreeView
- SListView
- SColorPicker
- SSplitter 통합

🎯 테스트:
파일 브라우저 UI 만들어보기
```

### Phase 4: Cascade 에디터 (2주)

```cpp
✅ 완료 목표:
- SParticleEmitterPanel
- SParticleDetailsPanel
- SParticleViewportPanel
- SParticleCurveEditorPanel
- 메인 에디터 윈도우

🎯 테스트:
실제 파티클 시스템 편집
```

### Phase 5: 최적화 & 확장 (1주)

```cpp
✅ 완료 목표:
- Invalidation 시스템
- Attribute 시스템
- 스타일 시스템
- Undo/Redo

🎯 테스트:
성능 측정 및 최적화
```

---

## 8. 참고 자료

### 8.1 언리얼 Slate 문서

- [Slate UI Framework](https://docs.unrealengine.com/5.0/en-US/slate-ui-framework-in-unreal-engine/)
- [Slate Overview](https://docs.unrealengine.com/5.0/en-US/slate-overview/)

### 8.2 ImGui 문서

- [Dear ImGui](https://github.com/ocornut/imgui)
- [ImGui Demo](https://github.com/ocornut/imgui/blob/master/imgui_demo.cpp)

### 8.3 Mundi 기존 코드

```cpp
// 참고할 기존 구현:
✅ SSplitter.h/cpp - 스플리터 구현
✅ SWindow.h - 윈도우 베이스
✅ BlendSpace2DEditorWindow.h - 복잡한 에디터 예시
✅ USlateManager.h - 레이아웃 관리
```

---

## 9. 마무리

### 9.1 핵심 요약

1. **Slate는 컴포넌트 기반 UI 시스템**
   - 재사용 가능한 위젯 조합
   - 계층 구조로 복잡한 UI 구성

2. **Mundi는 Slate의 모든 개념을 구현 가능**
   - ImGui 기반이지만 Slate 아키텍처 적용
   - 이미 SSplitter, SWindow 등 구현됨

3. **단계적 접근이 중요**
   - 기초부터 차근차근 구축
   - 각 단계마다 테스트

4. **실전 활용**
   - Cascade 파티클 에디터
   - Material Editor
   - Animation Timeline
   - 모든 게임 에디터 UI 가능

### 9.2 다음 단계

1. Phase 1 시작: SPanel, SVerticalBox 구현
2. 간단한 테스트 UI 만들기
3. 점진적으로 위젯 추가
4. Cascade 에디터 완성!

---

**작성자**: Claude
**마지막 업데이트**: 2025-01-21
