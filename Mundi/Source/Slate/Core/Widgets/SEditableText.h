#pragma once
#include "SCompoundWidget.h"
#include "Delegates.h"
#include <string>

/**
 * SEditableText - 텍스트 입력 위젯
 *
 * 특징:
 * - 사용자가 텍스트를 입력/편집 가능
 * - 힌트 텍스트 지원 (Placeholder)
 * - 비밀번호 모드 지원
 * - 델리게이트를 통한 이벤트 처리
 *
 * 사용 예시:
 * auto Input = new SEditableText();
 * Input->SetText("Initial Value");
 * Input->SetHintText("Enter name...");
 * Input->OnTextChanged.Add([](const FString& NewText) {
 *     // 텍스트 변경 시 처리
 * });
 */
class SEditableText : public SCompoundWidget
{
public:
    SEditableText();
    SEditableText(const FString& InText);
    virtual ~SEditableText();

    // ===== 텍스트 설정 =====
    void SetText(const FString& InText);
    FString GetText() const { return Text; }

    // ===== 힌트 텍스트 (Placeholder) =====
    void SetHintText(const FString& InHint) { HintText = InHint; }
    FString GetHintText() const { return HintText; }

    // ===== 스타일 설정 =====
    void SetIsPassword(bool bInPassword) { bIsPassword = bInPassword; }
    bool IsPassword() const { return bIsPassword; }

    void SetIsReadOnly(bool bInReadOnly) { bIsReadOnly = bInReadOnly; }
    bool IsReadOnly() const { return bIsReadOnly; }

    void SetMaxLength(int32_t InMaxLength) { MaxLength = InMaxLength; }
    int32_t GetMaxLength() const { return MaxLength; }

    // ===== 포커스 =====
    void SetFocus() { bWantsFocus = true; }
    bool IsFocused() const { return bIsFocused; }

    // ===== 델리게이트 (이벤트) =====
    TDelegate<const FString&> OnTextChanged;    // 텍스트가 변경될 때마다
    TDelegate<const FString&> OnTextCommitted;  // Enter 또는 포커스 해제 시

    // ===== 렌더링 =====
    virtual void RenderContent() override;

private:
    FString Text;                   // 현재 텍스트
    FString HintText;               // 힌트 텍스트 (비어있을 때 표시)

    bool bIsPassword = false;       // 비밀번호 모드
    bool bIsReadOnly = false;       // 읽기 전용
    bool bIsFocused = false;        // 현재 포커스 상태
    bool bWantsFocus = false;       // 포커스를 요청함

    int32_t MaxLength = 256;        // 최대 길이
    char Buffer[512];               // ImGui용 버퍼 (임시)

    // ImGui ID (고유 식별자)
    FString GetImGuiID() const;
};
