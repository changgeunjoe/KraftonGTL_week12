# Slate UI 사용 가이드

## 목차
1. [기본 개념](#기본-개념)
2. [레이아웃 패널](#레이아웃-패널)
3. [크기 조절 (Size Rules)](#크기-조절-size-rules)
4. [정렬 (Alignment)](#정렬-alignment)
5. [여백 (Padding)](#여백-padding)
6. [위젯 종류](#위젯-종류)
7. [실전 예제](#실전-예제)

---

## 기본 개념

### 위젯 계층 구조
```
SPanel (컨테이너)
├─ SVerticalBox (수직 배치)
├─ SHorizontalBox (수평 배치)
├─ SGridPanel (그리드 배치)
└─ SScrollBox (스크롤 가능)

SCompoundWidget (리프 위젯)
├─ SButton
├─ STextBlock
├─ SEditableText
├─ SCheckBox
├─ STreeView
└─ SListView
```

### 기본 사용 패턴
```cpp
// 1. 루트 레이아웃 생성
RootLayout = new SVerticalBox();

// 2. 자식 위젯 추가
RootLayout->AddSlot()
    .AutoHeight()           // 크기 규칙
    .SetPadding(10.0f)      // 여백
    .AttachWidget(MyWidget); // 위젯 연결
```

---

## 레이아웃 패널

### SVerticalBox (수직 배치)
위젯을 **위에서 아래로** 배치합니다.

```cpp
auto VBox = new SVerticalBox();

VBox->AddSlot()
    .AutoHeight()
    .AttachWidget(TitleText);

VBox->AddSlot()
    .FillHeight(1.0f)
    .AttachWidget(ContentPanel);

VBox->AddSlot()
    .FixedHeight(50.0f)
    .AttachWidget(ButtonRow);
```

**결과:**
```
┌─────────────────┐
│ Title (Auto)    │ ← 텍스트 크기만큼
├─────────────────┤
│                 │
│ Content (Fill)  │ ← 남은 공간 전부
│                 │
├─────────────────┤
│ Buttons (50px)  │ ← 고정 50px
└─────────────────┘
```

### SHorizontalBox (수평 배치)
위젯을 **왼쪽에서 오른쪽으로** 배치합니다.

```cpp
auto HBox = new SHorizontalBox();

HBox->AddSlot()
    .FixedWidth(200.0f)
    .AttachWidget(LeftPanel);

HBox->AddSlot()
    .FillWidth(1.0f)
    .AttachWidget(CenterPanel);

HBox->AddSlot()
    .FixedWidth(300.0f)
    .AttachWidget(RightPanel);
```

**결과:**
```
┌──────┬────────────────┬─────────┐
│ Left │    Center      │  Right  │
│200px │    (Fill)      │  300px  │
└──────┴────────────────┴─────────┘
```

---

## 크기 조절 (Size Rules)

### 1. AutoHeight / AutoWidth
**위젯의 실제 크기만큼** 공간을 차지합니다.

```cpp
.AutoHeight()    // 수직: 위젯 높이만큼
.AutoWidth()     // 수평: 위젯 너비만큼
```

**사용 예시:**
- STextBlock: 텍스트 크기만큼
- SButton: SetSize()로 설정한 크기만큼
- STreeView: 기본 크기(150px) 또는 Rect 크기

**주의:** GetHeight()/GetWidth()가 0을 반환하는 위젯은 공간을 차지하지 않습니다!

### 2. FillHeight / FillWidth
**남은 공간을 비율로 나눠** 차지합니다.

```cpp
.FillHeight(1.0f)    // 남은 공간의 100%
.FillWidth(2.0f)     // 비율이 2
```

**비율 계산 예시:**
```cpp
// 예시 1: 1:1 비율
VBox->AddSlot().FillHeight(1.0f).AttachWidget(Widget1);  // 50%
VBox->AddSlot().FillHeight(1.0f).AttachWidget(Widget2);  // 50%

// 예시 2: 2:1 비율
VBox->AddSlot().FillHeight(2.0f).AttachWidget(Widget1);  // 66.6%
VBox->AddSlot().FillHeight(1.0f).AttachWidget(Widget2);  // 33.3%

// 예시 3: 3:2:1 비율
VBox->AddSlot().FillHeight(3.0f).AttachWidget(Widget1);  // 50%
VBox->AddSlot().FillHeight(2.0f).AttachWidget(Widget2);  // 33.3%
VBox->AddSlot().FillHeight(1.0f).AttachWidget(Widget3);  // 16.6%
```

**계산 공식:**
```
Ratio = SizeValue / TotalFillWeight
할당 크기 = RemainingSpace × Ratio

예: FillHeight(2.0) + FillHeight(1.0) = 총 3.0
   - Widget1 = 남은공간 × (2.0/3.0) = 66.6%
   - Widget2 = 남은공간 × (1.0/3.0) = 33.3%
```

### 3. FixedHeight / FixedWidth
**고정된 픽셀 크기**를 차지합니다.

```cpp
.FixedHeight(50.0f)    // 50픽셀 고정
.FixedWidth(200.0f)    // 200픽셀 고정
```

**사용 예시:**
- 툴바: `.FixedHeight(50.0f)`
- 사이드바: `.FixedWidth(250.0f)`
- 버튼 영역: `.FixedHeight(40.0f)`

### 크기 규칙 조합 예시

```cpp
auto Layout = new SVerticalBox();

// 고정 크기 헤더
Layout->AddSlot()
    .FixedHeight(60.0f)
    .AttachWidget(Header);

// 자동 크기 타이틀
Layout->AddSlot()
    .AutoHeight()
    .AttachWidget(Title);

// 확장 가능한 콘텐츠 (남은 공간의 70%)
Layout->AddSlot()
    .FillHeight(0.7f)
    .AttachWidget(MainContent);

// 확장 가능한 푸터 (남은 공간의 30%)
Layout->AddSlot()
    .FillHeight(0.3f)
    .AttachWidget(Footer);

// 고정 크기 버튼
Layout->AddSlot()
    .FixedHeight(40.0f)
    .AttachWidget(ButtonRow);
```

**결과:**
```
총 높이 800px 가정:
┌─────────────────┐
│ Header  (60px)  │ ← 고정
├─────────────────┤
│ Title   (20px)  │ ← 자동 (텍스트 크기)
├─────────────────┤
│                 │
│ Main    (504px) │ ← Fill 70% (800-60-20-40) × 0.7
│                 │
├─────────────────┤
│ Footer  (216px) │ ← Fill 30% (800-60-20-40) × 0.3
├─────────────────┤
│ Buttons (40px)  │ ← 고정
└─────────────────┘
```

---

## 정렬 (Alignment)

### VAlignment (수직 정렬)
슬롯 **내부**에서 위젯을 수직으로 어떻게 배치할지 결정합니다.

```cpp
.VAlignment(VAlign_Top)     // 위쪽 정렬
.VAlignment(VAlign_Center)  // 중간 정렬
.VAlignment(VAlign_Bottom)  // 아래쪽 정렬
.VAlignment(VAlign_Fill)    // 전체 높이 채움 (기본값)
```

**시각적 예시 (SHorizontalBox 내부):**
```
VAlign_Top:
┌─────────┐
│ Widget  │ ← 위쪽에 위젯
│         │
│         │
└─────────┘

VAlign_Center:
┌─────────┐
│         │
│ Widget  │ ← 중간에 위젯
│         │
└─────────┘

VAlign_Fill:
┌─────────┐
│ Widget  │
│ Widget  │ ← 전체 높이 채움
│ Widget  │
└─────────┘
```

### HAlignment (수평 정렬)
슬롯 **내부**에서 위젯을 수평으로 어떻게 배치할지 결정합니다.

```cpp
.HAlignment(HAlign_Left)    // 왼쪽 정렬
.HAlignment(HAlign_Center)  // 중간 정렬
.HAlignment(HAlign_Right)   // 오른쪽 정렬
.HAlignment(HAlign_Fill)    // 전체 너비 채움
```

### 정렬 사용 예시

```cpp
// 중앙 정렬된 타이틀
TopBar->AddSlot()
    .FillWidth(1.0f)
    .HAlignment(HAlign_Center)
    .VAlignment(VAlign_Center)
    .AttachWidget(TitleText);

// 오른쪽 정렬된 버튼
TopBar->AddSlot()
    .AutoWidth()
    .HAlignment(HAlign_Right)
    .VAlignment(VAlign_Center)
    .AttachWidget(CloseButton);
```

### 기본값 주의사항

**SHorizontalBox 기본값:**
- HAlign = `HAlign_Left`
- VAlign = `VAlign_Fill` ✅

**SVerticalBox 기본값:**
- VAlign = `VAlign_Top`

**왜 VAlign_Fill이 기본값인가?**
- 컨테이너 패널(SVerticalBox, SHorizontalBox)이 **자식을 배치할 공간**을 제대로 받기 위함
- GetHeight()가 0인 패널도 전체 높이를 받아 내부 자식들을 배치 가능

---

## 여백 (Padding)

슬롯 내부의 **여백**을 설정합니다.

### 사용 방법

```cpp
// 1. 모든 방향 동일
.SetPadding(10.0f)  // 상하좌우 10px

// 2. 수평/수직 다르게
.SetPadding(20.0f, 10.0f)  // 좌우 20px, 상하 10px

// 3. 각 방향 개별 설정
.SetPadding(5.0f, 10.0f, 15.0f, 20.0f)  // 좌, 상, 우, 하
```

### 시각적 예시

```
SetPadding(10.0f):
┌─────────────────────┐
│ ← 10px →            │ ← 위 10px
│         ┌────────┐  │
│         │ Widget │  │
│         └────────┘  │
│                     │ ← 아래 10px
└─────────────────────┘
  ↑               ↑
 왼쪽 10px    오른쪽 10px
```

### Padding 실전 활용

```cpp
// 툴바 - 좌우 여백만
TopToolbar->AddSlot()
    .AutoHeight()
    .SetPadding(10.0f, 0.0f)  // 좌우 10px, 상하 0px
    .AttachWidget(TitleText);

// 버튼 - 균일한 여백
ButtonRow->AddSlot()
    .AutoWidth()
    .SetPadding(5.0f)  // 모든 방향 5px
    .AttachWidget(SaveButton);

// 패널 - 여백 없음 (딱 붙임)
MainContent->AddSlot()
    .FillWidth(1.0f)
    .SetPadding(0.0f)
    .AttachWidget(ViewportPanel);
```

---

## 위젯 종류

### 레이아웃 위젯

#### SVerticalBox
```cpp
auto VBox = new SVerticalBox();
VBox->AddSlot()
    .AutoHeight()
    .SetPadding(5.0f)
    .AttachWidget(MyWidget);
```

#### SHorizontalBox
```cpp
auto HBox = new SHorizontalBox();
HBox->AddSlot()
    .FillWidth(1.0f)
    .VAlignment(VAlign_Fill)
    .AttachWidget(MyPanel);
```

#### SScrollBox
스크롤 가능한 컨테이너
```cpp
auto ScrollBox = new SScrollBox();
ScrollBox->SetScrollbarVisibility(true, false); // 수직만
ScrollBox->AddChild(ContentWidget);
```

### 기본 위젯

#### STextBlock
텍스트 표시
```cpp
auto Text = new STextBlock("Hello World");
Text->SetFontSize(14.0f);
Text->SetColor(0xFFFFFFFF);
Text->SetJustification(STextBlock::Center);
```

#### SButton
클릭 가능한 버튼
```cpp
auto Button = new SButton();
Button->SetText("Click Me");
Button->SetSize(FVector2D(100, 30));
Button->OnClicked.Add([]() {
    UE_LOG("Button clicked!");
});
```

#### SEditableText
텍스트 입력
```cpp
auto Input = new SEditableText();
Input->SetText("Initial Value");
Input->SetHintText("Enter name...");
Input->OnTextChanged.Add([](const FString& NewText) {
    UE_LOG("Text changed: %s", NewText.c_str());
});
```

#### SCheckBox
체크박스
```cpp
auto CheckBox = new SCheckBox("Enable Feature", false);
CheckBox->OnCheckStateChanged.Add([](bool bChecked) {
    UE_LOG("Checked: %d", bChecked);
});
```

#### STreeView
트리 구조 뷰
```cpp
auto TreeView = new STreeView();
auto Root = TreeView->AddRootNode("Root");
Root->AddChild("Child 1");
Root->AddChild("Child 2");

TreeView->OnSelectionChanged.Add([](STreeNode* Node) {
    UE_LOG("Selected: %s", Node->GetLabel().c_str());
});
```

#### SListView
리스트 뷰
```cpp
auto ListView = new SListView();
ListView->AddItem("Item 1");
ListView->AddItem("Item 2");
ListView->AddItem("Item 3");

ListView->OnSelectionChanged.Add([](uint32 Index, const FString& Item) {
    UE_LOG("Selected: %s (Index: %d)", Item.c_str(), Index);
});
```

---

## 실전 예제

### 예제 1: 3패널 에디터 레이아웃

```cpp
void CreateEditorLayout()
{
    // 루트 레이아웃
    RootLayout = new SVerticalBox();

    // 상단 툴바 (고정 높이)
    TopToolbar = new SHorizontalBox();

    auto TitleText = new STextBlock("My Editor");
    TitleText->SetFontSize(18.0f);

    TopToolbar->AddSlot()
        .AutoWidth()
        .VAlignment(VAlign_Fill)
        .SetPadding(10.0f, 0.0f)
        .AttachWidget(TitleText);

    RootLayout->AddSlot()
        .FixedHeight(50.0f)
        .AttachWidget(TopToolbar);

    // 메인 콘텐츠 영역 (3분할)
    MainContentArea = new SHorizontalBox();

    // 왼쪽 패널 (고정 너비 250px)
    LeftPanel = new SVerticalBox();
    CreateLeftPanel();

    MainContentArea->AddSlot()
        .FixedWidth(250.0f)
        .AttachWidget(LeftPanel);

    // 중앙 패널 (확장)
    CenterPanel = new SVerticalBox();
    CreateCenterPanel();

    MainContentArea->AddSlot()
        .FillWidth(1.0f)
        .AttachWidget(CenterPanel);

    // 오른쪽 패널 (고정 너비 300px)
    RightPanel = new SVerticalBox();
    CreateRightPanel();

    MainContentArea->AddSlot()
        .FixedWidth(300.0f)
        .AttachWidget(RightPanel);

    RootLayout->AddSlot()
        .FillHeight(1.0f)
        .AttachWidget(MainContentArea);
}

void CreateLeftPanel()
{
    // 타이틀
    auto Title = new STextBlock("Items");
    Title->SetFontSize(14.0f);
    Title->SetColor(0xFFFFCC64);

    LeftPanel->AddSlot()
        .AutoHeight()
        .SetPadding(5.0f)
        .AttachWidget(Title);

    // 리스트뷰
    ItemListView = new SListView();
    ItemListView->AddItem("Item 1");
    ItemListView->AddItem("Item 2");

    LeftPanel->AddSlot()
        .FillHeight(1.0f)
        .SetPadding(5.0f)
        .AttachWidget(ItemListView);

    // 버튼 영역
    auto ButtonRow = new SHorizontalBox();

    auto AddButton = new SButton();
    AddButton->SetText("Add");
    AddButton->SetSize(FVector2D(100, 30));

    ButtonRow->AddSlot()
        .FillWidth(1.0f)
        .SetPadding(2.0f)
        .AttachWidget(AddButton);

    auto RemoveButton = new SButton();
    RemoveButton->SetText("Remove");
    RemoveButton->SetSize(FVector2D(100, 30));

    ButtonRow->AddSlot()
        .FillWidth(1.0f)
        .SetPadding(2.0f)
        .AttachWidget(RemoveButton);

    LeftPanel->AddSlot()
        .FixedHeight(40.0f)
        .SetPadding(5.0f)
        .AttachWidget(ButtonRow);
}
```

**결과:**
```
┌─────────────────────────────────────────────────────┐
│ My Editor                                     Ready │ ← 50px 툴바
├──────────┬────────────────────────┬─────────────────┤
│ Items    │                        │ Properties      │
│ ┌──────┐ │                        │ ┌─────────────┐ │
│ │Item 1│ │                        │ │ Name: ...   │ │
│ │Item 2│ │      Viewport          │ │ Size: ...   │ │
│ │      │ │                        │ │ Color: ...  │ │
│ │      │ │                        │ └─────────────┘ │
│ └──────┘ │                        │                 │
│ [Add][Rm]│                        │                 │
│          │                        │                 │
│  250px   │       (Fill)           │     300px       │
└──────────┴────────────────────────┴─────────────────┘
```

### 예제 2: 프로퍼티 에디터

```cpp
void CreatePropertyEditor()
{
    PropertyPanel = new SVerticalBox();

    // 섹션 타이틀
    auto SectionTitle = new STextBlock("Transform");
    SectionTitle->SetFontSize(12.0f);
    SectionTitle->SetColor(0xFFAAAAFF);

    PropertyPanel->AddSlot()
        .AutoHeight()
        .SetPadding(5.0f)
        .AttachWidget(SectionTitle);

    // 구분선
    auto Separator = new SSeparator(SSeparator::Horizontal);

    PropertyPanel->AddSlot()
        .AutoHeight()
        .SetPadding(5.0f, 2.0f)
        .AttachWidget(Separator);

    // Position X
    auto PositionXRow = new SHorizontalBox();

    auto LabelX = new STextBlock("Position X:");
    PositionXRow->AddSlot()
        .FixedWidth(80.0f)
        .VAlignment(VAlign_Fill)
        .AttachWidget(LabelX);

    auto InputX = new SEditableText("0.0");
    PositionXRow->AddSlot()
        .FillWidth(1.0f)
        .AttachWidget(InputX);

    PropertyPanel->AddSlot()
        .AutoHeight()
        .SetPadding(5.0f, 2.0f)
        .AttachWidget(PositionXRow);

    // Position Y
    auto PositionYRow = new SHorizontalBox();

    auto LabelY = new STextBlock("Position Y:");
    PositionYRow->AddSlot()
        .FixedWidth(80.0f)
        .VAlignment(VAlign_Fill)
        .AttachWidget(LabelY);

    auto InputY = new SEditableText("0.0");
    PositionYRow->AddSlot()
        .FillWidth(1.0f)
        .AttachWidget(InputY);

    PropertyPanel->AddSlot()
        .AutoHeight()
        .SetPadding(5.0f, 2.0f)
        .AttachWidget(PositionYRow);
}
```

**결과:**
```
┌─────────────────────┐
│ Transform           │
├─────────────────────┤
│ Position X: [  0.0 ]│
│ Position Y: [  0.0 ]│
└─────────────────────┘
```

### 예제 3: 비율 나누기

```cpp
// 2:1 비율로 나눈 레이아웃
void Create2to1Layout()
{
    auto VBox = new SVerticalBox();

    // 상단 2/3
    auto TopPanel = new SPanel();
    VBox->AddSlot()
        .FillHeight(2.0f)  // 66.6%
        .AttachWidget(TopPanel);

    // 하단 1/3
    auto BottomPanel = new SPanel();
    VBox->AddSlot()
        .FillHeight(1.0f)  // 33.3%
        .AttachWidget(BottomPanel);
}

// 1:2:1 비율로 나눈 레이아웃
void Create1to2to1Layout()
{
    auto HBox = new SHorizontalBox();

    // 왼쪽 1/4
    HBox->AddSlot()
        .FillWidth(1.0f)  // 25%
        .AttachWidget(LeftPanel);

    // 중앙 2/4
    HBox->AddSlot()
        .FillWidth(2.0f)  // 50%
        .AttachWidget(CenterPanel);

    // 오른쪽 1/4
    HBox->AddSlot()
        .FillWidth(1.0f)  // 25%
        .AttachWidget(RightPanel);
}
```

---

## 팁과 주의사항

### 1. GetHeight()/GetWidth() 구현
커스텀 위젯을 만들 때 **반드시 GetHeight()/GetWidth()를 오버라이드**하세요.

```cpp
// ❌ 나쁜 예: 오버라이드 없음
class MyWidget : public SCompoundWidget
{
    // GetHeight() 없음 → Rect.GetHeight() 반환 → ArrangeChildren 전에는 0
};

// ✅ 좋은 예: 오버라이드 있음
class MyWidget : public SCompoundWidget
{
public:
    virtual float GetHeight() const override
    {
        if (Rect.GetHeight() > 0.0f)
            return Rect.GetHeight();
        return DefaultHeight;  // 기본 높이 반환
    }

private:
    float DefaultHeight = 100.0f;
};
```

### 2. ImGui 렌더링 위치
ImGui 함수를 사용하는 위젯은 **커서 위치를 명시적으로 설정**하세요.

```cpp
void RenderContent() override
{
    // ✅ 올바른 방법: 커서 위치 설정
    ImGui::SetCursorScreenPos(ImVec2(Rect.Left, Rect.Top));

    ImGui::BeginChild("MyWidget", ImVec2(Width, Height), true);
    // ... 렌더링
    ImGui::EndChild();
}
```

### 3. VAlign 기본값 이해
- **SHorizontalBox**: VAlign 기본값 = `VAlign_Fill`
- **SVerticalBox**: VAlign 기본값 = `VAlign_Top`

컨테이너 패널은 대부분 `VAlign_Fill`이 적합합니다.

### 4. Padding vs Margin
현재 Slate 구현에는 **Padding만 있습니다** (슬롯 내부 여백).
위젯 외부 여백이 필요하면 **투명한 SPanel로 감싸세요**.

### 5. 동적 크기 변경
창 크기가 변경되면 **자동으로 ArrangeChildren()이 호출**됩니다.
별도 처리 불필요!

---

## 디버깅

### 레이아웃 문제 해결

**문제: 위젯이 보이지 않음**
1. `bIsVisible = true` 확인
2. `GetHeight()` / `GetWidth()` 반환값 확인 (0이면 안보임)
3. 부모 패널에 `AddChild()` 또는 `AttachWidget()` 호출 확인

**문제: 위젯이 중간에 배치됨**
1. 부모의 VAlign 기본값 확인 (`VAlign_Center`인지?)
2. `.VAlignment(VAlign_Fill)` 명시

**문제: 크기가 이상함**
1. SizeRule 확인 (Auto/Fill/Fixed)
2. FillHeight/FillWidth 비율 계산 확인
3. Padding 값 확인

**디버그 로그 활용:**
```cpp
// SVerticalBox.cpp에 이미 디버그 로그가 있습니다
static int debugCount = 0;
if (debugCount < 3)
{
    UE_LOG("CalculateSlotSizes: TotalHeight=%.1f, Slots=%d", TotalHeight, (int)Slots.Num());
    debugCount++;
}
```

---

## 참고 자료

- `Docs/Slate_Complete_Guide.md` - 전체 Slate 시스템 가이드
- `Mundi/Source/Slate/Core/` - Slate 소스 코드
- `Mundi/Source/Slate/UObject/Windows/ParticleEditorWindow.cpp` - 실전 예제

---

**작성일:** 2025-01-23
**버전:** 1.0
