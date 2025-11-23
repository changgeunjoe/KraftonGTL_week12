#pragma once
#include "SPanel.h"

/**
 * SViewportPanel - ImGui::BeginChild로 감싸진 뷰포트 패널
 * InputManager가 마우스/키보드 입력을 허용하도록 특별한 이름을 가집니다.
 */
class SViewportPanel : public SPanel
{
public:
	SViewportPanel(const FString& InChildName = "ViewportRenderArea");
	virtual ~SViewportPanel();

	virtual void RenderContent() override;

	void SetChildName(const FString& InName) { ChildName = InName; }
	FString GetChildName() const { return ChildName; }

private:
	FString ChildName;
};
