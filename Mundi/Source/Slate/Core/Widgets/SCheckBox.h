#pragma once
#include "SCompoundWidget.h"
#include "Delegates.h"

/**
 * SCheckBox - 체크박스 위젯
 *
 * 특징:
 * - On/Off 상태 토글
 * - 라벨 텍스트 지원
 * - 델리게이트를 통한 상태 변경 이벤트
 *
 * 사용 예시:
 * auto CheckBox = new SCheckBox();
 * CheckBox->SetLabel("Enable Feature");
 * CheckBox->SetChecked(true);
 * CheckBox->OnCheckStateChanged.Add([](bool bIsChecked) {
 *     // 상태 변경 처리
 * });
 */
class SCheckBox : public SCompoundWidget
{
public:
    SCheckBox();
    SCheckBox(const FString& InLabel, bool bInChecked = false);
    virtual ~SCheckBox();

    // ===== 상태 설정 =====
    void SetChecked(bool bInChecked);
    bool IsChecked() const { return bIsChecked; }
    void ToggleChecked() { SetChecked(!bIsChecked); }

    // ===== 라벨 설정 =====
    void SetLabel(const FString& InLabel) { Label = InLabel; }
    FString GetLabel() const { return Label; }

    // ===== 델리게이트 (이벤트) =====
    TDelegate<bool> OnCheckStateChanged;  // 체크 상태가 변경될 때

    // ===== 렌더링 =====
    virtual void RenderContent() override;

private:
    bool bIsChecked = false;
    FString Label;

    // ImGui ID (고유 식별자)
    FString GetImGuiID() const;
};
