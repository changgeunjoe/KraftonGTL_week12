#include "pch.h"
#include "SPanel.h"
#include "ImGui/imgui.h"

SPanel::SPanel()
	: bIsVisible(true)
	, bIsEnabled(true)
	, bNeedsRepaint(true)
	, bAlwaysRepaint(false)
	, ParentPanel(nullptr)
{
}

SPanel::~SPanel()
{
	ClearChildren();
}

void SPanel::AddChild(SWindow* Child)
{
	if (Child == nullptr)
		return;

	Children.Add(Child);

	// SPanel인 경우에만 SetParent 호출 (dynamic_cast 사용)
	SPanel* ChildPanel = dynamic_cast<SPanel*>(Child);
	if (ChildPanel)
	{
		ChildPanel->SetParent(this);
	}

	Invalidate();
}

void SPanel::RemoveChild(SWindow* Child)
{
	if (Child == nullptr)
		return;

	int32 Index = Children.Find(Child);
	if (Index != -1)
	{
		// SPanel인 경우에만 SetParent(nullptr) 호출
		SPanel* ChildPanel = dynamic_cast<SPanel*>(Children[Index]);
		if (ChildPanel)
		{
			ChildPanel->SetParent(nullptr);
		}

		Children.RemoveAt(Index);
		Invalidate();
	}
}

void SPanel::ClearChildren()
{
	for (SWindow* Child : Children)
	{
		if (Child)
		{
			// SPanel인 경우에만 SetParent(nullptr) 호출
			SPanel* ChildPanel = dynamic_cast<SPanel*>(Child);
			if (ChildPanel)
			{
				ChildPanel->SetParent(nullptr);
			}

			delete Child;
		}
	}
	Children.Empty();
	Invalidate();
}

void SPanel::SetVisible(bool bInVisible)
{
	if (bIsVisible != bInVisible)
	{
		bIsVisible = bInVisible;
		Invalidate();
	}
}

void SPanel::Invalidate()
{
	bNeedsRepaint = true;

	// 부모에게도 전파
	if (ParentPanel)
	{
		ParentPanel->Invalidate();
	}
}

void SPanel::OnRender()
{
	if (!bIsVisible)
		return;

	// 레이아웃 재계산 (Invalidate 호출 시에만)
	// ArrangeChildren()는 비용이 크므로 변경 시에만 실행
	// if (bNeedsRepaint || bAlwaysRepaint)
	// {
	// 	ArrangeChildren();
	// 	bNeedsRepaint = false;
	// }
	ArrangeChildren();
	// ImGui는 즉시 모드 렌더링이므로 매 프레임 실행 필수
	RenderContent();
	RenderChildren();
	ProcessMouseEvents();
}

void SPanel::ProcessMouseEvents()
{
	ImGuiIO& IO = ImGui::GetIO();
	FVector2D MousePos(IO.MousePos.x, IO.MousePos.y);

	// 마우스 이동 이벤트
	OnMouseMove(MousePos);
	for (SWindow* Child : Children)
	{
		if (Child)
		{
			Child->OnMouseMove(MousePos);
		}
	}

	// 마우스 다운 이벤트
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		OnMouseDown(MousePos, 0);
		for (SWindow* Child : Children)
		{
			if (Child)
			{
				Child->OnMouseDown(MousePos, 0);
			}
		}
	}

	// 마우스 업 이벤트
	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		OnMouseUp(MousePos, 0);
		for (SWindow* Child : Children)
		{
			if (Child)
			{
				Child->OnMouseUp(MousePos, 0);
			}
		}
	}
}

void SPanel::RenderChildren()
{
	for (SWindow* Child : Children)
	{
		if (Child)
		{
			Child->OnRender();
		}
	}
}
