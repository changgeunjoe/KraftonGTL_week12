#include "pch.h"
#include "SViewportPanel.h"
#include "ImGui/imgui.h"

SViewportPanel::SViewportPanel(const FString& InChildName)
	: ChildName(InChildName)
{
}

SViewportPanel::~SViewportPanel()
{
}

void SViewportPanel::RenderContent()
{
	if (!bIsVisible)
		return;

	FRect ContentRect = GetRect();
	ImVec2 Size(ContentRect.GetWidth(), ContentRect.GetHeight());

	// BeginChild로 감싸서 InputManager가 인식하도록 함
	ImGui::SetCursorScreenPos(ImVec2(ContentRect.Left, ContentRect.Top));

	// ViewportRenderArea를 한 단계 더 감싸기
	ImGui::BeginChild(ChildName.c_str(), Size, true, ImGuiWindowFlags_NoScrollbar);
	{
		// 내부에 실제 렌더링 영역 생성
		ImGui::BeginChild("ViewportRenderArea", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);
		{
			// BeginChild 내부의 실제 렌더 영역 계산
			ImVec2 childPos = ImGui::GetWindowPos();
			ImVec2 childSize = ImGui::GetWindowSize();
			ImVec2 rectMin = childPos;
			ImVec2 rectMax(childPos.x + childSize.x, childPos.y + childSize.y);

			// Rect를 내부 영역으로 업데이트 (부모가 참조할 수 있도록)
			Rect.Left = rectMin.x;
			Rect.Top = rectMin.y;
			Rect.Right = rectMax.x;
			Rect.Bottom = rectMax.y;
			Rect.UpdateMinMax();

			// 자식 위젯들은 여기서 렌더링됨
			// 실제 3D 뷰포트는 OnRenderViewport()에서 별도로 렌더링
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();
}
