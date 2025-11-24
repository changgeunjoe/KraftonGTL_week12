#pragma once
#include "Source/Slate/Core/Windows/SWindow.h"

struct FMargin
{
	float Left;
	float Top;
	float Right;
	float Bottom;

	FMargin() : Left(0), Top(0), Right(0), Bottom(0) {}
	FMargin(float Uniform) : Left(Uniform), Top(Uniform), Right(Uniform), Bottom(Uniform) {}
	FMargin(float Horizontal, float Vertical) : Left(Horizontal), Top(Vertical), Right(Horizontal), Bottom(Vertical) {}
	FMargin(float InLeft, float InTop, float InRight, float InBottom)
		: Left(InLeft), Top(InTop), Right(InRight), Bottom(InBottom) {}
};

/**
 * SPanel - 모든 Slate 위젯의 기반이 되는 베이스 클래스
 * 자식 위젯 관리 기능을 제공합니다.
 */
class SPanel : public SWindow
{
public:
	SPanel();
	virtual ~SPanel();

	// ===== 자식 관리 =====
	void AddChild(SWindow* Child);
	void RemoveChild(SWindow* Child);
	void ClearChildren();
	const TArray<SWindow*>& GetChildren() const { return Children; }
	uint32 GetChildCount() const { return static_cast<uint32>(Children.Num()); }

	// ===== 가시성 =====
	void SetVisible(bool bInVisible);
	bool IsVisible() const { return bIsVisible; }

	// ===== 활성화 =====
	void SetEnabled(bool bInEnabled) { bIsEnabled = bInEnabled; }
	bool IsEnabled() const { return bIsEnabled; }

	// ===== 렌더링 =====
	virtual void OnRender() override;
	virtual void RenderContent() {}  // 파생 클래스에서 구현
	virtual void RenderChildren();    // 자식들 렌더링
	virtual void ProcessMouseEvents(); // 마우스 이벤트 처리

	// ===== 레이아웃 =====
	virtual void ArrangeChildren() {}  // 파생 클래스에서 자식 배치 로직 구현
	virtual void OnPanelResized() {}   // 패널 크기 변경 시 호출

	// ===== Invalidation (선택적 리렌더링) =====
	void Invalidate();
	bool NeedsRepaint() const { return bNeedsRepaint; }

	// ===== 부모 관리 =====
	void SetParent(SPanel* InParent) { ParentPanel = InParent; }
	SPanel* GetParent() const { return ParentPanel; }

protected:
	TArray<SWindow*> Children;
	SPanel* ParentPanel = nullptr;

	bool bIsVisible = true;
	bool bIsEnabled = true;
	bool bNeedsRepaint = true;
	bool bAlwaysRepaint = false;  // 매 프레임 강제 렌더링 (디버깅용)
};
