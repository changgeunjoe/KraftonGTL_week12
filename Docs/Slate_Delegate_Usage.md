# Slate 위젯에서 기존 델리게이트 사용하기

## 개요
Mundi의 Core Slate 위젯들은 기존 `TDelegate` 시스템을 사용합니다.

---

## 기존 델리게이트 시스템

**위치**: `Mundi/Source/Runtime/Core/Misc/Delegates.h`

### TDelegate 주요 API

```cpp
template<typename... Args>
class TDelegate
{
public:
    // 핸들러 추가
    FDelegateHandle Add(const HandlerType& Handler);

    // 멤버 함수 바인딩
    template<typename TObj, typename TClass>
    FDelegateHandle AddDynamic(TObj* Instance, void(TClass::* Func)(Args...));

    // 모든 핸들러 실행
    void Broadcast(Args... args);

    // 핸들러 제거
    void Remove(FDelegateHandle Handle);

    // 모든 핸들러 제거
    void Clear();
};
```

---

## Core Slate 위젯 사용법

### SButton 이벤트 바인딩

```cpp
#include "Slate/Core/Widgets/SButton.h"

// 1. 람다로 바인딩
SButton* SaveButton = new SButton();
SaveButton->SetText("Save");
SaveButton->OnClicked.Add([]() {
    SaveScene();
});

// 2. 멤버 함수 바인딩
SaveButton->OnClicked.AddDynamic(this, &UMyClass::OnSaveClicked);

// 3. 핸들 저장 후 나중에 제거
FDelegateHandle Handle = SaveButton->OnClicked.Add([]() {
    LOG_INFO("Button clicked");
});

// 나중에 제거
SaveButton->OnClicked.Remove(Handle);
```

### 여러 핸들러 등록

```cpp
SButton* Button = new SButton();

// 여러 핸들러 추가 가능 (모두 실행됨)
Button->OnClicked.Add([]() {
    LOG_INFO("Handler 1");
});

Button->OnClicked.Add([]() {
    LOG_INFO("Handler 2");
});

Button->OnClicked.Add([]() {
    LOG_INFO("Handler 3");
});

// 클릭 시 모든 핸들러가 순서대로 실행됨
```

---

## 기존 사용법과 비교

### ❌ 이전 방식 (문서 예제)

```cpp
// 잘못된 API (제거됨)
Button->OnClicked.Bind([]() { ... });
Button->OnClicked.Execute();
Button->OnClicked.IsBound();
```

### ✅ 올바른 방식 (기존 델리게이트 사용)

```cpp
// 추가
Button->OnClicked.Add([]() { ... });

// 실행 (위젯 내부에서 자동 호출)
Button->OnClicked.Broadcast();

// 핸들러 여부는 체크 불필요 (Broadcast가 알아서 처리)
```

---

## 실전 예제

### 1. 툴바 버튼

```cpp
class UMyEditorWindow : public UUIWindow
{
private:
    SButton* PlayButton = nullptr;
    SButton* PauseButton = nullptr;
    SButton* StopButton = nullptr;

    FDelegateHandle PlayHandle;
    FDelegateHandle PauseHandle;
    FDelegateHandle StopHandle;

    void CreateToolbar()
    {
        PlayButton = new SButton();
        PlayButton->SetText("Play");
        PlayHandle = PlayButton->OnClicked.Add([this]() {
            StartSimulation();
        });

        PauseButton = new SButton();
        PauseButton->SetText("Pause");
        PauseHandle = PauseButton->OnClicked.Add([this]() {
            PauseSimulation();
        });

        StopButton = new SButton();
        StopButton->SetText("Stop");
        StopHandle = StopButton->OnClicked.Add([this]() {
            StopSimulation();
        });
    }

    void Cleanup()
    {
        // 핸들 제거 (선택사항 - 객체 파괴 시 자동 정리됨)
        if (PlayButton)
        {
            PlayButton->OnClicked.Remove(PlayHandle);
        }
    }
};
```

### 2. 멤버 함수 바인딩

```cpp
class UParticleEditor : public UUIWindow
{
public:
    void Initialize() override
    {
        SaveButton = new SButton();
        SaveButton->SetText("Save");

        // 멤버 함수 바인딩
        SaveButton->OnClicked.AddDynamic(this, &UParticleEditor::OnSave);

        // Hover 이벤트도 바인딩
        SaveButton->OnHovered.AddDynamic(this, &UParticleEditor::OnButtonHovered);
        SaveButton->OnUnhovered.AddDynamic(this, &UParticleEditor::OnButtonUnhovered);
    }

private:
    SButton* SaveButton = nullptr;

    void OnSave()
    {
        LOG_INFO("Saving particle system...");
        SaveParticleSystem();
    }

    void OnButtonHovered()
    {
        StatusText->SetText("Click to save");
    }

    void OnButtonUnhovered()
    {
        StatusText->SetText("Ready");
    }
};
```

### 3. 동적 이벤트 제거

```cpp
class UDynamicUI : public UUIWindow
{
private:
    SButton* ToggleButton = nullptr;
    FDelegateHandle ToggleHandle;
    bool bIsEnabled = false;

    void CreateButton()
    {
        ToggleButton = new SButton();
        ToggleButton->SetText("Enable");

        ToggleHandle = ToggleButton->OnClicked.Add([this]() {
            bIsEnabled = !bIsEnabled;

            if (bIsEnabled)
            {
                ToggleButton->SetText("Disable");
                EnableFeature();
            }
            else
            {
                ToggleButton->SetText("Enable");
                DisableFeature();
            }
        });
    }

    void RemoveToggleButton()
    {
        if (ToggleButton)
        {
            // 이벤트 제거
            ToggleButton->OnClicked.Remove(ToggleHandle);

            // 버튼 삭제
            delete ToggleButton;
            ToggleButton = nullptr;
        }
    }
};
```

---

## 주의사항

### 1. 핸들 관리
```cpp
// ✅ 좋은 방법: 핸들 저장
FDelegateHandle Handle = Button->OnClicked.Add(...);
// 나중에 제거 가능
Button->OnClicked.Remove(Handle);

// ⚠️ 핸들 저장 안 함
Button->OnClicked.Add(...);
// 나중에 특정 핸들러만 제거 불가능
// Clear()로 전체 제거만 가능
```

### 2. 객체 수명
```cpp
class UMyWindow : public UUIWindow
{
private:
    SButton* Button = nullptr;

    void CreateButton()
    {
        Button = new SButton();

        // ❌ 위험: this 캡처 시 수명 주의
        Button->OnClicked.Add([this]() {
            // UMyWindow가 파괴된 후에도 호출될 수 있음!
            DoSomething();
        });
    }

    ~UMyWindow()
    {
        // ✅ 안전: 버튼 파괴 전에 핸들러 정리
        if (Button)
        {
            Button->OnClicked.Clear();
            delete Button;
        }
    }
};
```

### 3. 멀티스레드
```cpp
// ⚠️ 주의: TDelegate는 멀티스레드 안전하지 않음
// UI 스레드에서만 사용할 것

Button->OnClicked.Add([]() {
    // ✅ UI 스레드에서 실행
    UpdateUI();
});

// ❌ 다른 스레드에서 Broadcast 호출 금지
std::thread([&]() {
    Button->OnClicked.Broadcast();  // 크래시 가능!
}).detach();
```

---

## 요약

| 작업 | API | 예시 |
|------|-----|------|
| **이벤트 추가** | `Add()` | `Button->OnClicked.Add([]() { ... })` |
| **멤버 함수 바인딩** | `AddDynamic()` | `Button->OnClicked.AddDynamic(this, &Class::Func)` |
| **이벤트 실행** | `Broadcast()` | 위젯 내부에서 자동 호출 |
| **이벤트 제거** | `Remove(Handle)` | `Button->OnClicked.Remove(Handle)` |
| **전체 제거** | `Clear()` | `Button->OnClicked.Clear()` |

---

**작성일**: 2025-01-21
**버전**: 기존 델리게이트 통합
