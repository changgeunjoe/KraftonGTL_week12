#pragma once
#include "SCompoundWidget.h"
#include "Delegates.h"
#include <vector>

/**
 * SComboBox - 드롭다운 선택 위젯
 *
 * 특징:
 * - 여러 옵션 중 하나 선택
 * - 델리게이트를 통한 선택 변경 이벤트
 * - 인덱스 또는 값으로 현재 선택 확인
 *
 * 사용 예시:
 * auto ComboBox = new SComboBox();
 * ComboBox->AddOption("Option 1");
 * ComboBox->AddOption("Option 2");
 * ComboBox->AddOption("Option 3");
 * ComboBox->SetSelectedIndex(0);
 * ComboBox->OnSelectionChanged.Add([](uint32 Index, const FString& Value) {
 *     // 선택 변경 처리
 * });
 */
class SComboBox : public SCompoundWidget
{
public:
    SComboBox();
    SComboBox(const TArray<FString>& InOptions);
    virtual ~SComboBox();

    // ===== 옵션 관리 =====
    void AddOption(const FString& Option);
    void SetOptions(const TArray<FString>& InOptions);
    void ClearOptions();
    const TArray<FString>& GetOptions() const { return Options; }
    uint32 GetOptionCount() const { return static_cast<uint32>(Options.size()); }

    // ===== 선택 관리 =====
    void SetSelectedIndex(uint32 Index);
    uint32 GetSelectedIndex() const { return SelectedIndex; }
    FString GetSelectedValue() const;

    // ===== 라벨 설정 =====
    void SetLabel(const FString& InLabel) { Label = InLabel; }
    FString GetLabel() const { return Label; }

    // ===== 델리게이트 (이벤트) =====
    TDelegate<uint32, const FString&> OnSelectionChanged;  // (Index, Value)

    // ===== 렌더링 =====
    virtual void RenderContent() override;

private:
    TArray<FString> Options;
    uint32 SelectedIndex = -1;
    FString Label;

    // ImGui ID (고유 식별자)
    FString GetImGuiID() const;
};
