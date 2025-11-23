#pragma once
#include "UIWindow.h"

class SVerticalBox;
class SHorizontalBox;
class SScrollBox;
class SButton;
class STextBlock;
class STreeView;
class SListView;
class SPanel;

class STreeNode;

class UParticleEditorWindow : public UUIWindow
{
public:
	DECLARE_CLASS(UParticleEditorWindow, UUIWindow)

	UParticleEditorWindow();
	virtual ~UParticleEditorWindow();

	virtual void Initialize() override;
	virtual void RenderWidget() const override;

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
	SPanel* ViewportPlaceholder = nullptr;
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

	// State
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
