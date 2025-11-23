#include "pch.h"
#include "ParticleEditorWindow.h"
#include "Source/Slate/Core/Panels/SVerticalBox.h"
#include "Source/Slate/Core/Panels/SHorizontalBox.h"
#include "Source/Slate/Core/Panels/SScrollBox.h"
#include "Source/Slate/Core/Panels/SPanel.h"
#include "Source/Slate/Core/Panels/SViewportPanel.h"
#include "Source/Slate/Core/Widgets/SButton.h"
#include "Source/Slate/Core/Widgets/STextBlock.h"
#include "Source/Slate/Core/Widgets/STreeView.h"
#include "Source/Slate/Core/Widgets/SListView.h"
#include "Source/Runtime/Engine/SkeletalViewer/SkeletalViewerBootstrap.h"
#include "Source/Runtime/Engine/SkeletalViewer/ViewerState.h"
#include "Source/Runtime/Renderer/FViewport.h"
#include "ImGui/imgui.h"

SParticleEditorWindow::SParticleEditorWindow()
{
}

SParticleEditorWindow::~SParticleEditorWindow()
{
	// Destroy ViewerState
	if (PreviewState)
	{
		SkeletalViewerBootstrap::DestroyViewerState(PreviewState);
		PreviewState = nullptr;
	}

	if (RootLayout)
	{
		delete RootLayout;
		RootLayout = nullptr;
	}
}

bool SParticleEditorWindow::Initialize(float StartX, float StartY, float Width, float Height, UWorld* InWorld, ID3D11Device* InDevice)
{
	Device = InDevice;
	SetRect(StartX, StartY, StartX + Width, StartY + Height);
	bIsOpen = true;

	// Create ViewerState for preview viewport
	PreviewState = SkeletalViewerBootstrap::CreateViewerState("Particle Preview", InWorld, InDevice);
	if (!PreviewState)
	{
		UE_LOG("Failed to create ViewerState for Particle Editor");
		return false;
	}

	CreateLayout();
	return true;
}

void SParticleEditorWindow::CreateLayout()
{
	// Root layout
	RootLayout = new SVerticalBox();

	// Top Toolbar
	CreateTopToolbar();
	RootLayout->AddSlot()
		.FixedHeight(10.0f)
		.SetPadding(10.0f, 0.0f)
		.AttachWidget(TopToolbar);

	// Main Content Area (3 panels side by side)
	MainContentArea = new SHorizontalBox();

	// LEFT: Emitter List Panel (Fixed width 250px)
	CreateLeftPanel();
	MainContentArea->AddSlot()
		.FillWidth(1.0f)
		.SetPadding(0.0f)
		.AttachWidget(LeftPanel);

	// CENTER: Preview Viewport (Fills remaining width)
	CreateCenterPanel();
	MainContentArea->AddSlot()
		.FillWidth(1.0f)
		.SetPadding(0.0f)
		.AttachWidget(CenterPanel);

	// RIGHT: Module Details Panel (Fixed width 300px)
	CreateRightPanel();
	MainContentArea->AddSlot()
		.FillWidth(1.0f)
		.SetPadding(0.0f)
		.AttachWidget(RightPanel);

	RootLayout->AddSlot()
		.FillHeight(1.0f)
		.SetPadding(0.0f)
		.AttachWidget(MainContentArea);
}

void SParticleEditorWindow::CreateTopToolbar()
{
	TopToolbar = new SHorizontalBox();

	// Title
	TitleText = new STextBlock("Particle Editor");
	TitleText->SetFontSize(18.0f);
	TitleText->SetColor(0xFFFFFFFF);
	TopToolbar->AddSlot()
		.AutoWidth()
		.VAlignment(SHorizontalBox::VAlign_Center)
		.SetPadding(10.0f, 0.0f)
		.AttachWidget(TitleText);

	// Spacer
	auto Spacer = new SPanel();
	TopToolbar->AddSlot()
		.FillWidth(1.0f)
		.AttachWidget(Spacer);

	// Status Text
	StatusText = new STextBlock("Ready");
	StatusText->SetColor(0xFFAAAAAA);
	TopToolbar->AddSlot()
		.AutoWidth()
		.VAlignment(SHorizontalBox::VAlign_Center)
		.SetPadding(10.0f, 0.0f)
		.AttachWidget(StatusText);
}

void SParticleEditorWindow::CreateLeftPanel()
{
	LeftPanel = new SVerticalBox();

	// Title
	EmitterListTitle = new STextBlock("Emitters");
	EmitterListTitle->SetFontSize(14.0f);
	EmitterListTitle->SetColor(0xFFFFCC64);
	LeftPanel->AddSlot()
		.AutoHeight()
		.SetPadding(5.0f)
		.AttachWidget(EmitterListTitle);

	// Tree View (Fills panel)
	EmitterTreeView = new STreeView();
	EmitterTreeView->OnSelectionChanged.Add([this](STreeNode* Node) {
		OnEmitterSelected(Node);
	});
	LeftPanel->AddSlot()
		.FillHeight(1.0f)
		.SetPadding(5.0f)
		.AttachWidget(EmitterTreeView);

	// Button Row (Fixed height)
	EmitterButtonRow = new SHorizontalBox();

	AddEmitterButton = new SButton();
	AddEmitterButton->SetText("Add");
	AddEmitterButton->SetSize(FVector2D(115.0f, 30.0f));
	AddEmitterButton->OnClicked.Add([this]() {
		OnAddEmitter();
	});
	EmitterButtonRow->AddSlot()
		.FillWidth(1.0f)
		.SetPadding(2.0f)
		.AttachWidget(AddEmitterButton);

	RemoveEmitterButton = new SButton();
	RemoveEmitterButton->SetText("Remove");
	RemoveEmitterButton->SetSize(FVector2D(115.0f, 30.0f));
	RemoveEmitterButton->OnClicked.Add([this]() {
		OnRemoveEmitter();
	});
	EmitterButtonRow->AddSlot()
		.FillWidth(1.0f)
		.SetPadding(2.0f)
		.AttachWidget(RemoveEmitterButton);

	LeftPanel->AddSlot()
		.FixedHeight(40.0f)
		.SetPadding(5.0f)
		.AttachWidget(EmitterButtonRow);
}

void SParticleEditorWindow::CreateCenterPanel()
{
	CenterPanel = new SVerticalBox();

	// Viewport Title
	ViewportTitle = new STextBlock("Preview");
	ViewportTitle->SetFontSize(14.0f);
	ViewportTitle->SetColor(0xFFFFCC64);
	ViewportTitle->SetJustification(STextBlock::Center);
	CenterPanel->AddSlot()
		.AutoHeight()
		.SetPadding(5.0f)
		.AttachWidget(ViewportTitle);

	// Viewport Area (Fills center)
	ViewportPlaceholder = new SViewportPanel("ParticleEditorViewport");
	CenterPanel->AddSlot()
		.FillHeight(1.0f)
		.SetPadding(10.0f)
		.AttachWidget(ViewportPlaceholder);

	// Viewport Toolbar (Fixed height)
	ViewportToolbar = new SHorizontalBox();

	// Left spacer
	auto LeftSpacer = new SPanel();
	ViewportToolbar->AddSlot()
		.FillWidth(1.0f)
		.AttachWidget(LeftSpacer);

	PlayButton = new SButton();
	PlayButton->SetText("Play");
	PlayButton->SetSize(FVector2D(80.0f, 30.0f));
	PlayButton->OnClicked.Add([this]() {
		OnPlayClicked();
	});
	ViewportToolbar->AddSlot()
		.AutoWidth()
		.SetPadding(5.0f)
		.AttachWidget(PlayButton);

	PauseButton = new SButton();
	PauseButton->SetText("Pause");
	PauseButton->SetSize(FVector2D(80.0f, 30.0f));
	PauseButton->OnClicked.Add([this]() {
		OnPauseClicked();
	});
	ViewportToolbar->AddSlot()
		.AutoWidth()
		.SetPadding(5.0f)
		.AttachWidget(PauseButton);

	ResetButton = new SButton();
	ResetButton->SetText("Reset");
	ResetButton->SetSize(FVector2D(80.0f, 30.0f));
	ResetButton->OnClicked.Add([this]() {
		OnResetClicked();
	});
	ViewportToolbar->AddSlot()
		.AutoWidth()
		.SetPadding(5.0f)
		.AttachWidget(ResetButton);

	// Right spacer
	auto RightSpacer = new SPanel();
	ViewportToolbar->AddSlot()
		.FillWidth(1.0f)
		.AttachWidget(RightSpacer);

	CenterPanel->AddSlot()
		.FixedHeight(50.0f)
		.SetPadding(5.0f)
		.AttachWidget(ViewportToolbar);
}

void SParticleEditorWindow::CreateRightPanel()
{
	RightPanel = new SVerticalBox();

	// Title
	DetailsTitle = new STextBlock("Module Details");
	DetailsTitle->SetFontSize(14.0f);
	DetailsTitle->SetColor(0xFFFFCC64);
	RightPanel->AddSlot()
		.AutoHeight()
		.SetPadding(5.0f)
		.AttachWidget(DetailsTitle);

	// Scrollable module list
	DetailsScrollBox = new SScrollBox();
	ModuleListView = new SListView();
	ModuleListView->OnSelectionChanged.Add([this](uint32 Index, const FString& Item) {
		OnModuleSelected(Index, Item);
	});
	DetailsScrollBox->AddChild(ModuleListView);

	RightPanel->AddSlot()
		.FillHeight(1.0f)
		.SetPadding(5.0f)
		.AttachWidget(DetailsScrollBox);

	// Add Module Button
	AddModuleButton = new SButton();
	AddModuleButton->SetText("Add Module");
	AddModuleButton->SetSize(FVector2D(280.0f, 30.0f));
	AddModuleButton->OnClicked.Add([this]() {
		OnAddModule();
	});
	RightPanel->AddSlot()
		.FixedHeight(40.0f)
		.SetPadding(5.0f)
		.AttachWidget(AddModuleButton);
}

void SParticleEditorWindow::OnRender()
{
	if (!bIsOpen)
	{
		return;
	}

	// 첫 프레임에만 윈도우 크기 설정
	static bool bFirstFrame = true;
	if (bFirstFrame)
	{
		ImGui::SetNextWindowSize(ImVec2(1200.0f, 800.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(160.0f, 90.0f), ImGuiCond_FirstUseEver);
		bFirstFrame = false;
	}

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings;

	if (ImGui::Begin("Particle Editor", &bIsOpen, flags))
	{
		// 윈도우 Rect 업데이트 (마우스 입력 감지용)
		ImVec2 WinPos = ImGui::GetWindowPos();
		ImVec2 WinSize = ImGui::GetWindowSize();
		SetRect(WinPos.x, WinPos.y, WinPos.x + WinSize.x, WinPos.y + WinSize.y);

		if (RootLayout)
		{
			RootLayout->Invalidate();

			// Get available content region
			ImVec2 ContentMin = ImGui::GetCursorScreenPos();
			ImVec2 ContentRegionAvail = ImGui::GetContentRegionAvail();

			// Set layout bounds
			RootLayout->SetRect(
				ContentMin.x, ContentMin.y,
				ContentMin.x + ContentRegionAvail.x,
				ContentMin.y + ContentRegionAvail.y
			);

			// Render Slate layout
			RootLayout->OnRender();

			// Update preview viewport rect from ViewportPlaceholder widget
			if (ViewportPlaceholder)
			{
				PreviewViewportRect = ViewportPlaceholder->GetRect();
			}
		}
	}
	ImGui::End();
}

// Event Handlers

void SParticleEditorWindow::OnEmitterSelected(STreeNode* Node)
{
	if (Node)
	{
		SelectedEmitterName = Node->GetLabel();
		StatusText->SetText("Selected: " + SelectedEmitterName);
		UE_LOG("Emitter selected: %s", SelectedEmitterName.c_str());
	}
}

void SParticleEditorWindow::OnAddEmitter()
{
	// Add root node to tree
	static uint32 EmitterCount = 0;
	EmitterCount++;
	FString EmitterName = "Emitter_" + std::to_string(EmitterCount);

	EmitterTreeView->AddRootNode(EmitterName);
	StatusText->SetText("Added: " + EmitterName);
	UE_LOG("Added emitter: %s", EmitterName.c_str());
}

void SParticleEditorWindow::OnRemoveEmitter()
{
	if (!SelectedEmitterName.empty())
	{
		// In future, implement RemoveNode in STreeView
		StatusText->SetText("Remove not yet implemented");
		UE_LOG("Remove emitter requested: %s", SelectedEmitterName.c_str());
	}
	else
	{
		StatusText->SetText("No emitter selected");
	}
}

void SParticleEditorWindow::OnPlayClicked()
{
	bIsPlaying = true;
	StatusText->SetText("Playing");
	UE_LOG("Play clicked");
}

void SParticleEditorWindow::OnPauseClicked()
{
	bIsPlaying = false;
	StatusText->SetText("Paused");
	UE_LOG("Pause clicked");
}

void SParticleEditorWindow::OnResetClicked()
{
	bIsPlaying = false;
	StatusText->SetText("Reset");
	UE_LOG("Reset clicked");
}

void SParticleEditorWindow::OnModuleSelected(uint32 Index, const FString& ModuleName)
{
	SelectedModuleIndex = Index;
	StatusText->SetText("Module: " + ModuleName);
	UE_LOG("Module selected: %s (Index: %d)", ModuleName.c_str(), Index);
}

void SParticleEditorWindow::OnAddModule()
{
	// Add module to list
	static uint32 ModuleCount = 0;
	ModuleCount++;
	FString ModuleName = "Module_" + std::to_string(ModuleCount);

	ModuleListView->AddItem(ModuleName);
	StatusText->SetText("Added module: " + ModuleName);
	UE_LOG("Added module: %s", ModuleName.c_str());
}

void SParticleEditorWindow::OnMouseMove(FVector2D MousePos)
{
	if (!PreviewState || !PreviewState->Viewport) return;

	if (PreviewViewportRect.Contains(MousePos))
	{
		FVector2D LocalPos = MousePos - FVector2D(PreviewViewportRect.Left, PreviewViewportRect.Top);
		PreviewState->Viewport->ProcessMouseMove((int32)LocalPos.X, (int32)LocalPos.Y);
	}
}

void SParticleEditorWindow::OnMouseDown(FVector2D MousePos, uint32 Button)
{
	if (!PreviewState || !PreviewState->Viewport) return;

	if (PreviewViewportRect.Contains(MousePos))
	{
		FVector2D LocalPos = MousePos - FVector2D(PreviewViewportRect.Left, PreviewViewportRect.Top);
		PreviewState->Viewport->ProcessMouseButtonDown((int32)LocalPos.X, (int32)LocalPos.Y, (int32)Button);
	}
}

void SParticleEditorWindow::OnMouseUp(FVector2D MousePos, uint32 Button)
{
	if (!PreviewState || !PreviewState->Viewport) return;

	if (PreviewViewportRect.Contains(MousePos))
	{
		FVector2D LocalPos = MousePos - FVector2D(PreviewViewportRect.Left, PreviewViewportRect.Top);
		PreviewState->Viewport->ProcessMouseButtonUp((int32)LocalPos.X, (int32)LocalPos.Y, (int32)Button);
	}
}

void SParticleEditorWindow::OnRenderViewport()
{
	// Render the 3D preview viewport
	if (PreviewState && PreviewState->Viewport &&
		PreviewViewportRect.GetWidth() > 0 && PreviewViewportRect.GetHeight() > 0)
	{
		const uint32 NewStartX = static_cast<uint32>(PreviewViewportRect.Left);
		const uint32 NewStartY = static_cast<uint32>(PreviewViewportRect.Top);
		const uint32 NewWidth = static_cast<uint32>(PreviewViewportRect.Right - PreviewViewportRect.Left);
		const uint32 NewHeight = static_cast<uint32>(PreviewViewportRect.Bottom - PreviewViewportRect.Top);

		PreviewState->Viewport->Resize(NewStartX, NewStartY, NewWidth, NewHeight);
		PreviewState->Viewport->Render();
	}
}
