#pragma once
#include "SCompoundWidget.h"
#include "Delegates.h"
#include <vector>
#include <memory>

/**
 * STreeNode - 트리 노드 데이터
 */
class STreeNode
{
public:
    STreeNode(const FString& InLabel) : Label(InLabel), bIsExpanded(false) {}

    FString GetLabel() const { return Label; }
    void SetLabel(const FString& InLabel) { Label = InLabel; }

    bool IsExpanded() const { return bIsExpanded; }
    void SetExpanded(bool bExpanded) { bIsExpanded = bExpanded; }

    STreeNode* AddChild(const FString& ChildLabel)
    {
        auto Child = std::make_shared<STreeNode>(ChildLabel);
        Child->Parent = this;
        Children.push_back(Child);
        return Child.get();
    }

    const TArray<std::shared_ptr<STreeNode>>& GetChildren() const { return Children; }
    STreeNode* GetParent() const { return Parent; }

private:
    FString Label;
    bool bIsExpanded;
    STreeNode* Parent = nullptr;
    TArray<std::shared_ptr<STreeNode>> Children;
};

/**
 * STreeView - 트리 뷰 위젯
 *
 * 특징:
 * - 계층 구조 데이터 표시
 * - 노드 확장/축소
 * - 노드 선택
 * - 델리게이트 이벤트
 *
 * 사용 예시:
 * auto TreeView = new STreeView();
 * auto Root = TreeView->AddRootNode("Root");
 * auto Child = Root->AddChild("Child");
 * TreeView->OnSelectionChanged.Add([](STreeNode* Node) {
 *     // 선택 변경 처리
 * });
 */
class STreeView : public SCompoundWidget
{
public:
    STreeView();
    virtual ~STreeView();

    // ===== 노드 관리 =====
    STreeNode* AddRootNode(const FString& Label);
    void ClearNodes();
    const TArray<std::shared_ptr<STreeNode>>& GetRootNodes() const { return RootNodes; }

    // ===== 선택 관리 =====
    void SetSelectedNode(STreeNode* Node);
    STreeNode* GetSelectedNode() const { return SelectedNode; }

    // ===== 델리게이트 =====
    TDelegate<STreeNode*> OnSelectionChanged;
    TDelegate<STreeNode*> OnNodeExpanded;
    TDelegate<STreeNode*> OnNodeCollapsed;

    // ===== 크기 =====
    void SetSize(float Width, float Height) { TreeWidth = Width; TreeHeight = Height; Invalidate(); }
    virtual float GetWidth() const override
    {
        float Width = Rect.GetWidth();
        return Width > 0.0f ? Width : TreeWidth;
    }
    virtual float GetHeight() const override
    {
        float Height = Rect.GetHeight();
        return Height > 0.0f ? Height : TreeHeight;
    }

    // ===== 렌더링 =====
    virtual void RenderContent() override;

private:
    TArray<std::shared_ptr<STreeNode>> RootNodes;
    STreeNode* SelectedNode = nullptr;
    STreeNode* HoveredNode = nullptr;
    float TreeWidth = 200.0f;
    float TreeHeight = 150.0f;

    void RenderNode(STreeNode* Node, uint32 Depth);
    FString GetImGuiID(STreeNode* Node) const;
};
