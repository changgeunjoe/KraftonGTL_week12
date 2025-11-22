#include "pch.h"
#include "SPanel.h"
#include "ImGui/imgui.h"
#include <algorithm>

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

void SPanel::AddChild(SPanel* Child)
{
	if (Child == nullptr)
		return;

	Children.push_back(Child);
	Child->SetParent(this);
	Invalidate();
}

void SPanel::RemoveChild(SPanel* Child)
{
	if (Child == nullptr)
		return;

	auto it = std::find(Children.begin(), Children.end(), Child);
	if (it != Children.end())
	{
		(*it)->SetParent(nullptr);
		Children.erase(it);
		Invalidate();
	}
}

void SPanel::ClearChildren()
{
	for (SPanel* Child : Children)
	{
		if (Child)
		{
			Child->SetParent(nullptr);
			delete Child;
		}
	}
	Children.clear();
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

	// Invalidation: 변경 없으면 스킵
	if (!bNeedsRepaint && !bAlwaysRepaint)
		return;

	// 자식 배치
	ArrangeChildren();

	// 콘텐츠 렌더링
	RenderContent();

	// 자식들 렌더링
	RenderChildren();

	// 마우스 이벤트 처리
	ProcessMouseEvents();

	//bNeedsRepaint = false;
}

void SPanel::ProcessMouseEvents()
{
	ImGuiIO& IO = ImGui::GetIO();
	FVector2D MousePos(IO.MousePos.x, IO.MousePos.y);

	// 마우스 이동 이벤트
	OnMouseMove(MousePos);
	for (SPanel* Child : Children)
	{
		if (Child && Child->IsVisible())
		{
			Child->OnMouseMove(MousePos);
		}
	}

	// 마우스 다운 이벤트
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		OnMouseDown(MousePos, 0);
		for (SPanel* Child : Children)
		{
			if (Child && Child->IsVisible())
			{
				Child->OnMouseDown(MousePos, 0);
			}
		}
	}

	// 마우스 업 이벤트
	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		OnMouseUp(MousePos, 0);
		for (SPanel* Child : Children)
		{
			if (Child && Child->IsVisible())
			{
				Child->OnMouseUp(MousePos, 0);
			}
		}
	}
}

void SPanel::RenderChildren()
{
	for (SPanel* Child : Children)
	{
		if (Child && Child->IsVisible())
		{
			Child->OnRender();
		}
	}
}
