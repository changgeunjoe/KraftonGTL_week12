#pragma once
#include "SCompoundWidget.h"
#include "Delegates.h"
#include <vector>

/**
 * SListView - 리스트 뷰 위젯
 *
 * 특징:
 * - 스크롤 가능한 항목 리스트
 * - 단일/다중 선택 모드
 * - 항목 추가/제거/정렬
 * - 델리게이트 이벤트
 *
 * 사용 예시:
 * auto ListView = new SListView();
 * ListView->AddItem("Item 1");
 * ListView->AddItem("Item 2");
 * ListView->SetSelectionMode(SListView::Single);
 * ListView->OnSelectionChanged.Add([](uint32 Index, const FString& Item) {
 *     // 선택 변경 처리
 * });
 */
class SListView : public SCompoundWidget
{
public:
    enum ESelectionMode
    {
        None,       // 선택 불가
        Single,     // 단일 선택
        Multi       // 다중 선택
    };

    SListView();
    virtual ~SListView();

    // ===== 항목 관리 =====
    void AddItem(const FString& Item);
    void RemoveItem(uint32 Index);
    void ClearItems();
    const TArray<FString>& GetItems() const { return Items; }
    uint32 GetItemCount() const { return static_cast<uint32>(Items.size()); }

    // ===== 선택 관리 =====
    void SetSelectionMode(ESelectionMode Mode) { SelectionMode = Mode; Invalidate(); }
    ESelectionMode GetSelectionMode() const { return SelectionMode; }

    void SetSelectedIndex(uint32 Index);
    uint32 GetSelectedIndex() const { return SelectedIndex; }
    FString GetSelectedItem() const;

    // 다중 선택
    void AddSelection(uint32 Index);
    void RemoveSelection(uint32 Index);
    const TArray<uint32>& GetSelectedIndices() const { return SelectedIndices; }

    // ===== 스타일 =====
    void SetItemHeight(float Height) { ItemHeight = Height; Invalidate(); }
    float GetItemHeight() const { return ItemHeight; }

    // ===== 델리게이트 =====
    TDelegate<uint32, const FString&> OnSelectionChanged;  // (Index, Item)
    TDelegate<uint32, const FString&> OnItemDoubleClicked; // (Index, Item)

    // ===== 크기 =====
    void SetSize(float Width, float Height) { ListWidth = Width; ListHeight = Height; Invalidate(); }
    virtual float GetWidth() const override
    {
        float Width = Rect.GetWidth();
        return Width > 0.0f ? Width : ListWidth;
    }
    virtual float GetHeight() const override
    {
        float Height = Rect.GetHeight();
        return Height > 0.0f ? Height : ListHeight;
    }

    // ===== 렌더링 =====
    virtual void RenderContent() override;

private:
    TArray<FString> Items;
    TArray<uint32> SelectedIndices;
    uint32 SelectedIndex = -1;
    ESelectionMode SelectionMode = Single;
    float ItemHeight = 20.0f;
    float ListWidth = 200.0f;
    float ListHeight = 150.0f;

    bool IsSelected(uint32 Index) const;
    FString GetImGuiID() const;
};
