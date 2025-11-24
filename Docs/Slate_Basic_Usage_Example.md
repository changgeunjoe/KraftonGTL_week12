# Slate 기본 위젯 사용 예제

## 구현 완료된 위젯들

### Phase 1 기초 인프라 (완료 ✅)

- **SPanel** - 베이스 클래스 (자식 관리, Invalidation)
- **SVerticalBox** - 수직 레이아웃
- **SHorizontalBox** - 수평 레이아웃
- **SButton** - 클릭 가능한 버튼
- **STextBlock** - 텍스트 표시

---

## 파일 구조

```
Mundi/Source/Slate/
├── Panels/
│   ├── SPanel.h / .cpp              ✅ 베이스 클래스
│   ├── SVerticalBox.h / .cpp        ✅ 수직 레이아웃
│   └── SHorizontalBox.h / .cpp      ✅ 수평 레이아웃
└── Widgets/
    ├── SButton.h / .cpp             ✅ 버튼 위젯
    └── STextBlock.h / .cpp          ✅ 텍스트 위젯
```

---

## 사용 예제

### 1. 기본 버튼 사용

```cpp
#include "Slate/Widgets/SButton.h"
#include "Slate/Widgets/STextBlock.h"

// 버튼 생성
SButton* SaveButton = new SButton();
SaveButton->SetText("Save");
SaveButton->SetSize(FVector2D(100, 30));
SaveButton->SetRect(10, 10, 110, 40);

// 클릭 이벤트 바인딩
SaveButton->OnClicked.Bind([]() {
    // 저장 로직
    SaveScene();
});

// 렌더링
SaveButton->OnRender();
```

---

### 2. 동적 텍스트 표시

```cpp
// 정적 텍스트
STextBlock* TitleText = new STextBlock("Particle Editor");
TitleText->SetColor(0xFFFFCC64);  // 주황색
TitleText->SetRect(10, 10, 200, 30);

// 동적 텍스트 (델리게이트)
STextBlock* ParticleCount = new STextBlock();
ParticleCount->SetText([]() -> FString {
    return "Particles: " + std::to_string(GetActiveParticleCount());
});
ParticleCount->SetRect(10, 50, 200, 70);

// 렌더링 (매 프레임 자동 갱신)
ParticleCount->OnRender();
```

---

### 3. 수직 레이아웃 (툴바 + 콘텐츠)

```cpp
#include "Slate/Panels/SVerticalBox.h"

// 수직 박스 생성
SVerticalBox* MainLayout = new SVerticalBox();
MainLayout->SetRect(0, 0, 800, 600);

// 슬롯 1: 툴바 (고정 높이 40px)
MainLayout->AddSlot()
    .FixedHeight(40.0f)
    .AttachWidget(CreateToolbar());

// 슬롯 2: 메인 콘텐츠 (남은 공간 채움)
MainLayout->AddSlot()
    .FillHeight(1.0f)
    .AttachWidget(CreateMainContent());

// 슬롯 3: 상태바 (자동 높이)
MainLayout->AddSlot()
    .AutoHeight()
    .SetPadding(5.0f)
    .AttachWidget(CreateStatusBar());

// 렌더링
MainLayout->OnRender();
```

---

### 4. 수평 레이아웃 (좌우 분할)

```cpp
#include "Slate/Panels/SHorizontalBox.h"

// 수평 박스 생성
SHorizontalBox* SplitLayout = new SHorizontalBox();
SplitLayout->SetRect(0, 0, 800, 600);

// 왼쪽: 30% 너비
SplitLayout->AddSlot()
    .FillWidth(0.3f)
    .AttachWidget(CreateLeftPanel());

// 오른쪽: 70% 너비
SplitLayout->AddSlot()
    .FillWidth(0.7f)
    .AttachWidget(CreateRightPanel());

// 렌더링
SplitLayout->OnRender();
```

---

### 5. 복잡한 레이아웃 조합 (툴바 예제)

```cpp
SHorizontalBox* CreateToolbar()
{
    auto Toolbar = new SHorizontalBox();
    Toolbar->SetRect(0, 0, 1280, 50);

    // 좌측: 버튼 그룹
    auto ButtonGroup = new SHorizontalBox();

    auto NewButton = new SButton();
    NewButton->SetText("New");
    NewButton->SetSize(FVector2D(60, 30));
    NewButton->OnClicked.Bind([]() { NewScene(); });

    auto SaveButton = new SButton();
    SaveButton->SetText("Save");
    SaveButton->SetSize(FVector2D(60, 30));
    SaveButton->OnClicked.Bind([]() { SaveScene(); });

    ButtonGroup->AddSlot()
        .AutoWidth()
        .SetPadding(5, 0)
        .AttachWidget(NewButton);

    ButtonGroup->AddSlot()
        .AutoWidth()
        .SetPadding(5, 0)
        .AttachWidget(SaveButton);

    Toolbar->AddSlot()
        .AutoWidth()
        .SetPadding(10, 0)
        .VAlignment(SHorizontalBox::VAlign_Center)
        .AttachWidget(ButtonGroup);

    // 중앙: 빈 공간 (Spacer)
    Toolbar->AddSlot()
        .FillWidth(1.0f)
        .AttachWidget(new SPanel());  // 빈 패널

    // 우측: 상태 텍스트
    auto StatusText = new STextBlock();
    StatusText->SetText([]() -> FString {
        return "Ready";
    });

    Toolbar->AddSlot()
        .AutoWidth()
        .SetPadding(10, 0)
        .VAlignment(SHorizontalBox::VAlign_Center)
        .AttachWidget(StatusText);

    return Toolbar;
}
```

---

### 6. 전체 윈도우 예제 (에디터 레이아웃)

```cpp
class SMyEditorWindow : public SWindow
{
public:
    void Initialize()
    {
        SetRect(0, 0, 1280, 720);

        // 루트 레이아웃
        RootLayout = new SVerticalBox();
        RootLayout->SetRect(GetRect());

        // 툴바 (50px)
        RootLayout->AddSlot()
            .FixedHeight(50.0f)
            .AttachWidget(CreateToolbar());

        // 메인 영역 (남은 공간)
        auto MainArea = new SHorizontalBox();

        // 좌측 패널 (30%)
        MainArea->AddSlot()
            .FillWidth(0.3f)
            .AttachWidget(CreateLeftPanel());

        // 우측 패널 (70%)
        MainArea->AddSlot()
            .FillWidth(0.7f)
            .AttachWidget(CreateRightPanel());

        RootLayout->AddSlot()
            .FillHeight(1.0f)
            .AttachWidget(MainArea);

        // 상태바 (25px)
        RootLayout->AddSlot()
            .FixedHeight(25.0f)
            .AttachWidget(CreateStatusBar());
    }

    virtual void OnRender() override
    {
        if (RootLayout)
        {
            RootLayout->OnRender();
        }
    }

private:
    SVerticalBox* RootLayout = nullptr;

    SPanel* CreateToolbar()
    {
        auto Toolbar = new SHorizontalBox();

        auto Title = new STextBlock("My Editor");
        Title->SetColor(0xFFFFCC64);

        Toolbar->AddSlot()
            .AutoWidth()
            .SetPadding(10, 0)
            .AttachWidget(Title);

        return Toolbar;
    }

    SPanel* CreateLeftPanel()
    {
        auto Panel = new SVerticalBox();

        auto Header = new STextBlock("Scene Hierarchy");
        Panel->AddSlot()
            .AutoHeight()
            .SetPadding(5)
            .AttachWidget(Header);

        // 여기에 트리뷰 추가 예정

        return Panel;
    }

    SPanel* CreateRightPanel()
    {
        auto Panel = new SVerticalBox();

        auto Header = new STextBlock("Viewport");
        Panel->AddSlot()
            .AutoHeight()
            .SetPadding(5)
            .AttachWidget(Header);

        // 여기에 3D 뷰포트 추가 예정

        return Panel;
    }

    SPanel* CreateStatusBar()
    {
        auto StatusBar = new SHorizontalBox();

        auto StatusText = new STextBlock();
        StatusText->SetText([]() -> FString {
            return "Ready | FPS: " + std::to_string(GetFPS());
        });

        StatusBar->AddSlot()
            .FillWidth(1.0f)
            .SetPadding(5, 0)
            .AttachWidget(StatusText);

        return StatusBar;
    }
};
```

---

## 주요 기능

### Invalidation (선택적 렌더링)

변경이 없을 때 렌더링을 스킵하여 성능 향상:

```cpp
SButton* Button = new SButton();
Button->SetText("Click Me");

// 텍스트 변경 시 자동으로 Invalidate 호출됨
Button->SetText("Clicked!");  // 다음 프레임에 다시 그려짐

// 수동으로 Invalidate 호출
Button->Invalidate();
```

### 가시성 제어

```cpp
SButton* Button = new SButton();

// 숨기기
Button->SetVisible(false);

// 보이기
Button->SetVisible(true);

// 확인
if (Button->IsVisible())
{
    // ...
}
```

### 활성화/비활성화

```cpp
SButton* Button = new SButton();

// 비활성화 (클릭 불가)
Button->SetEnabled(false);

// 활성화
Button->SetEnabled(true);
```

---

## 다음 단계 (Phase 2)

추가로 구현할 위젯들:

- **SEditableText** - 텍스트 입력 필드
- **SCheckBox** - 체크박스
- **SComboBox** - 드롭다운 메뉴
- **SScrollBox** - 스크롤 가능한 컨테이너
- **SGridPanel** - 그리드 레이아웃
- **STreeView** - 트리 구조 뷰
- **SImage** - 이미지 표시

---

## 성능 최적화 팁

1. **Invalidation 활용**: 변경이 없는 UI는 자동으로 렌더링 스킵
2. **계층 구조**: 부모-자식 관계로 효율적인 업데이트
3. **델리게이트**: 동적 데이터 바인딩으로 수동 갱신 불필요

---

## 문제 해결

### Q: 위젯이 화면에 안 보여요
A: `SetRect()`로 위치와 크기를 먼저 설정하세요.

### Q: 버튼 클릭이 안 돼요
A: `OnMouseDown`, `OnMouseUp`, `OnMouseMove` 이벤트가 전달되는지 확인하세요.

### Q: 레이아웃이 제대로 안 잡혀요
A: `ArrangeChildren()`이 호출되고 있는지, 슬롯의 SizeRule이 올바른지 확인하세요.

---

**작성일**: 2025-01-21
**버전**: Phase 1 완료
