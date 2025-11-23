#include "pch.h"
#include "ParticleEditorWindow.h"
#include "Source/Slate/Core/Panels/SVerticalBox.h"
#include "Source/Slate/Core/Panels/SHorizontalBox.h"
#include "Source/Slate/Core/Panels/SScrollBox.h"
#include "Source/Slate/Core/Panels/SPanel.h"
#include "Source/Slate/Core/Widgets/SButton.h"
#include "Source/Slate/Core/Widgets/STextBlock.h"
#include "Source/Slate/Core/Widgets/STreeView.h"
#include "Source/Slate/Core/Widgets/SListView.h"
#include "ImGui/imgui.h"

IMPLEMENT_CLASS(UParticleEditorWindow)

UParticleEditorWindow::UParticleEditorWindow()
{
	FUIWindowConfig Config;
	Config.WindowTitle = "Particle Editor";
	Config.DefaultSize = ImVec2(1200, 800);
	SetConfig(Config);
}

UParticleEditorWindow::~UParticleEditorWindow()
{
	if (RootLayout)
	{
		delete RootLayout;
		RootLayout = nullptr;
	}
}

void UParticleEditorWindow::Initialize()
{
	UUIWindow::Initialize();
	CreateLayout();
}

void UParticleEditorWindow::CreateLayout()
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
		.FixedWidth(250.0f)
		.SetPadding(0.0f)
		.AttachWidget(LeftPanel);
	//
	// // CENTER: Preview Viewport (Fills remaining width)
	// CreateCenterPanel();
	// MainContentArea->AddSlot()
	// 	.FillWidth(1.0f)
	// 	.SetPadding(0.0f)
	// 	.AttachWidget(CenterPanel);
	//
	// // RIGHT: Module Details Panel (Fixed width 300px)
	// CreateRightPanel();
	// MainContentArea->AddSlot()
	// 	.FixedWidth(300.0f)
	// 	.SetPadding(0.0f)
	// 	.AttachWidget(RightPanel);

	RootLayout->AddSlot()
		.FillHeight(1.0f)
		.SetPadding(0.0f)
		.AttachWidget(MainContentArea);
}

void UParticleEditorWindow::CreateTopToolbar()
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

void UParticleEditorWindow::CreateLeftPanel()
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

void UParticleEditorWindow::CreateCenterPanel()
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
	// For now, use placeholder
	ViewportPlaceholder = new SPanel();
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

void UParticleEditorWindow::CreateRightPanel()
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

void UParticleEditorWindow::RenderWidget() const
{
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

		RootLayout->OnRender();
	}
}

// Event Handlers

void UParticleEditorWindow::OnEmitterSelected(STreeNode* Node)
{
	if (Node)
	{
		SelectedEmitterName = Node->GetLabel();
		StatusText->SetText("Selected: " + SelectedEmitterName);
		UE_LOG("Emitter selected: %s", SelectedEmitterName.c_str());
	}
}

void UParticleEditorWindow::OnAddEmitter()
{
	// Add root node to tree
	static uint32 EmitterCount = 0;
	EmitterCount++;
	FString EmitterName = "Emitter_" + std::to_string(EmitterCount);

	EmitterTreeView->AddRootNode(EmitterName);
	StatusText->SetText("Added: " + EmitterName);
	UE_LOG("Added emitter: %s", EmitterName.c_str());
}

void UParticleEditorWindow::OnRemoveEmitter()
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

void UParticleEditorWindow::OnPlayClicked()
{
	bIsPlaying = true;
	StatusText->SetText("Playing");
	UE_LOG("Play clicked");
}

void UParticleEditorWindow::OnPauseClicked()
{
	bIsPlaying = false;
	StatusText->SetText("Paused");
	UE_LOG("Pause clicked");
}

void UParticleEditorWindow::OnResetClicked()
{
	bIsPlaying = false;
	StatusText->SetText("Reset");
	UE_LOG("Reset clicked");
}

void UParticleEditorWindow::OnModuleSelected(uint32 Index, const FString& ModuleName)
{
	SelectedModuleIndex = Index;
	StatusText->SetText("Module: " + ModuleName);
	UE_LOG("Module selected: %s (Index: %d)", ModuleName.c_str(), Index);
}

void UParticleEditorWindow::OnAddModule()
{
	// Add module to list
	static uint32 ModuleCount = 0;
	ModuleCount++;
	FString ModuleName = "Module_" + std::to_string(ModuleCount);

	ModuleListView->AddItem(ModuleName);
	StatusText->SetText("Added module: " + ModuleName);
	UE_LOG("Added module: %s", ModuleName.c_str());
}
