# Core와 UObject Slate 혼합 사용 가이드

## 개요
Core (SWindow 기반)와 UObject (UWidget/UUIWindow 기반)를 함께 사용하는 방법을 설명합니다.

---

## 사용 패턴

### 패턴 1: UUIWindow 안에 Core Slate 위젯 포함

**가장 일반적인 패턴** - UObject 윈도우 관리 + Core Slate 레이아웃

```cpp
// MyEditorWindow.h
#include "Slate/UObject/Windows/UIWindow.h"
#include "Slate/Core/Panels/SVerticalBox.h"
#include "Slate/Core/Panels/SHorizontalBox.h"
#include "Slate/Core/Widgets/SButton.h"
#include "Slate/Core/Widgets/STextBlock.h"

class UMyEditorWindow : public UUIWindow
{
public:
    DECLARE_CLASS(UMyEditorWindow, UUIWindow)

    UMyEditorWindow();
    virtual ~UMyEditorWindow();

    void Initialize() override;
    void RenderWindow() override;

private:
    // Core Slate 위젯들
    SVerticalBox* RootLayout = nullptr;
    SHorizontalBox* Toolbar = nullptr;
    SButton* SaveButton = nullptr;
    STextBlock* StatusText = nullptr;

    void CreateLayout();
};
```

```cpp
// MyEditorWindow.cpp
#include "MyEditorWindow.h"

IMPLEMENT_CLASS(UMyEditorWindow)

UMyEditorWindow::UMyEditorWindow()
{
    // UUIWindow 설정
    FUIWindowConfig Config;
    Config.WindowTitle = "My Editor";
    Config.DefaultSize = ImVec2(1280, 720);
    SetConfig(Config);
}

UMyEditorWindow::~UMyEditorWindow()
{
    // Core Slate 위젯들 정리
    if (RootLayout)
    {
        delete RootLayout;
        RootLayout = nullptr;
    }
}

void UMyEditorWindow::Initialize()
{
    UUIWindow::Initialize();
    CreateLayout();
}

void UMyEditorWindow::CreateLayout()
{
    // 루트 레이아웃 생성
    RootLayout = new SVerticalBox();

    // 툴바 (고정 높이 50px)
    Toolbar = new SHorizontalBox();

    SaveButton = new SButton();
    SaveButton->SetText("Save");
    SaveButton->SetSize(FVector2D(80, 30));
    SaveButton->OnClicked.Bind([this]() {
        // 저장 로직
        LOG_INFO("Save clicked!");
    });

    StatusText = new STextBlock("Ready");
    StatusText->SetColor(0xFF00FF00); // 초록색

    // 툴바 구성
    Toolbar->AddSlot()
        .AutoWidth()
        .SetPadding(5, 10)
        .AttachWidget(SaveButton);

    Toolbar->AddSlot()
        .FillWidth(1.0f)
        .AttachWidget(new SPanel()); // Spacer

    Toolbar->AddSlot()
        .AutoWidth()
        .SetPadding(5, 10)
        .AttachWidget(StatusText);

    // 루트 레이아웃에 추가
    RootLayout->AddSlot()
        .FixedHeight(50.0f)
        .AttachWidget(Toolbar);

    // 메인 콘텐츠 (남은 공간)
    RootLayout->AddSlot()
        .FillHeight(1.0f)
        .AttachWidget(new SPanel()); // 나중에 콘텐츠 추가
}

void UMyEditorWindow::RenderWindow()
{
    // UUIWindow의 기본 렌더링 (ImGui 윈도우 시작)
    UUIWindow::RenderWindow();

    if (!IsVisible())
        return;

    // Core Slate 레이아웃 렌더링
    if (RootLayout)
    {
        // 윈도우 크기에 맞춰 레이아웃 크기 설정
        ImVec2 WindowSize = ImGui::GetWindowSize();
        ImVec2 WindowPos = ImGui::GetWindowPos();

        RootLayout->SetRect(
            WindowPos.x,
            WindowPos.y,
            WindowPos.x + WindowSize.x,
            WindowPos.y + WindowSize.y
        );

        RootLayout->OnRender();
    }
}
```

---

## 패턴 2: Core SWindow에 UWidget 포함

Core 윈도우 안에서 기존 UWidget 사용

```cpp
// SMyEditorWindow.h
#include "Slate/Core/Windows/SWindow.h"
#include "Slate/Core/Panels/SVerticalBox.h"
#include "Slate/UObject/Widgets/PropertyRenderer.h"

class SMyEditorWindow : public SWindow
{
public:
    SMyEditorWindow();
    virtual ~SMyEditorWindow();

    void Initialize();
    virtual void OnRender() override;

private:
    SVerticalBox* Layout = nullptr;
    UPropertyRenderer* PropertyPanel = nullptr; // UObject 위젯
};
```

```cpp
// SMyEditorWindow.cpp
SMyEditorWindow::SMyEditorWindow()
{
}

SMyEditorWindow::~SMyEditorWindow()
{
    if (Layout)
        delete Layout;
}

void SMyEditorWindow::Initialize()
{
    Layout = new SVerticalBox();
    Layout->SetRect(Rect);

    // UObject 위젯 생성
    PropertyPanel = NewObject<UPropertyRenderer>();
    PropertyPanel->Initialize();

    // 레이아웃 구성
    Layout->AddSlot()
        .FillHeight(1.0f)
        .AttachWidget(new SPanel()); // PropertyPanel을 렌더링할 공간
}

void SMyEditorWindow::OnRender()
{
    if (Layout)
    {
        Layout->SetRect(Rect);
        Layout->OnRender();
    }

    // UWidget 수동 렌더링
    if (PropertyPanel)
    {
        PropertyPanel->Update();
        PropertyPanel->RenderWidget();
    }
}
```

---

## 패턴 3: UWidget을 Core 레이아웃에 래핑

UWidget을 SPanel로 감싸서 레이아웃에 통합

```cpp
// SWidgetWrapper.h
#include "Slate/Core/Panels/SPanel.h"
#include "Slate/UObject/Widgets/Widget.h"

/**
 * UWidget을 Core Slate 레이아웃에서 사용할 수 있게 래핑하는 클래스
 */
class SWidgetWrapper : public SPanel
{
public:
    SWidgetWrapper(UWidget* InWidget)
        : WrappedWidget(InWidget)
    {
        if (WrappedWidget)
        {
            WrappedWidget->Initialize();
        }
    }

    virtual ~SWidgetWrapper()
    {
        // UWidget은 UObject이므로 GC가 관리 (delete 안 함)
        WrappedWidget = nullptr;
    }

    void RenderContent() override
    {
        if (WrappedWidget)
        {
            WrappedWidget->Update();
            WrappedWidget->RenderWidget();
        }
    }

private:
    UWidget* WrappedWidget = nullptr;
};
```

**사용 예시:**

```cpp
// Core 레이아웃에 UWidget 추가
SVerticalBox* Layout = new SVerticalBox();

// 기존 UWidget 래핑
UMainToolbarWidget* Toolbar = NewObject<UMainToolbarWidget>();
SWidgetWrapper* ToolbarWrapper = new SWidgetWrapper(Toolbar);

Layout->AddSlot()
    .FixedHeight(50.0f)
    .AttachWidget(ToolbarWrapper);
```

---

## 패턴 4: 하이브리드 윈도우 (가장 유연함)

UUIWindow + Core Slate + UWidget 모두 사용

```cpp
// UHybridEditorWindow.h
#include "Slate/UObject/Windows/UIWindow.h"
#include "Slate/Core/Panels/SVerticalBox.h"
#include "Slate/Core/Panels/SHorizontalBox.h"
#include "Slate/Core/Widgets/SButton.h"
#include "Slate/UObject/Widgets/SceneManagerWidget.h"

class UHybridEditorWindow : public UUIWindow
{
public:
    DECLARE_CLASS(UHybridEditorWindow, UUIWindow)

    void Initialize() override;
    void RenderWindow() override;

private:
    // Core Slate 레이아웃
    SVerticalBox* RootLayout = nullptr;
    SHorizontalBox* MainArea = nullptr;

    // Core Slate 위젯
    SButton* SaveButton = nullptr;

    // UObject 위젯
    USceneManagerWidget* SceneManager = nullptr;

    void CreateLayout();
};
```

```cpp
// UHybridEditorWindow.cpp
void UHybridEditorWindow::Initialize()
{
    UUIWindow::Initialize();
    CreateLayout();

    // UObject 위젯 생성
    SceneManager = NewObject<USceneManagerWidget>();
    SceneManager->Initialize();
}

void UHybridEditorWindow::CreateLayout()
{
    RootLayout = new SVerticalBox();
    MainArea = new SHorizontalBox();

    // 툴바 (Core Slate)
    auto Toolbar = new SHorizontalBox();

    SaveButton = new SButton();
    SaveButton->SetText("Save");
    SaveButton->OnClicked.Bind([this]() {
        LOG_INFO("Save");
    });

    Toolbar->AddSlot()
        .AutoWidth()
        .SetPadding(5)
        .AttachWidget(SaveButton);

    RootLayout->AddSlot()
        .FixedHeight(50.0f)
        .AttachWidget(Toolbar);

    // 메인 영역
    // 좌측: UWidget (SceneManager) - 30%
    // 우측: Core Slate 콘텐츠 - 70%

    MainArea->AddSlot()
        .FillWidth(0.3f)
        .AttachWidget(new SPanel()); // SceneManager 렌더링 공간

    MainArea->AddSlot()
        .FillWidth(0.7f)
        .AttachWidget(new SPanel()); // 메인 콘텐츠

    RootLayout->AddSlot()
        .FillHeight(1.0f)
        .AttachWidget(MainArea);
}

void UHybridEditorWindow::RenderWindow()
{
    UUIWindow::RenderWindow();

    if (!IsVisible())
        return;

    // 윈도우 크기 가져오기
    ImVec2 WindowSize = ImGui::GetWindowSize();
    ImVec2 WindowPos = ImGui::GetWindowPos();

    // Core Slate 레이아웃 렌더링
    if (RootLayout)
    {
        RootLayout->SetRect(
            WindowPos.x, WindowPos.y,
            WindowPos.x + WindowSize.x,
            WindowPos.y + WindowSize.y
        );
        RootLayout->OnRender();
    }

    // UWidget 수동 렌더링 (좌측 30% 영역)
    if (SceneManager)
    {
        // 좌측 영역 계산 (MainArea의 첫 번째 슬롯)
        float LeftWidth = WindowSize.x * 0.3f;

        // ImGui 커서 위치 설정
        ImGui::SetCursorScreenPos(ImVec2(
            WindowPos.x,
            WindowPos.y + 50.0f  // 툴바 높이만큼 내림
        ));

        ImGui::BeginChild("SceneManager", ImVec2(LeftWidth, WindowSize.y - 50.0f), true);
        SceneManager->Update();
        SceneManager->RenderWidget();
        ImGui::EndChild();
    }
}
```

---

## 실전 예제: Particle Editor

```cpp
// UParticleEditorWindow.h
class UParticleEditorWindow : public UUIWindow
{
public:
    DECLARE_CLASS(UParticleEditorWindow, UUIWindow)

    void Initialize() override;
    void RenderWindow() override;

private:
    // Core Slate (레이아웃 + 간단한 위젯)
    SVerticalBox* RootLayout = nullptr;
    SHorizontalBox* Toolbar = nullptr;
    SHorizontalBox* MainArea = nullptr;
    SButton* PlayButton = nullptr;
    SButton* PauseButton = nullptr;
    STextBlock* ParticleCount = nullptr;

    // UObject (복잡한 기능)
    UPropertyRenderer* DetailsPanel = nullptr;

    void CreateLayout();
};
```

```cpp
void UParticleEditorWindow::CreateLayout()
{
    RootLayout = new SVerticalBox();

    // ===== 툴바 (Core Slate) =====
    Toolbar = new SHorizontalBox();

    PlayButton = new SButton();
    PlayButton->SetText("Play");
    PlayButton->OnClicked.Bind([this]() {
        PlayParticlePreview();
    });

    PauseButton = new SButton();
    PauseButton->SetText("Pause");

    ParticleCount = new STextBlock();
    ParticleCount->SetText([this]() -> FString {
        return "Particles: " + std::to_string(GetActiveParticleCount());
    });

    Toolbar->AddSlot().AutoWidth().SetPadding(5).AttachWidget(PlayButton);
    Toolbar->AddSlot().AutoWidth().SetPadding(5).AttachWidget(PauseButton);
    Toolbar->AddSlot().FillWidth(1.0f).AttachWidget(new SPanel()); // Spacer
    Toolbar->AddSlot().AutoWidth().SetPadding(5).AttachWidget(ParticleCount);

    RootLayout->AddSlot()
        .FixedHeight(50.0f)
        .AttachWidget(Toolbar);

    // ===== 메인 영역 =====
    MainArea = new SHorizontalBox();

    // 좌측: Emitter 리스트 (20%)
    MainArea->AddSlot()
        .FillWidth(0.2f)
        .AttachWidget(new SPanel());

    // 중앙: 3D 뷰포트 (50%)
    MainArea->AddSlot()
        .FillWidth(0.5f)
        .AttachWidget(new SPanel());

    // 우측: 프로퍼티 (30%) - UPropertyRenderer 사용
    MainArea->AddSlot()
        .FillWidth(0.3f)
        .AttachWidget(new SPanel());

    RootLayout->AddSlot()
        .FillHeight(1.0f)
        .AttachWidget(MainArea);

    // ===== UObject 위젯 생성 =====
    DetailsPanel = NewObject<UPropertyRenderer>();
    DetailsPanel->Initialize();
}

void UParticleEditorWindow::RenderWindow()
{
    UUIWindow::RenderWindow();
    if (!IsVisible()) return;

    ImVec2 WindowSize = ImGui::GetWindowSize();
    ImVec2 WindowPos = ImGui::GetWindowPos();

    // Core Slate 레이아웃
    if (RootLayout)
    {
        RootLayout->SetRect(
            WindowPos.x, WindowPos.y,
            WindowPos.x + WindowSize.x,
            WindowPos.y + WindowSize.y
        );
        RootLayout->OnRender();
    }

    // UPropertyRenderer 렌더링 (우측 30% 영역)
    if (DetailsPanel)
    {
        float RightX = WindowPos.x + WindowSize.x * 0.7f;
        float RightWidth = WindowSize.x * 0.3f;

        ImGui::SetCursorScreenPos(ImVec2(RightX, WindowPos.y + 50.0f));
        ImGui::BeginChild("Details", ImVec2(RightWidth, WindowSize.y - 50.0f), true);

        DetailsPanel->Update();
        DetailsPanel->RenderWidget();

        ImGui::EndChild();
    }
}
```

---

## 장단점 비교

| 패턴 | 장점 | 단점 | 추천 상황 |
|------|------|------|----------|
| **패턴 1: UUIWindow + Core** | • 윈도우 관리 간편<br>• 레이아웃 자동화 | • Core 위젯 수명 관리 필요 | ✅ 대부분의 경우 (추천) |
| **패턴 2: Core + UWidget** | • 성능 우수<br>• 레이아웃 완전 제어 | • 윈도우 관리 수동 | 고성능 에디터 |
| **패턴 3: Wrapper** | • 기존 UWidget 재사용 쉬움 | • 래핑 오버헤드 | 점진적 전환 |
| **패턴 4: Hybrid** | • 최대 유연성 | • 복잡도 높음 | 복잡한 에디터 |

---

## 추천 사용 전략

### 🎯 새 에디터 만들 때

```
UUIWindow (윈도우 관리)
└── Core Slate (레이아웃 + 간단한 위젯)
    └── UPropertyRenderer (복잡한 프로퍼티 편집)
```

**예시**: Particle Editor, Material Editor

### 🔧 기존 에디터 개선할 때

```
기존 UUIWindow 유지
└── 복잡한 레이아웃 부분만 Core Slate로 교체
    └── 기존 UWidget들은 Wrapper로 감싸서 재사용
```

**예시**: BlendSpace2D 개선

### ⚡ 고성능 에디터

```
Core SWindow (순수 C++)
└── Core Slate (모든 UI)
    └── 필요시에만 UWidget 수동 렌더링
```

**예시**: Animation Timeline, Node Graph Editor

---

## 주의사항

### 1. **메모리 관리**

```cpp
// ❌ 잘못된 방법
UMyEditorWindow::~UMyEditorWindow()
{
    // UWidget은 UObject이므로 delete 하면 안 됨!
    delete MyUWidget; // 크래시!
}

// ✅ 올바른 방법
UMyEditorWindow::~UMyEditorWindow()
{
    // Core Slate만 delete
    if (RootLayout)
        delete RootLayout;

    // UWidget은 null만 설정 (GC가 관리)
    MyUWidget = nullptr;
}
```

### 2. **렌더링 순서**

```cpp
void RenderWindow() override
{
    // 1. UUIWindow 렌더링 (ImGui 윈도우 시작)
    UUIWindow::RenderWindow();

    // 2. Core Slate 렌더링
    if (Layout)
        Layout->OnRender();

    // 3. UWidget 수동 렌더링 (필요 시)
    if (MyUWidget)
        MyUWidget->RenderWidget();

    // 4. ImGui::End()는 UUIWindow에서 자동 호출
}
```

### 3. **좌표계 동기화**

```cpp
// ImGui 윈도우 크기를 Core Slate에 전달
ImVec2 WindowSize = ImGui::GetWindowSize();
ImVec2 WindowPos = ImGui::GetWindowPos();

Layout->SetRect(
    WindowPos.x, WindowPos.y,
    WindowPos.x + WindowSize.x,
    WindowPos.y + WindowSize.y
);
```

---

**작성일**: 2025-01-21
**버전**: 혼합 사용 가이드 v1.0
