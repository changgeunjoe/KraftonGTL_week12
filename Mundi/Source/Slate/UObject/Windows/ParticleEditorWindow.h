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

	// Left Panel - Emitter List
	SVerticalBox* LeftPanel = nullptr;
	STextBlock* EmitterListTitle = nullptr;
	STreeView* EmitterTreeView = nullptr;
	SHorizontalBox* EmitterButtonRow = nullptr;
	SButton* AddEmitterButton = nullptr;
	SButton* RemoveEmitterButton = nullptr;

	// Center Panel - Preview Viewport
	SVerticalBox* CenterPanel = nullptr;
	STextBlock* ViewportTitle = nullptr;
	SViewportPanel* ViewportPlaceholder = nullptr;
	SHorizontalBox* ViewportToolbar = nullptr;
	SButton* PlayButton = nullptr;
	SButton* PauseButton = nullptr;
	SButton* ResetButton = nullptr;

	// Right Panel - Module Details
	SVerticalBox* RightPanel = nullptr;
	STextBlock* DetailsTitle = nullptr;
	SScrollBox* DetailsScrollBox = nullptr;
	SListView* ModuleListView = nullptr;
	SButton* AddModuleButton = nullptr;

	// Top Toolbar Widgets
	STextBlock* TitleText = nullptr;
	STextBlock* StatusText = nullptr;

	// Preview Viewport
	ViewerState* PreviewState = nullptr;
	FRect PreviewViewportRect = FRect(0, 0, 0, 0);
	ID3D11Device* Device = nullptr;

	// State
	bool bIsOpen = true;
	FString SelectedEmitterName;
	uint32 SelectedModuleIndex = 0;
	bool bIsPlaying = false;

	void CreateLayout();
	void CreateTopToolbar();
	void CreateLeftPanel();      // Emitter list
	void CreateCenterPanel();    // Preview viewport
	void CreateRightPanel();     // Module details

	// Event handlers
	void OnEmitterSelected(STreeNode* Node);
	void OnAddEmitter();
	void OnRemoveEmitter();
	void OnPlayClicked();
	void OnPauseClicked();
	void OnResetClicked();
	void OnModuleSelected(uint32 Index, const FString& ModuleName);
	void OnAddModule();
};
