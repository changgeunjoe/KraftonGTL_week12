#pragma once
#include "Source/Slate/Core/Windows/SWindow.h"

class SVerticalBox;
class SHorizontalBox;
class SScrollBox;
class SButton;
class STextBlock;
class STreeView;
class SListView;
class SPanel;
class SViewportPanel;
class ViewerState;

class STreeNode;

class SParticleEditorWindow : public SWindow
{
public:
	SParticleEditorWindow();
	virtual ~SParticleEditorWindow();

	bool Initialize(float StartX, float StartY, float Width, float Height, class UWorld* InWorld, ID3D11Device* InDevice);

	virtual void OnRender() override;
	virtual void OnMouseDown(FVector2D MousePos, uint32 Button) override;
	virtual void OnMouseUp(FVector2D MousePos, uint32 Button) override;
	virtual void OnMouseMove(FVector2D MousePos) override;

	void OnRenderViewport();

	bool IsOpen() const { return bIsOpen; }
	void Close() { bIsOpen = false; }

private:
	// Layout hierarchy
	SVerticalBox* RootLayout = nullptr;
	SHorizontalBox* TopToolbar = nullptr;
	SHorizontalBox* MainContentArea = nullptr;

	// Left Column (Viewport + Details)
	SVerticalBox* LeftColumn = nullptr;
	SVerticalBox* ViewportPanel = nullptr;
	SViewportPanel* ViewportPlaceholder = nullptr;
	SVerticalBox* DetailsPanel = nullptr;
	STextBlock* DetailsTitle = nullptr;
	SScrollBox* DetailsScrollBox = nullptr;

	// Right Column (Emitters + Curve Editor)
	SVerticalBox* RightColumn = nullptr;
	SVerticalBox* EmittersPanel = nullptr;
	STextBlock* EmittersTitle = nullptr;
	SScrollBox* EmittersScrollBox = nullptr;
	SHorizontalBox* EmittersListRow = nullptr;

	SVerticalBox* CurveEditorPanel = nullptr;
	STextBlock* CurveEditorTitle = nullptr;
	SPanel* CurveEditorPlaceholder = nullptr;

	// Top Toolbar Widgets
	STextBlock* TitleText = nullptr;
	SButton* PlayButton = nullptr;
	SButton* PauseButton = nullptr;
	SButton* ResetButton = nullptr;
	SButton* RestartSimButton = nullptr;
	SButton* RestartLevelButton = nullptr;
	STextBlock* StatusText = nullptr;

	// Preview Viewport
	ViewerState* PreviewState = nullptr;
	FRect PreviewViewportRect = FRect(0, 0, 0, 0);
	ID3D11Device* Device = nullptr;

	// State
	bool bIsOpen = true;
	bool bIsPlaying = false;

	void CreateLayout();
	void CreateTopToolbar();
	void CreateLeftColumn();
	void CreateViewportPanel();
	void CreateDetailsPanel();
	void CreateRightColumn();
	void CreateEmittersPanel();
	void CreateCurveEditorPanel();

	// Event handlers
	void OnPlayClicked();
	void OnPauseClicked();
	void OnResetClicked();
	void OnRestartSimClicked();
	void OnRestartLevelClicked();
};
