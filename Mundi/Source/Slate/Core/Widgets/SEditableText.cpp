#include "pch.h"
#include "SEditableText.h"
#include "ImGui/imgui.h"
#include <cstring>

SEditableText::SEditableText()
    : bIsPassword(false)
    , bIsReadOnly(false)
    , bIsFocused(false)
    , bWantsFocus(false)
    , MaxLength(256)
{
    memset(Buffer, 0, sizeof(Buffer));
}

SEditableText::SEditableText(const FString& InText)
    : SEditableText()
{
    SetText(InText);
}

SEditableText::~SEditableText()
{
}

void SEditableText::SetText(const FString& InText)
{
    Text = InText;

    // Buffer에 복사 (안전하게)
    size_t CopyLength = std::min(Text.length(), sizeof(Buffer) - 1);
    memcpy(Buffer, Text.c_str(), CopyLength);
    Buffer[CopyLength] = '\0';

    Invalidate();
}

FString SEditableText::GetImGuiID() const
{
    // 포인터 주소를 ID로 사용 (고유성 보장)
    char IDBuffer[32];
    sprintf_s(IDBuffer, "##EditableText_%p", this);
    return FString(IDBuffer);
}

void SEditableText::RenderContent()
{
    if (!bIsVisible)
        return;

    ImGui::PushID(this);  // ID 스택 푸시

    // 커서 위치 설정
    ImVec2 CursorPos(Rect.Left, Rect.Top);
    ImGui::SetCursorScreenPos(CursorPos);

    // 크기 계산
    float Width = Rect.GetWidth();
    float Height = Rect.GetHeight();

    // 최소 크기 보장
    if (Width < 50.0f) Width = 100.0f;
    if (Height < 20.0f) Height = 20.0f;

    ImGui::PushItemWidth(Width);

    // ImGui 플래그 설정
    ImGuiInputTextFlags Flags = ImGuiInputTextFlags_None;

    if (bIsPassword)
        Flags |= ImGuiInputTextFlags_Password;

    if (bIsReadOnly)
        Flags |= ImGuiInputTextFlags_ReadOnly;

    // Enter 키로 커밋
    Flags |= ImGuiInputTextFlags_EnterReturnsTrue;

    // 힌트 텍스트가 있고 텍스트가 비어있으면 힌트 표시
    bool bShowHint = Text.empty() && !HintText.empty() && !bIsFocused;

    // 포커스 요청 처리
    if (bWantsFocus)
    {
        ImGui::SetKeyboardFocusHere();
        bWantsFocus = false;
    }

    // 입력 필드 렌더링
    bool bTextChanged = false;
    bool bEnterPressed = false;

    if (bShowHint)
    {
        // 힌트 텍스트 표시 (읽기 전용, 회색)
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        char HintBuffer[256];
        strncpy_s(HintBuffer, HintText.c_str(), sizeof(HintBuffer) - 1);
        ImGui::InputText(GetImGuiID().c_str(), HintBuffer, sizeof(HintBuffer),
            ImGuiInputTextFlags_ReadOnly);
        ImGui::PopStyleColor();
    }
    else
    {
        // 일반 입력 필드
        bEnterPressed = ImGui::InputText(GetImGuiID().c_str(), Buffer, sizeof(Buffer), Flags);
        bTextChanged = ImGui::IsItemEdited();
    }

    // 포커스 상태 업데이트
    bool bWasFocused = bIsFocused;
    bIsFocused = ImGui::IsItemActive();

    // 텍스트 변경 처리
    if (bTextChanged)
    {
        FString OldText = Text;
        Text = FString(Buffer);

        // 최대 길이 제한
        if (MaxLength > 0 && Text.length() > (size_t)MaxLength)
        {
            Text = Text.substr(0, MaxLength);
            strncpy_s(Buffer, Text.c_str(), sizeof(Buffer) - 1);
        }

        if (OnTextChanged.IsBound())
        {
            OnTextChanged.Broadcast(Text);
        }

        Invalidate();
    }

    // Enter 키 또는 포커스 해제 시 Commit
    if (bEnterPressed || (bWasFocused && !bIsFocused))
    {
        if (OnTextCommitted.IsBound())
        {
            OnTextCommitted.Broadcast(Text);
        }
    }

    ImGui::PopItemWidth();
    ImGui::PopID();  // ID 스택 팝
}
