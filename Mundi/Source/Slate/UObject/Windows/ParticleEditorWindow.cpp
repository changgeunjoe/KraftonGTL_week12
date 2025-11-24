#include "pch.h"
#include "ParticleEditorWindow.h"
#include "Source/Runtime/Engine/ParticleViewer/ParticleViewerBootstrap.h"
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
#include "Source/Slate/UObject/Widgets/PropertyRenderer.h"
#include "ImGui/imgui.h"

SParticleEditorWindow::SParticleEditorWindow()
{
}

SParticleEditorWindow::~SParticleEditorWindow()
{
	// Destroy ViewerState
	if (PreviewState)
	{
		ParticleViewerBootstrap::DestroyViewerState(PreviewState);
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
	PreviewState = ParticleViewerBootstrap::CreateViewerState("Particle Preview", InWorld, InDevice);
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

	if (!EditingParticleSystem || EditingParticleSystem->Emitters.size() == 0)
	{
		ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No Particle System loaded");
		return;
	}

	// 각 이미터를 수평으로 카드 형태로 표시
	ImGui::BeginChild("EmittersScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
	{
		// 이미터별 컬러 (순환)
		ImVec4 emitterColors[] = {
			ImVec4(1.0f, 0.5f, 0.0f, 1.0f),  // 오렌지
			ImVec4(0.5f, 0.5f, 0.5f, 1.0f),  // 회색
			ImVec4(0.3f, 0.3f, 0.3f, 1.0f),  // 어두운 회색
			ImVec4(0.4f, 0.4f, 1.0f, 1.0f),  // 파란색
		};

		for (int emitterIdx = 0; emitterIdx <= EditingParticleSystem->Emitters.size(); emitterIdx++)
		{
			// 마지막은 "+ Add Emitter" 버튼
			if (emitterIdx == EditingParticleSystem->Emitters.size())
			{
				ImGui::BeginGroup();
				{
					ImVec2 addButtonSize = ImVec2(220, 60);
					ImVec2 addButtonPos = ImGui::GetCursorScreenPos();
					ImDrawList* drawList = ImGui::GetWindowDrawList();

					// 점선 테두리
					drawList->AddRect(
						addButtonPos,
						ImVec2(addButtonPos.x + addButtonSize.x, addButtonPos.y + addButtonSize.y),
						IM_COL32(100, 100, 100, 255),
						0.0f, 0, 2.0f
					);

					ImGui::SetCursorScreenPos(ImVec2(addButtonPos.x + addButtonSize.x / 2 - 50, addButtonPos.y + addButtonSize.y / 2 - 10));

					if (ImGui::Button("+ Add Emitter", ImVec2(100, 30)))
					{
						AddNewEmitter();
					}

					// 우클릭으로도 추가 가능
					if (ImGui::BeginPopupContextItem("AddEmitterCtx"))
					{
						if (ImGui::MenuItem("Add New Emitter"))
						{
							AddNewEmitter();
						}
						ImGui::EndPopup();
					}

					ImGui::SetCursorScreenPos(ImVec2(addButtonPos.x, addButtonPos.y + addButtonSize.y));
					ImGui::Dummy(addButtonSize);
				}
				ImGui::EndGroup();
				break;
			}

			UParticleEmitter* emitter = EditingParticleSystem->Emitters[emitterIdx];
			if (!emitter || emitter->LODLevels.size() == 0) continue;

			UParticleLODLevel* LODLevel = emitter->LODLevels[0];
			if (!LODLevel) continue;

			ImVec4 headerColor = emitterColors[emitterIdx % 4];

			ImGui::BeginGroup();
			{
				// 헤더 영역
				ImVec2 headerSize = ImVec2(220, 60);
				ImVec2 headerPos = ImGui::GetCursorScreenPos();
				ImDrawList* drawList = ImGui::GetWindowDrawList();

				// 선택 여부에 따라 테두리 표시
				bool isEmitterSelected = (SelectedEmitterIndex == emitterIdx && SelectedModuleIndex == -1);
				if (isEmitterSelected)
				{
					drawList->AddRect(
						ImVec2(headerPos.x - 2, headerPos.y - 2),
						ImVec2(headerPos.x + headerSize.x + 2, headerPos.y + headerSize.y + 2),
						IM_COL32(255, 255, 0, 255),
						0.0f, 0, 3.0f
					);
				}

				// 헤더 배경 (클릭 가능한 버튼으로)
				ImGui::PushStyleColor(ImGuiCol_Button, headerColor);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(headerColor.x * 1.2f, headerColor.y * 1.2f, headerColor.z * 1.2f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(headerColor.x * 0.8f, headerColor.y * 0.8f, headerColor.z * 0.8f, 1.0f));

				if (ImGui::Button(("##EmitterHeader_" + std::to_string(emitterIdx)).c_str(), headerSize))
				{
					// 이미터 선택
					SelectedEmitterIndex = emitterIdx;
					SelectedModuleIndex = -1;
					SelectedModule = nullptr;

					// 디테일 위젯에 이미터 설정
					if (DetailWidget)
					{
						DetailWidget->SetSelectedModule(nullptr);
						DetailWidget->SetSelectedEmitter(emitter);
					}
				}

				ImGui::PopStyleColor(3);

				// 텍스트와 이미지는 버튼 위에 오버레이
				ImVec2 currentPos = ImGui::GetCursorScreenPos();

				// 파티클 카운트 (오른쪽 상단)
				char countText[16];
				sprintf_s(countText, "%d", emitter->PeakActiveParticles);
				ImVec2 textSize = ImGui::CalcTextSize(countText);
				ImGui::SetCursorScreenPos(ImVec2(headerPos.x + headerSize.x - textSize.x - 5, headerPos.y + 5));
				ImGui::Text("%s", countText);

				// 이미터 이름 (왼쪽 하단)
				ImGui::SetCursorScreenPos(ImVec2(headerPos.x + 5, headerPos.y + headerSize.y - 20));
				FString emitterNameStr = emitter->GetEmitterName().ToString();
				ImGui::Text("%s", emitterNameStr.c_str());

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

				ImGui::SetCursorScreenPos(currentPos);

				// "GPU Sprites" 라벨
				ImGui::Text("GPU Sprites");

				ImGui::Spacing();

				// 모듈 리스트
				ImGui::BeginChild(("Modules_" + std::to_string(emitterIdx)).c_str(), ImVec2(220, 300), true);
				{
					// 모듈 컬러 매핑
					auto GetModuleColor = [](const char* className) -> ImVec4 {
						if (strstr(className, "Required")) return ImVec4(1.0f, 1.0f, 0.5f, 1.0f);
						if (strstr(className, "Spawn")) return ImVec4(1.0f, 0.5f, 0.5f, 1.0f);
						if (strstr(className, "Lifetime")) return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
						if (strstr(className, "Drag")) return ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
						return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
					};

					for (int moduleIdx = 0; moduleIdx < LODLevel->Modules.size(); moduleIdx++)
					{
						UParticleModule* module = LODLevel->Modules[moduleIdx];
						if (!module) continue;

						const char* moduleName = module->GetClass()->Name;
						ImVec4 moduleColor = GetModuleColor(moduleName);

						// 컬러 바
						ImVec2 pos = ImGui::GetCursorScreenPos();
						drawList->AddRectFilled(
							ImVec2(pos.x, pos.y),
							ImVec2(pos.x + 4, pos.y + ImGui::GetTextLineHeight()),
							ImGui::ColorConvertFloat4ToU32(moduleColor)
						);

						ImGui::Dummy(ImVec2(8, 0));
						ImGui::SameLine();

						// 체크박스
						bool enabled = module->bEnabled;
						if (ImGui::Checkbox(("##Mod_" + std::to_string(emitterIdx) + "_" + std::to_string(moduleIdx)).c_str(), &enabled))
						{
							module->bEnabled = enabled;
						}
						ImGui::SameLine();

						// 모듈 이름
						bool isSelected = (SelectedEmitterIndex == emitterIdx && SelectedModuleIndex == moduleIdx);
						if (ImGui::Selectable((std::string(moduleName) + "##Sel_" + std::to_string(emitterIdx) + "_" + std::to_string(moduleIdx)).c_str(),
							isSelected, ImGuiSelectableFlags_AllowItemOverlap))
						{
							SelectedEmitterIndex = emitterIdx;
							SelectedModuleIndex = moduleIdx;
							SelectedModule = module;

							if (DetailWidget)
							{
								DetailWidget->SetSelectedModule(SelectedModule);
							}
						}

						// 우클릭 컨텍스트 메뉴
						if (ImGui::BeginPopupContextItem(("ModuleCtx_" + std::to_string(emitterIdx) + "_" + std::to_string(moduleIdx)).c_str()))
						{
							if (ImGui::MenuItem("Delete Module"))
							{
								// TODO: 모듈 삭제
							}
							ImGui::EndPopup();
						}
					}

					// 빈 공간 우클릭으로 모듈 추가
					ImGui::Spacing();
					ImGui::Separator();
					if (ImGui::Selectable("+ Add Module", false, 0, ImVec2(ImGui::GetContentRegionAvail().x, 0)))
					{
						ImGui::OpenPopup(("AddModulePopup_" + std::to_string(emitterIdx)).c_str());
					}

					// 우클릭으로도 팝업 열기
					if (ImGui::BeginPopupContextItem(("AddModuleCtxItem_" + std::to_string(emitterIdx)).c_str()))
					{
						ShowAddModuleContextMenu(emitterIdx);
						ImGui::EndPopup();
					}

					// + Add Module 클릭시 팝업
					if (ImGui::BeginPopup(("AddModulePopup_" + std::to_string(emitterIdx)).c_str()))
					{
						ShowAddModuleContextMenu(emitterIdx);
						ImGui::EndPopup();
					}
				}
				ImGui::EndChild();
			}
			ImGui::EndGroup();

			// 수평으로 배치
			if (emitterIdx < EditingParticleSystem->Emitters.size())
			{
				ImGui::SameLine();
			}
		}

		// 빈 공간 우클릭으로 이미터 추가
		if (ImGui::BeginPopupContextWindow("EmittersAreaCtx", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Add New Emitter"))
			{
				AddNewEmitter();
			}
			ImGui::EndPopup();
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

	// 선택된 모듈이 없으면 안내 메시지 표시
	if (!SelectedModule)
	{
		ImGui::Spacing();
		ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No module selected");
		ImGui::Spacing();
		ImGui::TextWrapped("Select a module from the Emitters panel to edit its curve properties");
		return;
	}

	// 선택된 모듈의 이름 표시
	ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "Module: %s", SelectedModule->GetClass()->DisplayName);
	ImGui::Separator();
	ImGui::Spacing();

	// 선택된 모듈에서 커브 타입의 프로퍼티들을 찾아서 렌더링
	UClass* ModuleClass = SelectedModule->GetClass();
	if (ModuleClass && ModuleClass->Properties.size() > 0)
	{
		bool hasCurveProperties = false;

		for (const FProperty& Prop : ModuleClass->Properties)
		{
			// Curve 타입 프로퍼티만 표시
			if (Prop.Type == EPropertyType::Curve)
			{
				hasCurveProperties = true;

				// UPropertyRenderer를 사용하여 커브 프로퍼티 렌더링
				UPropertyRenderer::RenderCurveProperty(Prop, SelectedModule);

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
			}
		}

		// 커브 프로퍼티가 없으면 안내 메시지
		if (!hasCurveProperties)
		{
			ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "This module has no curve properties");
		}
	}
	else
	{
		ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No properties available");
	}
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
	LODLevel->Modules.Add(LifetimeModule);

	// LOD 레벨을 이미터에 추가
	TestEmitter->LODLevels.Add(LODLevel);

	// 이미터를 파티클 시스템에 추가
	EditingParticleSystem->Emitters.Add(TestEmitter);

	UE_LOG("Test ParticleSystem created with %d emitters and %d modules",
		EditingParticleSystem->Emitters.Num(),
		LODLevel->Modules.Num());
}

void SParticleEditorWindow::ShowAddModuleContextMenu(int32 EmitterIndex)
{
	if (!EditingParticleSystem || EmitterIndex >= EditingParticleSystem->Emitters.size())
		return;

	UParticleEmitter* emitter = EditingParticleSystem->Emitters[EmitterIndex];
	if (!emitter || emitter->LODLevels.size() == 0)
		return;

	UParticleLODLevel* LODLevel = emitter->LODLevels[0];

	ImGui::Text("Add Module");
	ImGui::Separator();

	// 리플렉션 시스템을 사용하여 모든 UParticleModule 서브클래스 찾기
	UClass* ParticleModuleClass = UParticleModule::StaticClass();

	// 등록된 모든 클래스를 순회
	const TArray<UClass*>& AllClasses = UClass::GetAllClasses();
	for (UClass* classType : AllClasses)
	{
		// UParticleModule의 서브클래스인지 확인
		if (classType && classType != ParticleModuleClass && classType->IsChildOf(ParticleModuleClass))
		{
			// Base 클래스나 추상 클래스는 제외
			const char* className = classType->Name;
			if (strstr(className, "Base") != nullptr)
				continue;

			// DisplayName이 있으면 사용, 없으면 클래스 이름 사용
			const char* displayName = classType->DisplayName ? classType->DisplayName : className;

			// 메뉴 아이템 생성
			if (ImGui::MenuItem(displayName))
			{
				// AddModule은 UClass*를 받아서 내부에서 객체를 생성함
				UParticleModule* newModule = LODLevel->AddModule(classType);
				if (newModule)
				{
					// 기본값 설정
					newModule->SetToSensibleDefaults(emitter);
					UE_LOG("Added %s module to emitter %d", displayName, EmitterIndex);
				}
			}
		}
	}
}

void SParticleEditorWindow::AddNewEmitter()
{
	if (!EditingParticleSystem)
		return;

	// AddEmitter는 UClass*를 받아서 내부에서 객체를 생성함
	UParticleEmitter* NewEmitter = EditingParticleSystem->AddEmitter(UParticleEmitter::StaticClass());

	if (NewEmitter)
	{
		// 이미터 이름 설정
		char emitterName[64];
		sprintf_s(emitterName, "Emitter_%zu", EditingParticleSystem->Emitters.size() - 1);
		NewEmitter->SetEmitterName(FName(emitterName));

		UE_LOG("New emitter added: %s (Total emitters: %zu)", emitterName, EditingParticleSystem->Emitters.size());
	}
}
