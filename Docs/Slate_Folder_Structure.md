# Slate 폴더 구조 분리 완료

## 개요
Mundi 엔진의 Slate UI 시스템을 **UObject 기반**과 **순수 C++ 기반**으로 명확히 분리했습니다.

---

## 새로운 폴더 구조

```
Mundi/Source/Slate/
├── Core/                           # 순수 C++ (SWindow 기반)
│   ├── Windows/
│   │   ├── SWindow.h/cpp           # 베이스 클래스
│   │   ├── SSplitter.h/cpp         # 스플리터 베이스
│   │   ├── SSplitterH.h/cpp        # 수평 스플리터
│   │   ├── SSplitterV.h/cpp        # 수직 스플리터
│   │   ├── SDetailsWindow.h/cpp    # 디테일 패널
│   │   ├── SViewportWindow.h/cpp   # 뷰포트
│   │   └── SControlPanel.h/cpp     # 컨트롤 패널
│   │
│   ├── Panels/
│   │   ├── SPanel.h/cpp            # Slate 패널 베이스
│   │   ├── SVerticalBox.h/cpp      # 수직 레이아웃
│   │   └── SHorizontalBox.h/cpp    # 수평 레이아웃
│   │
│   └── Widgets/
│       ├── SButton.h/cpp           # 버튼 위젯
│       └── STextBlock.h/cpp        # 텍스트 위젯
│
└── UObject/                        # UObject 기반 (UWidget/UUIWindow 기반)
    ├── Windows/
    │   ├── UIWindow.h/cpp          # UObject 윈도우 베이스
    │   ├── AnimStateMachineWindow.h/cpp
    │   ├── BlendSpace2DEditorWindow.h/cpp
    │   ├── ConsoleWindow.h/cpp
    │   ├── ContentBrowserWindow.h/cpp
    │   ├── ControlPanelWindow.h/cpp
    │   ├── ExperimentalFeatureWindow.h/cpp
    │   ├── PreviewWindow.h/cpp
    │   ├── PropertyWindow.h/cpp
    │   └── SceneWindow.h/cpp
    │
    └── Widgets/
        ├── Widget.h/cpp            # UObject 위젯 베이스
        ├── ConsoleWidget.h/cpp
        ├── InputInformationWidget.h/cpp
        ├── MainToolbarWidget.h/cpp
        ├── PropertyRenderer.h/cpp
        ├── SceneManagerWidget.h/cpp
        └── TargetActorTransformWidget.h/cpp
```

---

## 분리 기준

### 1. **Core/ (순수 C++)**
- **상속 관계**: `SWindow` 기반
- **특징**:
  - UObject 오버헤드 없음 (성능 중시)
  - 리플렉션 시스템 미사용
  - 복잡한 에디터 윈도우에 적합
  - 수동 프로퍼티 편집 구현

- **사용 예시**:
  ```cpp
  #include "Slate/Core/Windows/SWindow.h"
  #include "Slate/Core/Panels/SVerticalBox.h"

  class MyEditorWindow : public SWindow
  {
      SVerticalBox* Layout;
  };
  ```

### 2. **UObject/ (UObject 기반)**
- **상속 관계**: `UWidget` 또는 `UUIWindow` 기반
- **특징**:
  - UObject 리플렉션 활용
  - PropertyRenderer와 연동 가능
  - 간단한 UI에 적합
  - ImGui 직접 호출

- **사용 예시**:
  ```cpp
  #include "Slate/UObject/Widgets/Widget.h"

  class UMyWidget : public UWidget
  {
      DECLARE_CLASS(UMyWidget, UWidget)

      virtual void RenderWidget() override
      {
          ImGui::Text("Hello");
      }
  };
  ```

---

## Include 경로 변경

### Core (순수 C++)

**변경 전:**
```cpp
#include "Slate/Windows/SWindow.h"
#include "Slate/Panels/SPanel.h"
```

**변경 후:**
```cpp
#include "Slate/Core/Windows/SWindow.h"
#include "Slate/Core/Panels/SPanel.h"
#include "Slate/Core/Widgets/SButton.h"
```

### UObject

**변경 전:**
```cpp
#include "Slate/Widgets/Widget.h"
#include "Slate/Windows/UIWindow.h"
```

**변경 후:**
```cpp
#include "Slate/UObject/Widgets/Widget.h"
#include "Slate/UObject/Windows/UIWindow.h"
```

---

## 선택 가이드

### Core (SWindow 기반)을 사용해야 할 때:

✅ 복잡한 에디터 제작 (Material, Particle, Animation 등)
✅ 성능이 중요한 경우
✅ 계층적 레이아웃이 필요한 경우
✅ Slate의 자동 레이아웃 기능을 활용하고 싶을 때

**예시**: BlendSpace2D, AnimStateMachine (현재는 UObject지만 Core로 전환 가능)

### UObject (UWidget/UUIWindow)을 사용해야 할 때:

✅ 간단한 UI 위젯
✅ 리플렉션 시스템이 필요한 경우
✅ PropertyRenderer 자동 UI 생성을 사용하고 싶을 때
✅ UObject 생명주기 관리가 필요한 경우

**예시**: MainToolbar, ConsoleWidget, SceneManager

---

## 마이그레이션 가이드

기존 코드를 Core/UObject로 전환하려면:

### UWidget → SPanel (Core)

**변경 전:**
```cpp
class UMyWidget : public UWidget
{
    DECLARE_CLASS(UMyWidget, UWidget)

    void RenderWidget() override
    {
        ImGui::Text("Hello");
    }
};
```

**변경 후:**
```cpp
#include "Slate/Core/Panels/SPanel.h"

class SMyWidget : public SPanel
{
    void RenderContent() override
    {
        ImDrawList* DrawList = ImGui::GetWindowDrawList();
        DrawList->AddText(ImVec2(Rect.Left, Rect.Top),
                         IM_COL32(255,255,255,255), "Hello");
    }
};
```

---

## 장점

### 1. **명확한 분리**
- UObject와 순수 C++ 코드가 명확히 구분됨
- 각각의 용도에 맞는 선택 가능

### 2. **유지보수 향상**
- 파일 찾기 쉬움
- 의존성 명확

### 3. **확장성**
- Core: 새로운 Slate 위젯 추가 시 일관된 구조
- UObject: 기존 UWidget 시스템 유지

### 4. **성능**
- Core는 UObject 오버헤드 없음
- UObject는 리플렉션 장점 활용

---

## 다음 단계

### Phase 2: 추가 Core 위젯 구현
- SEditableText (텍스트 입력)
- SCheckBox (체크박스)
- SComboBox (드롭다운)
- SScrollBox (스크롤)
- SGridPanel (그리드 레이아웃)

### Phase 3: 기존 복잡한 에디터를 Core로 전환
- BlendSpace2DEditorWindow → SBlendSpace2DEditor
- AnimStateMachineWindow → SAnimStateMachineEditor

---

**작성일**: 2025-01-21
**버전**: 폴더 분리 완료
