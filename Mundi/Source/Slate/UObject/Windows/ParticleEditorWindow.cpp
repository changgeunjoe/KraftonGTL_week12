#include "pch.h"
#include "ParticleEditorWindow.h"
#include "Source/Runtime/Engine/SkeletalViewer/SkeletalViewerBootstrap.h"
#include "Source/Runtime/Engine/SkeletalViewer/ViewerState.h"
#include "Source/Runtime/Renderer/FViewport.h"
#include "Source/Runtime/Core/Object/Property.h"
#include "Source/Runtime/Engine/Particle/ParticleModule.h"
#include "Source/Runtime/Engine/Particle/ParticleSystem.h"
#include "Source/Runtime/Engine/Particle/ParticleEmitter.h"
#include "Source/Runtime/Engine/Particle/ParticleLODLevel.h"
#include "Source/Runtime/Engine/Particle/ParticleModuleRequired.h"
#include "Source/Runtime/Engine/Particle/ParticleModuleSpawn.h"
#include "Source/Runtime/Engine/Particle/ParticleModuleLifetime.h"
#include "Source/Slate/UObject/Widgets/ParticleModuleDetailWidget.h"
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

	// Destroy Detail Widget
	if (DetailWidget)
	{
		delete DetailWidget;
		DetailWidget = nullptr;
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

	// Detail Widget 생성 및 초기화
	DetailWidget = NewObject<UParticleModuleDetailWidget>();
	DetailWidget->Initialize();

	// 테스트용 파티클 시스템 생성
	CreateTestParticleSystem();

	// Detail Widget에 파티클 시스템 설정
	if (DetailWidget)
	{
		DetailWidget->SetParticleSystem(EditingParticleSystem);
	}

	return true;
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
		ImGui::SetNextWindowSize(ImVec2(1600.0f, 900.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(160.0f, 90.0f), ImGuiCond_FirstUseEver);
		bFirstFrame = false;
	}

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;

	if (ImGui::Begin("Particle Editor", &bIsOpen, flags))
	{
		// 윈도우 Rect 업데이트 (마우스 입력 감지용)
		ImVec2 WinPos = ImGui::GetWindowPos();
		ImVec2 WinSize = ImGui::GetWindowSize();
		SetRect(WinPos.x, WinPos.y, WinPos.x + WinSize.x, WinPos.y + WinSize.y);

		// Top Toolbar (Menu Bar)
		RenderTopToolbar();

		// Main Content Area - 3 panels layout
		ImVec2 ContentRegionAvail = ImGui::GetContentRegionAvail();

		// Left panel (Viewport + Details) - wider for better visibility
		float LeftPanelWidth = 400.0f;
		ImGui::BeginChild("LeftPanel", ImVec2(LeftPanelWidth, 0), true, ImGuiWindowFlags_NoScrollbar);
		{
			float ViewportHeight = ContentRegionAvail.y * 0.5f - 5.0f;

			// Viewport Panel
			ImGui::BeginChild("ViewportChild", ImVec2(0, ViewportHeight), false);
			RenderViewportPanel();
			ImGui::EndChild();

			ImGui::Spacing();

			// Details Panel
			ImGui::BeginChild("DetailsChild", ImVec2(0, 0), false);
			RenderDetailsPanel();
			ImGui::EndChild();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		// Middle-Right area (Emitters + Curve Editor)
		ImGui::BeginChild("RightArea", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);
		{
			// Emitters Panel (top ~60%)
			float EmittersHeight = ContentRegionAvail.y * 0.6f - 5.0f;
			ImGui::BeginChild("EmittersPanel", ImVec2(0, EmittersHeight), true);
			RenderEmittersPanel();
			ImGui::EndChild();

			ImGui::Spacing();

			// Curve Editor Panel (bottom ~40%)
			ImGui::BeginChild("CurveEditorPanel", ImVec2(0, 0), true);
			RenderCurveEditorPanel();
			ImGui::EndChild();
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void SParticleEditorWindow::RenderTopToolbar()
{
	if (ImGui::BeginMenuBar())
	{
		// Restart Sim Button
		if (ImGui::Button("Restart Sim"))
		{
			OnRestartSimClicked();
		}

		// Restart Level Button
		if (ImGui::Button("Restart Level"))
		{
			OnRestartLevelClicked();
		}

		ImGui::SameLine();
		ImGui::Separator();
		ImGui::SameLine();

		// Undo button (placeholder)
		if (ImGui::Button("Undo"))
		{
			// TODO: Undo functionality
		}

		// Redo button (placeholder)
		if (ImGui::Button("Redo"))
		{
			// TODO: Redo functionality
		}

		ImGui::SameLine();
		ImGui::Separator();
		ImGui::SameLine();

		// Thumbnail button
		if (ImGui::Button("Thumbnail"))
		{
			// TODO: Thumbnail functionality
		}

		// Bounds button
		if (ImGui::Button("Bounds"))
		{
			// TODO: Bounds functionality
		}

		// Origin Axis
		if (ImGui::Button("Origin Axis"))
		{
			// TODO: Origin Axis toggle
		}

		// Background Color
		if (ImGui::Button("Background Color"))
		{
			// TODO: Background color picker
		}

		ImGui::SameLine();
		ImGui::Separator();
		ImGui::SameLine();

		// Regen LOD buttons
		if (ImGui::Button("Regen LOD"))
		{
			// TODO: Regen LOD
		}

		if (ImGui::Button("Regen LOD"))
		{
			// TODO: Regen LOD
		}

		if (ImGui::Button("Lowest LOD"))
		{
			// TODO: Lowest LOD
		}

		if (ImGui::Button("Lower LOD"))
		{
			// TODO: Lower LOD
		}

		if (ImGui::Button("Add LOD"))
		{
			// TODO: Add LOD
		}

		// LOD dropdown
		ImGui::SetNextItemWidth(60);
		const char* lodItems[] = { "LOD: 0" };
		static int currentLOD = 0;
		ImGui::Combo("##LOD", &currentLOD, lodItems, IM_ARRAYSIZE(lodItems));

		if (ImGui::Button("Add LOD"))
		{
			// TODO: Add LOD
		}

		if (ImGui::Button("Higher LOD"))
		{
			// TODO: Higher LOD
		}

		ImGui::EndMenuBar();
	}
}

void SParticleEditorWindow::RenderViewportPanel()
{
	// Viewport Header with tabs
	if (ImGui::BeginTabBar("ViewportTabs", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("Viewport"))
		{
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	// View/Time selector
	const char* viewModes[] = { "View", "Time" };
	static int currentViewMode = 0;
	ImGui::SetNextItemWidth(-1);
	ImGui::Combo("##ViewMode", &currentViewMode, viewModes, IM_ARRAYSIZE(viewModes));

	ImGui::Spacing();

	// Get available region for viewport
	ImVec2 ViewportSize = ImGui::GetContentRegionAvail();
	ImVec2 ViewportPos = ImGui::GetCursorScreenPos();

	// Update viewport rect
	PreviewViewportRect.Left = ViewportPos.x;
	PreviewViewportRect.Top = ViewportPos.y;
	PreviewViewportRect.Right = ViewportPos.x + ViewportSize.x;
	PreviewViewportRect.Bottom = ViewportPos.y + ViewportSize.y;

	// Draw viewport placeholder (dark background)
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	DrawList->AddRectFilled(
		ViewportPos,
		ImVec2(ViewportPos.x + ViewportSize.x, ViewportPos.y + ViewportSize.y),
		IM_COL32(20, 20, 25, 255)
	);

	// Advance cursor
	ImGui::Dummy(ViewportSize);
}

void SParticleEditorWindow::RenderDetailsPanel()
{
	// Detail Widget을 사용하여 렌더링
	if (DetailWidget)
	{
		DetailWidget->RenderWidget();
	}
}

void SParticleEditorWindow::RenderEmittersPanel()
{
	// Emitters header with tab
	if (ImGui::BeginTabBar("EmittersTabs", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("Emitters"))
		{
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::Spacing();

	// GPU Sprites label
	ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "GPU Sprites");

	ImGui::Spacing();

	// Module list with checkboxes
	struct ModuleInfo
	{
		const char* name;
		bool enabled;
		ImVec4 color;
	};

	// 각 이미터별 모듈 정보
	struct EmitterData
	{
		const char* name;
		int particleCount;
		ImVec4 headerColor;
		std::vector<ModuleInfo> modules;
	};

	EmitterData emitters[] = {
		{
			"smoke", 7, ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
			{
				{"Required", true, ImVec4(1.0f, 1.0f, 0.5f, 1.0f)},
				{"Spawn", true, ImVec4(1.0f, 0.5f, 0.5f, 1.0f)},
				{"Lifetime", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Initial Size", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Color Over Life", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Initial Rotation", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Initial Rotation Rate", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Size By Life", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Drag", true, ImVec4(1.0f, 0.5f, 0.0f, 1.0f)},
				{"Velocity Cone", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Drag", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)}
			}
		},
		{
			"blood1", 28, ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
			{
				{"Required", true, ImVec4(1.0f, 1.0f, 0.5f, 1.0f)},
				{"Spawn", true, ImVec4(1.0f, 0.5f, 0.5f, 1.0f)},
				{"Lifetime", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Initial Size", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Color Over Life", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"SubImage Index", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Pivot Offset", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Initial Rotation", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Size By Life", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Velocity Cone", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Drag", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)}
			}
		},
		{
			"dirt", 11, ImVec4(0.3f, 0.3f, 0.3f, 1.0f),
			{
				{"Required", true, ImVec4(1.0f, 1.0f, 0.5f, 1.0f)},
				{"Spawn", true, ImVec4(1.0f, 0.5f, 0.5f, 1.0f)},
				{"Lifetime", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Initial Size", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Size By Life", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Velocity Cone", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Pivot Offset", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Collision", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)}
			}
		},
		{
			"drops", 101, ImVec4(0.4f, 0.4f, 1.0f, 1.0f),
			{
				{"Required", true, ImVec4(1.0f, 1.0f, 0.5f, 1.0f)},
				{"Spawn", true, ImVec4(1.0f, 0.5f, 0.5f, 1.0f)},
				{"Lifetime", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Initial Size", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Color Over Life", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Const Acceleration", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Drag", true, ImVec4(1.0f, 0.5f, 0.0f, 1.0f)},
				{"Size By Speed", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Velocity Cone", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)},
				{"Collision", true, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)}
			}
		}
	};

	// 각 이미터를 수평으로 카드 형태로 표시
	ImGui::BeginChild("EmittersScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
	{
		for (int emitterIdx = 0; emitterIdx < 4; emitterIdx++)
		{
			const EmitterData& emitter = emitters[emitterIdx];

			ImGui::BeginGroup();
			{
				// 이미터 헤더 (이름과 스프라이트)
				ImGui::PushStyleColor(ImGuiCol_Header, emitter.headerColor);
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(emitter.headerColor.x * 1.2f, emitter.headerColor.y * 1.2f, emitter.headerColor.z * 1.2f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, emitter.headerColor);

				// 헤더 영역
				ImVec2 headerSize = ImVec2(180, 60);
				ImVec2 headerPos = ImGui::GetCursorScreenPos();
				ImDrawList* drawList = ImGui::GetWindowDrawList();

				// 헤더 배경
				drawList->AddRectFilled(
					headerPos,
					ImVec2(headerPos.x + headerSize.x, headerPos.y + headerSize.y),
					ImGui::ColorConvertFloat4ToU32(emitter.headerColor)
				);

				// 파티클 카운트 (오른쪽 상단)
				char countText[16];
				sprintf_s(countText, "%d", emitter.particleCount);
				ImVec2 textSize = ImGui::CalcTextSize(countText);
				ImGui::SetCursorScreenPos(ImVec2(headerPos.x + headerSize.x - textSize.x - 5, headerPos.y + 5));
				ImGui::Text("%s", countText);

				// 이미터 이름 (왼쪽 하단)
				ImGui::SetCursorScreenPos(ImVec2(headerPos.x + 5, headerPos.y + headerSize.y - 20));
				ImGui::Text("%s", emitter.name);

				// 스프라이트 썸네일 (중앙 우측)
				ImVec2 spritePos = ImVec2(headerPos.x + headerSize.x - 55, headerPos.y + 15);
				drawList->AddRectFilled(
					spritePos,
					ImVec2(spritePos.x + 45, spritePos.y + 35),
					IM_COL32(60, 60, 60, 255)
				);
				drawList->AddRect(
					spritePos,
					ImVec2(spritePos.x + 45, spritePos.y + 35),
					IM_COL32(120, 120, 120, 255)
				);

				ImGui::SetCursorScreenPos(ImVec2(headerPos.x, headerPos.y + headerSize.y));
				ImGui::Dummy(headerSize);

				ImGui::PopStyleColor(3);

				// "GPU Sprites" 라벨
				ImGui::Text("GPU Sprites");

				ImGui::Spacing();

				// 모듈 리스트
				ImGui::BeginChild(("Modules_" + std::to_string(emitterIdx)).c_str(), ImVec2(180, 300), true);
				{
					for (size_t moduleIdx = 0; moduleIdx < emitter.modules.size(); moduleIdx++)
					{
						const ModuleInfo& module = emitter.modules[moduleIdx];

						// 컬러 바
						ImVec2 pos = ImGui::GetCursorScreenPos();
						drawList->AddRectFilled(
							ImVec2(pos.x, pos.y),
							ImVec2(pos.x + 4, pos.y + ImGui::GetTextLineHeight()),
							ImGui::ColorConvertFloat4ToU32(module.color)
						);

						ImGui::Dummy(ImVec2(8, 0));
						ImGui::SameLine();

						// 체크박스
						bool enabled = module.enabled;
						ImGui::Checkbox(("##Mod_" + std::to_string(emitterIdx) + "_" + std::to_string(moduleIdx)).c_str(), &enabled);
						ImGui::SameLine();

						// 모듈 이름
						bool isSelected = (SelectedEmitterIndex == emitterIdx && SelectedModuleIndex == (int)moduleIdx);
						if (ImGui::Selectable((module.name + std::string("##Sel_") + std::to_string(emitterIdx) + "_" + std::to_string(moduleIdx)).c_str(),
							isSelected, ImGuiSelectableFlags_AllowItemOverlap))
						{
							SelectedEmitterIndex = emitterIdx;
							SelectedModuleIndex = (int)moduleIdx;
							SelectedModule = GetModuleFromCurrentEmitter((int)moduleIdx);

							if (DetailWidget)
							{
								DetailWidget->SetSelectedModule(SelectedModule);
							}
						}

						// 아이콘들 (오른쪽)
						if (isSelected)
						{
							ImGui::SameLine(ImGui::GetWindowWidth() - 40);
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
							if (ImGui::SmallButton(("3D##" + std::to_string(emitterIdx) + "_" + std::to_string(moduleIdx)).c_str())) {}
							ImGui::SameLine();
							if (ImGui::SmallButton(("C##" + std::to_string(emitterIdx) + "_" + std::to_string(moduleIdx)).c_str())) {}
							ImGui::PopStyleColor();
						}
					}
				}
				ImGui::EndChild();
			}
			ImGui::EndGroup();

			// 수평으로 배치
			if (emitterIdx < 3)
			{
				ImGui::SameLine();
			}
		}
	}
	ImGui::EndChild();
}

void SParticleEditorWindow::RenderCurveEditorPanel()
{
	// Curve Editor header with tab
	if (ImGui::BeginTabBar("CurveEditorTabs", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("Curve Editor"))
		{
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::Spacing();

	// Toolbar with icons
	ImGui::BeginGroup();
	{
		// Tool buttons
		if (ImGui::Button("H")) {} ImGui::SameLine();  // Horizontal
		if (ImGui::Button("V")) {} ImGui::SameLine();  // Vertical
		if (ImGui::Button("F")) {} ImGui::SameLine();  // Fit
		if (ImGui::Button("P")) {} ImGui::SameLine();  // Pan
		if (ImGui::Button("Z")) {} ImGui::SameLine();  // Zoom
		if (ImGui::Button("A")) {} ImGui::SameLine();  // Auto
		if (ImGui::Button("AC")) {} ImGui::SameLine(); // Auto Clamped
		if (ImGui::Button("U")) {} ImGui::SameLine();  // User
		if (ImGui::Button("B")) {} ImGui::SameLine();  // Break
		if (ImGui::Button("L")) {} ImGui::SameLine();  // Linear
		if (ImGui::Button("C")) {} ImGui::SameLine();  // Constant
		if (ImGui::Button("FL")) {} ImGui::SameLine(); // Flatten
		if (ImGui::Button("ST")) {} ImGui::SameLine(); // Straighten
		if (ImGui::Button("SA")) {} ImGui::SameLine(); // Show All
		if (ImGui::Button("CR")) {} ImGui::SameLine(); // Create
		if (ImGui::Button("DEL")) {}                    // Delete
	}
	ImGui::EndGroup();

	ImGui::Spacing();

	// Current Tab dropdown
	const char* tabs[] = { "Default" };
	static int currentTab = 0;
	ImGui::SetNextItemWidth(200);
	ImGui::Text("Current Tab:");
	ImGui::SameLine();
	ImGui::Combo("##CurrentTab", &currentTab, tabs, IM_ARRAYSIZE(tabs));

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// Curve editor grid area
	ImVec2 CurveEditorSize = ImGui::GetContentRegionAvail();
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	ImVec2 CurveEditorPos = ImGui::GetCursorScreenPos();

	// Background
	DrawList->AddRectFilled(
		CurveEditorPos,
		ImVec2(CurveEditorPos.x + CurveEditorSize.x, CurveEditorPos.y + CurveEditorSize.y),
		IM_COL32(45, 45, 50, 255)
	);

	// Grid lines (vertical)
	float gridSpacing = 50.0f;
	for (float x = 0; x < CurveEditorSize.x; x += gridSpacing)
	{
		DrawList->AddLine(
			ImVec2(CurveEditorPos.x + x, CurveEditorPos.y),
			ImVec2(CurveEditorPos.x + x, CurveEditorPos.y + CurveEditorSize.y),
			IM_COL32(70, 70, 75, 255)
		);
	}

	// Grid lines (horizontal)
	for (float y = 0; y < CurveEditorSize.y; y += gridSpacing)
	{
		DrawList->AddLine(
			ImVec2(CurveEditorPos.x, CurveEditorPos.y + y),
			ImVec2(CurveEditorPos.x + CurveEditorSize.x, CurveEditorPos.y + y),
			IM_COL32(70, 70, 75, 255)
		);
	}

	// Center axes (darker)
	float centerX = CurveEditorPos.x + CurveEditorSize.x * 0.5f;
	float centerY = CurveEditorPos.y + CurveEditorSize.y * 0.5f;
	DrawList->AddLine(
		ImVec2(centerX, CurveEditorPos.y),
		ImVec2(centerX, CurveEditorPos.y + CurveEditorSize.y),
		IM_COL32(100, 100, 105, 255)
	);
	DrawList->AddLine(
		ImVec2(CurveEditorPos.x, centerY),
		ImVec2(CurveEditorPos.x + CurveEditorSize.x, centerY),
		IM_COL32(100, 100, 105, 255)
	);

	// Axis labels
	DrawList->AddText(
		ImVec2(CurveEditorPos.x + 5, CurveEditorPos.y + 5),
		IM_COL32(200, 200, 200, 255),
		"0.50"
	);
	DrawList->AddText(
		ImVec2(CurveEditorPos.x + 5, CurveEditorPos.y + CurveEditorSize.y - 20),
		IM_COL32(200, 200, 200, 255),
		"-0.50"
	);
	DrawList->AddText(
		ImVec2(CurveEditorPos.x + 5, centerY - 10),
		IM_COL32(200, 200, 200, 255),
		"0.00"
	);

	// Bottom axis labels
	DrawList->AddText(
		ImVec2(CurveEditorPos.x + 5, CurveEditorPos.y + CurveEditorSize.y - 35),
		IM_COL32(200, 200, 200, 255),
		"0.05"
	);
	DrawList->AddText(
		ImVec2(CurveEditorPos.x + CurveEditorSize.x - 40, CurveEditorPos.y + CurveEditorSize.y - 35),
		IM_COL32(200, 200, 200, 255),
		"0.95"
	);

	ImGui::Dummy(CurveEditorSize);
}

// Event Handlers

void SParticleEditorWindow::OnPlayClicked()
{
	bIsPlaying = true;
	StatusMessage = "Playing";
	UE_LOG("Play clicked");
}

void SParticleEditorWindow::OnPauseClicked()
{
	bIsPlaying = false;
	StatusMessage = "Paused";
	UE_LOG("Pause clicked");
}

void SParticleEditorWindow::OnResetClicked()
{
	bIsPlaying = false;
	StatusMessage = "Reset";
	UE_LOG("Reset clicked");
}

void SParticleEditorWindow::OnRestartSimClicked()
{
	StatusMessage = "Simulation Restarted";
	UE_LOG("Restart Sim clicked");
	// TODO: Implement simulation restart logic
}

void SParticleEditorWindow::OnRestartLevelClicked()
{
	StatusMessage = "Level Restarted";
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

void SParticleEditorWindow::SetParticleSystem(UParticleSystem* InParticleSystem)
{
	EditingParticleSystem = InParticleSystem;
	SelectedModule = nullptr;
	SelectedModuleIndex = -1;
	SelectedEmitterIndex = 0;
}

UParticleModule* SParticleEditorWindow::GetModuleFromCurrentEmitter(int32 ModuleIndex)
{
	if (!EditingParticleSystem)
		return nullptr;

	if (SelectedEmitterIndex < 0 || SelectedEmitterIndex >= EditingParticleSystem->Emitters.Num())
		return nullptr;

	UParticleEmitter* Emitter = EditingParticleSystem->Emitters[SelectedEmitterIndex];
	if (!Emitter || Emitter->LODLevels.Num() == 0)
		return nullptr;

	UParticleLODLevel* LODLevel = Emitter->LODLevels[0];
	if (!LODLevel || ModuleIndex < 0 || ModuleIndex >= LODLevel->Modules.Num())
		return nullptr;

	return LODLevel->Modules[ModuleIndex];
}

void SParticleEditorWindow::CreateTestParticleSystem()
{
	// 테스트용 파티클 시스템 생성
	EditingParticleSystem = NewObject<UParticleSystem>();
	EditingParticleSystem->UpdateTime_FPS = 60.0f;
	EditingParticleSystem->UpdateTime_Delta = 1.0f / 60.0f;

	// 테스트용 이미터 생성
	UParticleEmitter* TestEmitter = NewObject<UParticleEmitter>();
	TestEmitter->EmitterName = "TestEmitter";

	// LOD 레벨 생성
	UParticleLODLevel* LODLevel = NewObject<UParticleLODLevel>();

	// Required 모듈 생성 및 추가
	UParticleModuleRequired* RequiredModule = NewObject<UParticleModuleRequired>();
	RequiredModule->Material = nullptr;
	RequiredModule->bUseLocalSpace = false;
	RequiredModule->EmitterDuration = 1.0f;
	RequiredModule->EmitterDurationLow = 1.0f;
	RequiredModule->EmitterLoops = 0;
	RequiredModule->EmitterDelay = 0.0f;
	LODLevel->Modules.Add(RequiredModule);

	// Spawn 모듈 생성 및 추가
	UParticleModuleSpawn* SpawnModule = NewObject<UParticleModuleSpawn>();
	SpawnModule->Rate = 10.0f;
	SpawnModule->RateScale = 1.0f;
	LODLevel->Modules.Add(SpawnModule);

	// Lifetime 모듈 생성 및 추가
	UParticleModuleLifetime* LifetimeModule = NewObject<UParticleModuleLifetime>();
	LifetimeModule->Lifetime = 3.0f;
	LifetimeModule->LifetimeMin = 2.0f;
	LifetimeModule->bUseLifetimeRange = true;
	LODLevel->Modules.Add(LifetimeModule);

	// LOD 레벨을 이미터에 추가
	TestEmitter->LODLevels.Add(LODLevel);

	// 이미터를 파티클 시스템에 추가
	EditingParticleSystem->Emitters.Add(TestEmitter);

	UE_LOG("Test ParticleSystem created with %d emitters and %d modules",
		EditingParticleSystem->Emitters.Num(),
		LODLevel->Modules.Num());
}
