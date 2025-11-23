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

	// Top Toolbar with controls
	CreateTopToolbar();
	RootLayout->AddSlot()
		.FixedHeight(40.0f)
		.SetPadding(5.0f, 0.0f)
		.AttachWidget(TopToolbar);

	// Main Content Area (Left + Right columns)
	MainContentArea = new SHorizontalBox();

	// Left Column: Viewport (top) + Details (bottom)
	CreateLeftColumn();
	MainContentArea->AddSlot()
		.FixedWidth(320.0f)  // Fixed width for left column
		.SetPadding(5.0f)
		.AttachWidget(LeftColumn);

	// Right Column: Emitters (top) + Curve Editor (bottom)
	CreateRightColumn();
	MainContentArea->AddSlot()
		.FillWidth(1.0f)  // Fill remaining space
		.SetPadding(5.0f)
		.AttachWidget(RightColumn);

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
	TitleText->SetFontSize(16.0f);
	TitleText->SetColor(0xFFFFFFFF);
	TopToolbar->AddSlot()
		.AutoWidth()
		.VAlignment(SHorizontalBox::VAlign_Center)
		.SetPadding(10.0f, 0.0f)
		.AttachWidget(TitleText);

	// Restart Sim Button
	RestartSimButton = new SButton();
	RestartSimButton->SetText("Restart Sim");
	RestartSimButton->SetSize(FVector2D(100.0f, 30.0f));
	RestartSimButton->OnClicked.Add([this]() {
		OnRestartSimClicked();
	});
	TopToolbar->AddSlot()
		.AutoWidth()
		.VAlignment(SHorizontalBox::VAlign_Center)
		.SetPadding(5.0f, 0.0f)
		.AttachWidget(RestartSimButton);

	// Restart Level Button
	RestartLevelButton = new SButton();
	RestartLevelButton->SetText("Restart Level");
	RestartLevelButton->SetSize(FVector2D(100.0f, 30.0f));
	RestartLevelButton->OnClicked.Add([this]() {
		OnRestartLevelClicked();
	});
	TopToolbar->AddSlot()
		.AutoWidth()
		.VAlignment(SHorizontalBox::VAlign_Center)
		.SetPadding(5.0f, 0.0f)
		.AttachWidget(RestartLevelButton);

	// Spacer
	auto LeftSpacer = new SPanel();
	TopToolbar->AddSlot()
		.FillWidth(1.0f)
		.AttachWidget(LeftSpacer);

	// Play Button
	PlayButton = new SButton();
	PlayButton->SetText("Play");
	PlayButton->SetSize(FVector2D(80.0f, 30.0f));
	PlayButton->OnClicked.Add([this]() {
		OnPlayClicked();
	});
	TopToolbar->AddSlot()
		.AutoWidth()
		.VAlignment(SHorizontalBox::VAlign_Center)
		.SetPadding(5.0f, 0.0f)
		.AttachWidget(PlayButton);

	// Pause Button
	PauseButton = new SButton();
	PauseButton->SetText("Pause");
	PauseButton->SetSize(FVector2D(80.0f, 30.0f));
	PauseButton->OnClicked.Add([this]() {
		OnPauseClicked();
	});
	TopToolbar->AddSlot()
		.AutoWidth()
		.VAlignment(SHorizontalBox::VAlign_Center)
		.SetPadding(5.0f, 0.0f)
		.AttachWidget(PauseButton);

	// Reset Button
	ResetButton = new SButton();
	ResetButton->SetText("Reset");
	ResetButton->SetSize(FVector2D(80.0f, 30.0f));
	ResetButton->OnClicked.Add([this]() {
		OnResetClicked();
	});
	TopToolbar->AddSlot()
		.AutoWidth()
		.VAlignment(SHorizontalBox::VAlign_Center)
		.SetPadding(5.0f, 0.0f)
		.AttachWidget(ResetButton);

	// Spacer
	auto RightSpacer = new SPanel();
	TopToolbar->AddSlot()
		.FillWidth(1.0f)
		.AttachWidget(RightSpacer);

	// Status Text
	StatusText = new STextBlock("Ready");
	StatusText->SetColor(0xFFAAAAAA);
	TopToolbar->AddSlot()
		.AutoWidth()
		.VAlignment(SHorizontalBox::VAlign_Center)
		.SetPadding(10.0f, 0.0f)
		.AttachWidget(StatusText);
}

void SParticleEditorWindow::CreateLeftColumn()
{
	LeftColumn = new SVerticalBox();

	// Viewport Panel (top half)
	CreateViewportPanel();
	LeftColumn->AddSlot()
		.FillHeight(1.0f)
		.SetPadding(0.0f)
		.AttachWidget(ViewportPanel);

	// Details Panel (bottom half)
	CreateDetailsPanel();
	LeftColumn->AddSlot()
		.FillHeight(1.0f)
		.SetPadding(0.0f, 5.0f, 0.0f, 0.0f)
		.AttachWidget(DetailsPanel);
}

void SParticleEditorWindow::CreateViewportPanel()
{
	ViewportPanel = new SVerticalBox();

	// Viewport title
	auto ViewportTitle = new STextBlock("Viewport");
	ViewportTitle->SetFontSize(12.0f);
	ViewportTitle->SetColor(0xFFFFCC64);
	ViewportPanel->AddSlot()
		.AutoHeight()
		.SetPadding(5.0f)
		.AttachWidget(ViewportTitle);

	// Viewport Area
	ViewportPlaceholder = new SViewportPanel("ParticleEditorViewport");
	ViewportPanel->AddSlot()
		.FillHeight(1.0f)
		.SetPadding(5.0f)
		.AttachWidget(ViewportPlaceholder);
}

void SParticleEditorWindow::CreateDetailsPanel()
{
	DetailsPanel = new SVerticalBox();

	// Title bar
	DetailsTitle = new STextBlock("Details");
	DetailsTitle->SetFontSize(12.0f);
	DetailsTitle->SetColor(0xFFFFCC64);
	DetailsPanel->AddSlot()
		.AutoHeight()
		.SetPadding(5.0f)
		.AttachWidget(DetailsTitle);

	// Scrollable details area
	DetailsScrollBox = new SScrollBox();

	// Add some sample detail items
	const char* detailNames[] = {"Cone", "Angle", "Velocity", "Direction", "In World Space",
	                              "Apply Owner Scale", "B 3DDraw Mode", "Module Editor Color"};
	for (int i = 0; i < 8; i++)
	{
		auto DetailItem = new STextBlock(detailNames[i]);
		DetailItem->SetFontSize(10.0f);
		DetailItem->SetColor(0xFFCCCCCC);
		DetailsScrollBox->AddChild(DetailItem);
	}

	DetailsPanel->AddSlot()
		.FillHeight(1.0f)
		.SetPadding(5.0f)
		.AttachWidget(DetailsScrollBox);
}

void SParticleEditorWindow::CreateRightColumn()
{
	RightColumn = new SVerticalBox();

	// Emitters Panel (top - takes more space)
	CreateEmittersPanel();
	RightColumn->AddSlot()
		.FillHeight(2.0f)
		.SetPadding(0.0f)
		.AttachWidget(EmittersPanel);

	// Curve Editor Panel (bottom - takes less space)
	CreateCurveEditorPanel();
	RightColumn->AddSlot()
		.FillHeight(1.0f)
		.SetPadding(0.0f, 5.0f, 0.0f, 0.0f)
		.AttachWidget(CurveEditorPanel);
}

void SParticleEditorWindow::CreateEmittersPanel()
{
	EmittersPanel = new SVerticalBox();

	// Title bar
	EmittersTitle = new STextBlock("Emitters");
	EmittersTitle->SetFontSize(12.0f);
	EmittersTitle->SetColor(0xFFFFCC64);
	EmittersPanel->AddSlot()
		.AutoHeight()
		.SetPadding(5.0f)
		.AttachWidget(EmittersTitle);

	// GPU Sprites label
	auto GPUSpritesLabel = new STextBlock("GPU Sprites");
	GPUSpritesLabel->SetFontSize(10.0f);
	GPUSpritesLabel->SetColor(0xFFAAAAAA);
	EmittersPanel->AddSlot()
		.AutoHeight()
		.SetPadding(5.0f, 2.0f)
		.AttachWidget(GPUSpritesLabel);

	// Scrollable emitters row (horizontal list of emitters)
	auto EmittersScrollArea = new SScrollBox();
	EmittersListRow = new SHorizontalBox();

	// Add sample emitters - smoke, blood1, dirt, drops, Particle Emitter
	const char* emitterNames[] = {"smoke", "blood1", "dirt", "drops", "Particle Emitter"};
	for (int i = 0; i < 5; i++)
	{
		auto EmitterButton = new SButton();
		EmitterButton->SetText(emitterNames[i]);
		EmitterButton->SetSize(FVector2D(100.0f, 60.0f));
		EmittersListRow->AddSlot()
			.FixedWidth(100.0f)
			.SetPadding(3.0f)
			.AttachWidget(EmitterButton);
	}

	EmittersScrollArea->AddChild(EmittersListRow);

	EmittersPanel->AddSlot()
		.FixedHeight(70.0f)
		.SetPadding(5.0f)
		.AttachWidget(EmittersScrollArea);

	// Scrollable emitters details area (module list)
	EmittersScrollBox = new SScrollBox();

	// Add some sample module items
	const char* moduleNames[] = {"Required", "Spawn", "Lifetime", "Initial Size", "Color Over Life",
	                              "Initial Rotation", "Velocity Cone", "Drag"};
	for (int i = 0; i < 8; i++)
	{
		auto ModuleItem = new STextBlock(moduleNames[i]);
		ModuleItem->SetFontSize(11.0f);
		ModuleItem->SetColor(0xFFDDDDDD);
		EmittersScrollBox->AddChild(ModuleItem);
	}

	EmittersPanel->AddSlot()
		.FillHeight(1.0f)
		.SetPadding(5.0f)
		.AttachWidget(EmittersScrollBox);
}

void SParticleEditorWindow::CreateCurveEditorPanel()
{
	CurveEditorPanel = new SVerticalBox();

	// Title bar
	CurveEditorTitle = new STextBlock("Curve Editor");
	CurveEditorTitle->SetFontSize(12.0f);
	CurveEditorTitle->SetColor(0xFFFFCC64);
	CurveEditorPanel->AddSlot()
		.AutoHeight()
		.SetPadding(5.0f)
		.AttachWidget(CurveEditorTitle);

	// Curve editor placeholder (fills remaining space)
	CurveEditorPlaceholder = new SPanel();
	CurveEditorPanel->AddSlot()
		.FillHeight(1.0f)
		.SetPadding(5.0f)
		.AttachWidget(CurveEditorPlaceholder);
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

void SParticleEditorWindow::OnRestartSimClicked()
{
	StatusText->SetText("Simulation Restarted");
	UE_LOG("Restart Sim clicked");
	// TODO: Implement simulation restart logic
}

void SParticleEditorWindow::OnRestartLevelClicked()
{
	StatusText->SetText("Level Restarted");
	UE_LOG("Restart Level clicked");
	// TODO: Implement level restart logic
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
