#include "pch.h"
#include "STreeView.h"
#include "ImGui/imgui.h"
#include <string>

STreeView::STreeView()
    : SelectedNode(nullptr)
    , HoveredNode(nullptr)
{
}

STreeView::~STreeView()
{
    ClearNodes();
}

STreeNode* STreeView::AddRootNode(const FString& Label)
{
    auto Node = std::make_shared<STreeNode>(Label);
    RootNodes.Add(Node);
    Invalidate();
    return Node.get();
}

void STreeView::ClearNodes()
{
    RootNodes.Empty();
    SelectedNode = nullptr;
    HoveredNode = nullptr;
    Invalidate();
}

void STreeView::SetSelectedNode(STreeNode* Node)
{
    if (SelectedNode != Node)
    {
        SelectedNode = Node;

        if (OnSelectionChanged.IsBound())
        {
            OnSelectionChanged.Broadcast(SelectedNode);
        }

        Invalidate();
    }
}

FString STreeView::GetImGuiID(STreeNode* Node) const
{
    return "##TreeNode_" + std::to_string(reinterpret_cast<uintptr_t>(Node));
}

void STreeView::RenderNode(STreeNode* Node, uint32 Depth)
{
    if (!Node)
        return;

    // 들여쓰기
    ImGui::Indent(Depth > 0 ? 16.0f : 0.0f);

    bool bHasChildren = !Node->GetChildren().IsEmpty();
    bool bIsExpanded = Node->IsExpanded();
    bool bIsSelected = (Node == SelectedNode);

    // TreeNode 플래그
    ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    if (bIsSelected)
        Flags |= ImGuiTreeNodeFlags_Selected;

    if (!bHasChildren)
        Flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    // 노드 렌더링
    FString NodeLabel = Node->GetLabel() + GetImGuiID(Node);
    bool bNodeOpen = ImGui::TreeNodeEx(NodeLabel.c_str(), Flags);

    // 클릭 감지
    if (ImGui::IsItemClicked())
    {
        SetSelectedNode(Node);
    }

    // 확장/축소 상태 변경 감지
    if (bHasChildren)
    {
        bool bCurrentlyOpen = ImGui::IsItemToggledOpen();
        if (bCurrentlyOpen != bIsExpanded)
        {
            Node->SetExpanded(bCurrentlyOpen);

            if (bCurrentlyOpen && OnNodeExpanded.IsBound())
            {
                OnNodeExpanded.Broadcast(Node);
            }
            else if (!bCurrentlyOpen && OnNodeCollapsed.IsBound())
            {
                OnNodeCollapsed.Broadcast(Node);
            }
        }
    }

    // 자식 노드 렌더링
    if (bNodeOpen && bHasChildren)
    {
        for (const auto& Child : Node->GetChildren())
        {
            RenderNode(Child.get(), Depth + 1);
        }
        ImGui::TreePop();
    }

    if (Depth > 0)
        ImGui::Unindent(16.0f);
}

void STreeView::RenderContent()
{
    if (!bIsVisible)
        return;

    // 스크롤 영역으로 감싸기 (부모가 설정한 Rect 사용, 없으면 기본 크기)
    float Width = Rect.GetWidth();
    float Height = Rect.GetHeight();

    if (Width <= 0.0f) Width = TreeWidth;
    if (Height <= 0.0f) Height = TreeHeight;

    ImVec2 TreeSize(Width, Height);

    // Rect 위치로 커서 이동
    ImGui::SetCursorScreenPos(ImVec2(Rect.Left, Rect.Top));

    ImGui::BeginChild(("##TreeView_" + std::to_string(reinterpret_cast<uintptr_t>(this))).c_str(),
        TreeSize, true);

    // 루트 노드들 렌더링
    for (const auto& RootNode : RootNodes)
    {
        RenderNode(RootNode.get(), 0);
    }

    ImGui::EndChild();
}
