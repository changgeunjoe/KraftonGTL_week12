#include "pch.h"
#include "ParticleEditorWindow.h"
#include "Source/Runtime/Engine/ParticleViewer/ParticleViewerBootstrap.h"
#include "Source/Runtime/Engine/SkeletalViewer/ViewerState.h"
#include "Source/Runtime/Renderer/FViewport.h"
#include "Source/Runtime/Renderer/FViewportClient.h"
#include "Source/Runtime/Core/Object/Property.h"
#include "Source/Runtime/Engine/Particle/ParticleModule.h"
#include "Source/Runtime/Engine/Particle/ParticleSystem.h"
#include "Source/Runtime/Engine/Particle/ParticleEmitter.h"
#include "Source/Runtime/Engine/Particle/ParticleLODLevel.h"
#include "Source/Runtime/Engine/Particle/ParticleModuleRequired.h"
#include "Source/Runtime/Engine/Particle/ParticleModuleSpawn.h"
#include "Source/Runtime/Engine/Particle/ParticleModuleLifetime.h"
#include "Source/Runtime/Engine/Particle/ParticleModuleTypeDataBase.h"
#include "Source/Runtime/Engine/GameFramework/ParticleSystemActor.h"
#include "Source/Runtime/Engine/Components/ParticleSystemComponent.h"
#include "Source/Runtime/Engine/Distribution/Distributions.h"
#include "Source/Slate/UObject/Widgets/ParticleModuleDetailWidget.h"
#include "Source/Slate/UObject/Widgets/PropertyRenderer.h"
#include "ImGui/imgui.h"
#include "PlatformProcess.h"
#include <filesystem>
#include <algorithm>

#include "ParticleSpriteEmitter.h"

SParticleEditorWindow::SParticleEditorWindow()
{
}

SParticleEditorWindow::~SParticleEditorWindow()
{
	// Destroy Detail Widget (UObject이므로 DeleteObject 사용)
	if (DetailWidget)
	{
		DeleteObject(DetailWidget);
		DetailWidget = nullptr;
	}

	// Destroy ViewerState (World와 Viewport 포함)
	if (PreviewState)
	{
		ParticleViewerBootstrap::DestroyViewerState(PreviewState);
		PreviewState = nullptr;
	}

	// EditingParticleSystem은 소유권 문제로 인해 여기서 삭제하지 않음
	// (외부에서 전달된 것일 수 있으므로 ResourceManager가 관리)
	EditingParticleSystem = nullptr;
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

	// ImGui::Image 방식 렌더링을 위한 렌더 타겟 모드 활성화
	if (PreviewState->Viewport)
	{
		PreviewState->Viewport->SetUseRenderTarget(true);
	}

	// 초기 배경색 설정
	if (PreviewState->Client)
	{
		PreviewState->Client->SetBackgroundColor(ViewportBackgroundColor);
	}

	// Detail Widget 생성 및 초기화
	DetailWidget = NewObject<UParticleModuleDetailWidget>();
	DetailWidget->Initialize();

	// 프리뷰 액터 생성 (원점에 배치)
	if (PreviewState && PreviewState->World)
	{
		PreviewActor = PreviewState->World->SpawnActor<AParticleSystemActor>();
		if (PreviewActor)
		{
			PreviewActor->SetActorLocation(FVector::Zero());
		}
	}

	// 테스트용 파티클 시스템 생성
	CreateTestParticleSystem();

	// Detail Widget에 파티클 시스템 설정
	if (DetailWidget)
	{
		DetailWidget->SetParticleSystem(EditingParticleSystem);
	}

	// 프리뷰 액터에 파티클 시스템 설정
	UpdatePreviewActor();

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
		// File Menu
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Load", "Ctrl+O"))
			{
				LoadParticleSystemFromFile();
			}

			if (ImGui::MenuItem("Save", "Ctrl+S"))
			{
				SaveParticleSystemToFile();
			}

			if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
			{
				SaveParticleSystemToFileAs();
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Close"))
			{
				Close();
			}

			ImGui::EndMenu();
		}

		ImGui::Separator();

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
			bShowBackgroundColorPicker = !bShowBackgroundColorPicker;
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

	ImGui::SameLine();
	ImGui::Checkbox("Show Stats", &bShowStatsOverlay);

	// 배경색 피커 표시
	if (bShowBackgroundColorPicker)
	{
		ImGui::Spacing();
		ImGui::Text("Background Color");
		float color[4] = { ViewportBackgroundColor.R, ViewportBackgroundColor.G, ViewportBackgroundColor.B, ViewportBackgroundColor.A };
		if (ImGui::ColorEdit4("##BgColor", color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar))
		{
			ViewportBackgroundColor = FLinearColor(color[0], color[1], color[2], color[3]);
			// ViewportClient에 배경색 적용
			if (PreviewState && PreviewState->Client)
			{
				PreviewState->Client->SetBackgroundColor(ViewportBackgroundColor);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Close##BgColorClose"))
		{
			bShowBackgroundColorPicker = false;
		}
	}

	// Get available region for viewport
	ImVec2 ViewportPos = ImGui::GetCursorScreenPos();
	ImVec2 ViewportSize = ImGui::GetContentRegionAvail();

	// Update viewport rect for input handling
	PreviewViewportRect.Left = ViewportPos.x;
	PreviewViewportRect.Top = ViewportPos.y;
	PreviewViewportRect.Right = ViewportPos.x + ViewportSize.x;
	PreviewViewportRect.Bottom = ViewportPos.y + ViewportSize.y;
	PreviewViewportRect.UpdateMinMax();

	// Render viewport to texture
	OnRenderViewport();

	// Display the rendered texture using ImGui::Image
	if (PreviewState && PreviewState->Viewport)
	{
		ID3D11ShaderResourceView* SRV = PreviewState->Viewport->GetSRV();
		if (SRV)
		{
			ImGui::Image((void*)SRV, ViewportSize);
			// Update viewport hover state for Z-order aware input handling
			PreviewState->Viewport->SetViewportHovered(ImGui::IsItemHovered());

			if (bShowStatsOverlay)
			{
				RenderStatsOverlay(ViewportPos, ViewportSize);
			}
		}
		else
		{
			ImGui::Dummy(ViewportSize);
			PreviewState->Viewport->SetViewportHovered(false);
		}
	}
	else
	{
		ImGui::Dummy(ViewportSize);
	}
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
						if (strstr(className, "TypeData")) return ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
						if (strstr(className, "Velocity")) return ImVec4(0.5f, 0.8f, 1.0f, 1.0f);
						return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
					};

					// 렌더링할 모듈 람다 함수
					auto RenderModuleItem = [&](UParticleModule* module, int moduleIdx, bool isSpecialModule) {
						if (!module) return;

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

						// 우클릭 컨텍스트 메뉴 (특수 모듈은 삭제 불가)
						if (!isSpecialModule && ImGui::BeginPopupContextItem(("ModuleCtx_" + std::to_string(emitterIdx) + "_" + std::to_string(moduleIdx)).c_str()))
						{
							if (ImGui::MenuItem("Delete Module"))
							{
								// TODO: 모듈 삭제
							}
							ImGui::EndPopup();
						}
					};

					int moduleIdx = 0;

					// RequiredModule 먼저 표시
					if (LODLevel->RequiredModule)
					{
						RenderModuleItem(LODLevel->RequiredModule, moduleIdx++, true);
					}

					// SpawnModule 표시
					if (LODLevel->SpawnModule)
					{
						RenderModuleItem(LODLevel->SpawnModule, moduleIdx++, true);
					}

					// TypeDataModule 표시
					if (LODLevel->TypeDataModule)
					{
						RenderModuleItem(LODLevel->TypeDataModule, moduleIdx++, true);
					}

					// 일반 Modules 배열 표시
					for (int i = 0; i < LODLevel->Modules.size(); i++)
					{
						RenderModuleItem(LODLevel->Modules[i], moduleIdx++, false);
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
			// Curve 타입 프로퍼티
			if (Prop.Type == EPropertyType::Curve)
			{
				hasCurveProperties = true;
				UPropertyRenderer::RenderCurveProperty(Prop, SelectedModule);
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
			}
			// Bezier Distribution Float
			else if (Prop.Type == EPropertyType::RawDistributionFloat)
			{
				FRawDistributionFloat* DistFloat = Prop.GetValuePtr<FRawDistributionFloat>(SelectedModule);
				if (DistFloat && DistFloat->Distribution)
				{
					if (auto* BezierDist = Cast<UDistributionFloatBezier>(DistFloat->Distribution))
					{
						hasCurveProperties = true;
						RenderBezierFloatCurveEditor(Prop.Name, BezierDist);
						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();
					}
				}
			}
			// Bezier Distribution Vector
			else if (Prop.Type == EPropertyType::RawDistributionVector)
			{
				FRawDistributionVector* DistVector = Prop.GetValuePtr<FRawDistributionVector>(SelectedModule);
				if (DistVector && DistVector->Distribution)
				{
					if (auto* BezierDist = Cast<UDistributionVectorBezier>(DistVector->Distribution))
					{
						hasCurveProperties = true;
						RenderBezierVectorCurveEditor(Prop.Name, BezierDist);
						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();
					}
				}
			}
		}

		// 커브 프로퍼티가 없으면 안내 메시지
		if (!hasCurveProperties)
		{
			ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "This module has no curve properties");
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Tip: Set a Distribution to 'Bezier' type to enable curve editing");
		}
	}
	else
	{
		ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No properties available");
	}
}

void SParticleEditorWindow::RenderBezierFloatCurveEditor(const char* PropName, UDistributionFloatBezier* BezierDist)
{
	if (!BezierDist) return;

	ImGui::Text("%s (Bezier Float)", PropName);

	// 드래그 상태 (static으로 유지)
	static int draggingPoint = -1; // -1: 없음, 0-3: P0-P3
	static FString draggingPropName;

	// 커브 에디터 영역 크기
	ImVec2 canvasSize = ImVec2(ImGui::GetContentRegionAvail().x, 150);
	ImVec2 canvasPos = ImGui::GetCursorScreenPos();
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	// 배경
	drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), IM_COL32(30, 30, 30, 255));
	drawList->AddRect(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), IM_COL32(80, 80, 80, 255));

	// 그리드 라인
	int gridLines = 10;
	for (int i = 1; i < gridLines; i++)
	{
		float x = canvasPos.x + (canvasSize.x * i / gridLines);
		float y = canvasPos.y + (canvasSize.y * i / gridLines);
		drawList->AddLine(ImVec2(x, canvasPos.y), ImVec2(x, canvasPos.y + canvasSize.y), IM_COL32(50, 50, 50, 255));
		drawList->AddLine(ImVec2(canvasPos.x, y), ImVec2(canvasPos.x + canvasSize.x, y), IM_COL32(50, 50, 50, 255));
	}

	// Y축 값 범위 (0 ~ 1 고정)
	float minVal = 0.0f;
	float maxVal = 1.0f;

	// X축 시간 범위
	float minTime = BezierDist->MinInput;
	float maxTime = BezierDist->MaxInput;
	if (maxTime - minTime < 0.1f) maxTime = minTime + 1.0f;

	// 좌표 변환 람다 (X: 시간, Y: 값)
	auto ToScreen = [&](float t, float v) -> ImVec2 {
		float x = canvasPos.x + ((t - minTime) / (maxTime - minTime)) * canvasSize.x;
		float y = canvasPos.y + canvasSize.y - ((v - minVal) / (maxVal - minVal)) * canvasSize.y;
		return ImVec2(x, y);
	};

	// 스크린 좌표를 값으로 변환
	auto FromScreenY = [&](float screenY) -> float {
		float normalizedY = 1.0f - (screenY - canvasPos.y) / canvasSize.y;
		return minVal + normalizedY * (maxVal - minVal);
	};

	// 베지어 커브 그리기
	ImVec2 prevPoint = ToScreen(minTime, BezierDist->P0);
	int segments = 50;
	for (int i = 1; i <= segments; i++)
	{
		float t = (float)i / segments;
		float time = minTime + t * (maxTime - minTime);
		float value = BezierDist->GetValue(time);
		ImVec2 point = ToScreen(time, value);
		drawList->AddLine(prevPoint, point, IM_COL32(255, 200, 50, 255), 2.0f);
		prevPoint = point;
	}

	// 제어점 스크린 좌표 (시간 기준: P0=0%, P1=33%, P2=66%, P3=100%)
	float t0 = minTime;
	float t1 = minTime + (maxTime - minTime) * 0.33f;
	float t2 = minTime + (maxTime - minTime) * 0.66f;
	float t3 = maxTime;

	ImVec2 p0Screen = ToScreen(t0, BezierDist->P0);
	ImVec2 p1Screen = ToScreen(t1, BezierDist->P1);
	ImVec2 p2Screen = ToScreen(t2, BezierDist->P2);
	ImVec2 p3Screen = ToScreen(t3, BezierDist->P3);

	// 탄젠트 라인
	drawList->AddLine(p0Screen, p1Screen, IM_COL32(100, 100, 255, 150), 1.0f);
	drawList->AddLine(p2Screen, p3Screen, IM_COL32(100, 100, 255, 150), 1.0f);

	ImVec2 mousePos = ImGui::GetMousePos();
	float hitRadius = 10.0f;

	// 각 제어점에 대한 마우스 상호작용
	struct ControlPoint {
		ImVec2 screenPos;
		float* value;
		ImU32 color;
		ImU32 hoverColor;
		float radius;
		const char* label;
	};

	ControlPoint points[] = {
		{p0Screen, &BezierDist->P0, IM_COL32(50, 255, 50, 255), IM_COL32(100, 255, 100, 255), 8.0f, "P0"},
		{p1Screen, &BezierDist->P1, IM_COL32(100, 100, 255, 255), IM_COL32(150, 150, 255, 255), 6.0f, "P1"},
		{p2Screen, &BezierDist->P2, IM_COL32(100, 100, 255, 255), IM_COL32(150, 150, 255, 255), 6.0f, "P2"},
		{p3Screen, &BezierDist->P3, IM_COL32(255, 50, 50, 255), IM_COL32(255, 100, 100, 255), 8.0f, "P3"},
	};

	// 마우스 버튼 상태 확인
	bool isMouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
	bool isMouseClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);

	// 캔버스 영역 내에서만 처리
	bool isInCanvas = mousePos.x >= canvasPos.x && mousePos.x <= canvasPos.x + canvasSize.x &&
					  mousePos.y >= canvasPos.y && mousePos.y <= canvasPos.y + canvasSize.y;

	// 드래그 시작 검사
	if (isMouseClicked && isInCanvas && draggingPoint == -1)
	{
		for (int i = 0; i < 4; i++)
		{
			float dx = mousePos.x - points[i].screenPos.x;
			float dy = mousePos.y - points[i].screenPos.y;
			if (dx * dx + dy * dy <= hitRadius * hitRadius)
			{
				draggingPoint = i;
				draggingPropName = PropName;
				break;
			}
		}
	}

	// 드래그 중 값 업데이트
	if (draggingPoint >= 0 && draggingPropName == PropName && isMouseDown)
	{
		float newValue = FromScreenY(mousePos.y);
		// 값 범위 제한 (MinInput ~ MaxInput 범위)
		newValue = std::clamp(newValue, BezierDist->MinInput, BezierDist->MaxInput);
		*points[draggingPoint].value = newValue;
	}

	// 드래그 종료
	if (!isMouseDown && draggingPoint >= 0 && draggingPropName == PropName)
	{
		draggingPoint = -1;
		draggingPropName = "";
	}

	// 제어점 그리기 (호버/드래그 상태에 따라 색상 변경)
	for (int i = 0; i < 4; i++)
	{
		float dx = mousePos.x - points[i].screenPos.x;
		float dy = mousePos.y - points[i].screenPos.y;
		bool isHovered = (dx * dx + dy * dy <= hitRadius * hitRadius);
		bool isDragging = (draggingPoint == i && draggingPropName == PropName);

		ImU32 color = (isHovered || isDragging) ? points[i].hoverColor : points[i].color;
		float radius = (isHovered || isDragging) ? points[i].radius + 2.0f : points[i].radius;

		drawList->AddCircleFilled(points[i].screenPos, radius, color);

		// 드래그 중인 점에 외곽선 추가
		if (isDragging)
		{
			drawList->AddCircle(points[i].screenPos, radius + 2.0f, IM_COL32(255, 255, 255, 255), 0, 2.0f);
		}
	}

	// 범례
	ImVec2 legendPos = ImVec2(canvasPos.x + 5, canvasPos.y + 5);
	drawList->AddText(legendPos, IM_COL32(50, 255, 50, 255), "P0: Start");
	drawList->AddText(ImVec2(legendPos.x, legendPos.y + 15), IM_COL32(255, 50, 50, 255), "P3: End");
	drawList->AddText(ImVec2(legendPos.x, legendPos.y + 30), IM_COL32(150, 150, 150, 255), "Drag points to edit");

	// 값 표시 (오른쪽 상단)
	char valText[64];
	sprintf_s(valText, "Range: %.2f - %.2f", BezierDist->MinInput, BezierDist->MaxInput);
	ImVec2 valTextSize = ImGui::CalcTextSize(valText);
	drawList->AddText(ImVec2(canvasPos.x + canvasSize.x - valTextSize.x - 5, canvasPos.y + 5), IM_COL32(200, 200, 200, 255), valText);

	// 드래그 중인 점의 값 표시
	if (draggingPoint >= 0 && draggingPropName == PropName)
	{
		char dragText[32];
		sprintf_s(dragText, "%s: %.3f", points[draggingPoint].label, *points[draggingPoint].value);
		drawList->AddText(ImVec2(mousePos.x + 15, mousePos.y - 10), IM_COL32(255, 255, 255, 255), dragText);
	}

	// InvisibleButton으로 캔버스 영역 캡처 (다른 위젯과의 충돌 방지)
	ImGui::InvisibleButton(("##BezierCanvas_" + std::string(PropName)).c_str(), canvasSize);

	// 제어점 값 편집 슬라이더
	ImGui::Spacing();
	ImGui::Text("Input Range: %.2f - %.2f", BezierDist->MinInput, BezierDist->MaxInput);

	ImGui::PushItemWidth(150);
	ImGui::DragFloat(("P0 (Start)##" + std::string(PropName)).c_str(), &BezierDist->P0, 0.01f);
	ImGui::SameLine();
	ImGui::DragFloat(("P1 (StartTan)##" + std::string(PropName)).c_str(), &BezierDist->P1, 0.01f);
	ImGui::DragFloat(("P2 (EndTan)##" + std::string(PropName)).c_str(), &BezierDist->P2, 0.01f);
	ImGui::SameLine();
	ImGui::DragFloat(("P3 (End)##" + std::string(PropName)).c_str(), &BezierDist->P3, 0.01f);
	ImGui::PopItemWidth();
}

void SParticleEditorWindow::RenderBezierVectorCurveEditor(const char* PropName, UDistributionVectorBezier* BezierDist)
{
	if (!BezierDist) return;

	ImGui::Text("%s (Bezier Vector)", PropName);

	// 드래그 상태 (static으로 유지)
	static int draggingPointVec = -1;
	static FString draggingPropNameVec;
	static int draggingChannelVec = -1;

	// 채널 선택 (프로퍼티별로 고유하게)
	static std::map<std::string, int> channelSelections;
	int& selectedChannel = channelSelections[PropName];

	ImGui::RadioButton(("X##" + std::string(PropName)).c_str(), &selectedChannel, 0); ImGui::SameLine();
	ImGui::RadioButton(("Y##" + std::string(PropName)).c_str(), &selectedChannel, 1); ImGui::SameLine();
	ImGui::RadioButton(("Z##" + std::string(PropName)).c_str(), &selectedChannel, 2); ImGui::SameLine();
	ImGui::RadioButton(("All##" + std::string(PropName)).c_str(), &selectedChannel, 3);

	// 커브 에디터 영역 크기
	ImVec2 canvasSize = ImVec2(ImGui::GetContentRegionAvail().x, 150);
	ImVec2 canvasPos = ImGui::GetCursorScreenPos();
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	// 배경
	drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), IM_COL32(30, 30, 30, 255));
	drawList->AddRect(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), IM_COL32(80, 80, 80, 255));

	// 그리드 라인
	int gridLines = 10;
	for (int i = 1; i < gridLines; i++)
	{
		float x = canvasPos.x + (canvasSize.x * i / gridLines);
		float y = canvasPos.y + (canvasSize.y * i / gridLines);
		drawList->AddLine(ImVec2(x, canvasPos.y), ImVec2(x, canvasPos.y + canvasSize.y), IM_COL32(50, 50, 50, 255));
		drawList->AddLine(ImVec2(canvasPos.x, y), ImVec2(canvasPos.x + canvasSize.x, y), IM_COL32(50, 50, 50, 255));
	}

	// 채널별 색상
	ImU32 channelColors[] = {
		IM_COL32(255, 100, 100, 255), // X - Red
		IM_COL32(100, 255, 100, 255), // Y - Green
		IM_COL32(100, 100, 255, 255)  // Z - Blue
	};
	ImU32 channelHoverColors[] = {
		IM_COL32(255, 150, 150, 255),
		IM_COL32(150, 255, 150, 255),
		IM_COL32(150, 150, 255, 255)
	};

	// 현재 편집 채널의 값 범위 계산
	auto GetChannelValue = [&](int channel, int pointIdx) -> float {
		FVector* points[] = {&BezierDist->P0, &BezierDist->P1, &BezierDist->P2, &BezierDist->P3};
		if (channel == 0) return points[pointIdx]->X;
		if (channel == 1) return points[pointIdx]->Y;
		return points[pointIdx]->Z;
	};

	auto SetChannelValue = [&](int channel, int pointIdx, float value) {
		FVector* points[] = {&BezierDist->P0, &BezierDist->P1, &BezierDist->P2, &BezierDist->P3};
		if (channel == 0) points[pointIdx]->X = value;
		else if (channel == 1) points[pointIdx]->Y = value;
		else points[pointIdx]->Z = value;
	};

	// 편집할 채널 결정 (All이면 첫 번째 채널 기준으로 스케일)
	int editChannel = (selectedChannel == 3) ? 0 : selectedChannel;

	// Y축 값 범위 (0 ~ 1 고정)
	float minVal = 0.0f;
	float maxVal = 1.0f;

	// X축 시간 범위
	float minTime = BezierDist->MinInput;
	float maxTime = BezierDist->MaxInput;
	if (maxTime - minTime < 0.1f) maxTime = minTime + 1.0f;

	// 좌표 변환 (X: 시간, Y: 값)
	auto ToScreen = [&](float t, float v) -> ImVec2 {
		float x = canvasPos.x + ((t - minTime) / (maxTime - minTime)) * canvasSize.x;
		float y = canvasPos.y + canvasSize.y - ((v - minVal) / (maxVal - minVal)) * canvasSize.y;
		return ImVec2(x, y);
	};

	auto FromScreenY = [&](float screenY) -> float {
		float normalizedY = 1.0f - (screenY - canvasPos.y) / canvasSize.y;
		return minVal + normalizedY * (maxVal - minVal);
	};

	// 채널별로 커브 그리기
	auto DrawChannelCurve = [&](int channel, ImU32 color) {
		float p0 = GetChannelValue(channel, 0);
		float p1 = GetChannelValue(channel, 1);
		float p2 = GetChannelValue(channel, 2);
		float p3 = GetChannelValue(channel, 3);

		int segments = 50;
		for (int i = 0; i < segments; i++)
		{
			float t0 = (float)i / segments;
			float t1 = (float)(i + 1) / segments;

			float u0 = 1.0f - t0;
			float u1 = 1.0f - t1;
			float v0 = u0*u0*u0*p0 + 3*u0*u0*t0*p1 + 3*u0*t0*t0*p2 + t0*t0*t0*p3;
			float v1 = u1*u1*u1*p0 + 3*u1*u1*t1*p1 + 3*u1*t1*t1*p2 + t1*t1*t1*p3;

			// 시간으로 변환
			float time0 = minTime + t0 * (maxTime - minTime);
			float time1 = minTime + t1 * (maxTime - minTime);
			drawList->AddLine(ToScreen(time0, v0), ToScreen(time1, v1), color, 2.0f);
		}
	};

	// 커브 그리기
	if (selectedChannel == 3)
	{
		DrawChannelCurve(0, channelColors[0]);
		DrawChannelCurve(1, channelColors[1]);
		DrawChannelCurve(2, channelColors[2]);
	}
	else
	{
		DrawChannelCurve(selectedChannel, channelColors[selectedChannel]);
	}

	// 제어점 그리기 및 드래그 처리 (단일 채널일 때만)
	if (selectedChannel != 3)
	{
		ImVec2 mousePos = ImGui::GetMousePos();
		float hitRadius = 10.0f;

		// 제어점 시간 위치 (0%, 33%, 66%, 100%)
		float pointTs[] = {
			minTime,
			minTime + (maxTime - minTime) * 0.33f,
			minTime + (maxTime - minTime) * 0.66f,
			maxTime
		};
		const char* pointLabels[] = {"P0", "P1", "P2", "P3"};

		bool isMouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
		bool isMouseClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
		bool isInCanvas = mousePos.x >= canvasPos.x && mousePos.x <= canvasPos.x + canvasSize.x &&
						  mousePos.y >= canvasPos.y && mousePos.y <= canvasPos.y + canvasSize.y;

		// 제어점 스크린 좌표 계산
		ImVec2 pointScreens[4];
		for (int i = 0; i < 4; i++)
		{
			pointScreens[i] = ToScreen(pointTs[i], GetChannelValue(selectedChannel, i));
		}

		// 탄젠트 라인
		drawList->AddLine(pointScreens[0], pointScreens[1], IM_COL32(150, 150, 150, 100), 1.0f);
		drawList->AddLine(pointScreens[2], pointScreens[3], IM_COL32(150, 150, 150, 100), 1.0f);

		// 드래그 시작 검사
		if (isMouseClicked && isInCanvas && draggingPointVec == -1)
		{
			for (int i = 0; i < 4; i++)
			{
				float dx = mousePos.x - pointScreens[i].x;
				float dy = mousePos.y - pointScreens[i].y;
				if (dx * dx + dy * dy <= hitRadius * hitRadius)
				{
					draggingPointVec = i;
					draggingPropNameVec = PropName;
					draggingChannelVec = selectedChannel;
					break;
				}
			}
		}

		// 드래그 중 값 업데이트
		if (draggingPointVec >= 0 && draggingPropNameVec == PropName &&
			draggingChannelVec == selectedChannel && isMouseDown)
		{
			float newValue = FromScreenY(mousePos.y);
			// 값 범위 제한 (MinInput ~ MaxInput 범위)
			newValue = std::clamp(newValue, BezierDist->MinInput, BezierDist->MaxInput);
			SetChannelValue(selectedChannel, draggingPointVec, newValue);
		}

		// 드래그 종료
		if (!isMouseDown && draggingPointVec >= 0 && draggingPropNameVec == PropName)
		{
			draggingPointVec = -1;
			draggingPropNameVec = "";
			draggingChannelVec = -1;
		}

		// 제어점 그리기
		for (int i = 0; i < 4; i++)
		{
			float dx = mousePos.x - pointScreens[i].x;
			float dy = mousePos.y - pointScreens[i].y;
			bool isHovered = (dx * dx + dy * dy <= hitRadius * hitRadius);
			bool isDragging = (draggingPointVec == i && draggingPropNameVec == PropName && draggingChannelVec == selectedChannel);

			ImU32 color = (isHovered || isDragging) ? channelHoverColors[selectedChannel] : channelColors[selectedChannel];
			float radius = (i == 0 || i == 3) ? 8.0f : 6.0f;
			if (isHovered || isDragging) radius += 2.0f;

			drawList->AddCircleFilled(pointScreens[i], radius, color);

			if (isDragging)
			{
				drawList->AddCircle(pointScreens[i], radius + 2.0f, IM_COL32(255, 255, 255, 255), 0, 2.0f);

				// 드래그 중 값 표시
				char dragText[32];
				sprintf_s(dragText, "%s: %.3f", pointLabels[i], GetChannelValue(selectedChannel, i));
				drawList->AddText(ImVec2(mousePos.x + 15, mousePos.y - 10), IM_COL32(255, 255, 255, 255), dragText);
			}
		}
	}

	// 범례
	ImVec2 legendPos = ImVec2(canvasPos.x + 5, canvasPos.y + 5);
	drawList->AddText(legendPos, IM_COL32(255, 100, 100, 255), "X");
	drawList->AddText(ImVec2(legendPos.x + 20, legendPos.y), IM_COL32(100, 255, 100, 255), "Y");
	drawList->AddText(ImVec2(legendPos.x + 40, legendPos.y), IM_COL32(100, 100, 255, 255), "Z");
	if (selectedChannel != 3)
	{
		drawList->AddText(ImVec2(legendPos.x, legendPos.y + 15), IM_COL32(150, 150, 150, 255), "Drag points to edit");
	}

	// InvisibleButton으로 캔버스 영역 캡처
	ImGui::InvisibleButton(("##BezierVecCanvas_" + std::string(PropName)).c_str(), canvasSize);

	// 제어점 값 편집
	ImGui::Spacing();
	ImGui::Text("Control Points:");

	float p0[3] = {BezierDist->P0.X, BezierDist->P0.Y, BezierDist->P0.Z};
	float p1[3] = {BezierDist->P1.X, BezierDist->P1.Y, BezierDist->P1.Z};
	float p2[3] = {BezierDist->P2.X, BezierDist->P2.Y, BezierDist->P2.Z};
	float p3[3] = {BezierDist->P3.X, BezierDist->P3.Y, BezierDist->P3.Z};

	if (ImGui::DragFloat3(("P0 (Start)##V" + std::string(PropName)).c_str(), p0, 0.01f))
	{
		BezierDist->P0 = FVector(p0[0], p0[1], p0[2]);
	}
	if (ImGui::DragFloat3(("P1 (StartTan)##V" + std::string(PropName)).c_str(), p1, 0.01f))
	{
		BezierDist->P1 = FVector(p1[0], p1[1], p1[2]);
	}
	if (ImGui::DragFloat3(("P2 (EndTan)##V" + std::string(PropName)).c_str(), p2, 0.01f))
	{
		BezierDist->P2 = FVector(p2[0], p2[1], p2[2]);
	}
	if (ImGui::DragFloat3(("P3 (End)##V" + std::string(PropName)).c_str(), p3, 0.01f))
	{
		BezierDist->P3 = FVector(p3[0], p3[1], p3[2]);
	}
}

void SParticleEditorWindow::RenderStatsOverlay(const ImVec2& ViewportMin, const ImVec2& ViewportSize)
{
    // 0. 토글 및 유효성 체크
    if (!bShowStatsOverlay || !EditingParticleSystem || !PreviewActor) return;

    UParticleSystemComponent* PSC = Cast<UParticleSystemComponent>(PreviewActor->GetRootComponent());
    if (!PSC) return;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // --- 설정 값 ---
    const float Margin = 5.0f;      // 10 -> 5로 줄여서 오른쪽에 더 붙임
    const float PaddingX = 10.0f;   // 텍스트 좌우 여백 (좌5 + 우5)
    const float Spacing = 5.0f;     // 패널 간 간격
    const float TextHeight = 18.0f; // 텍스트 한 줄 높이 (헤더 등)

    // --- 1. 텍스트 내용 미리 준비 및 너비/높이 계산 ---

    float FrameRate = ImGui::GetIO().Framerate;
    char fpsBuf[64];
    sprintf_s(fpsBuf, "FPS: %.1f (%.2f ms)", FrameRate, 1000.0f / FrameRate);
    ImVec2 fpsSize = ImGui::CalcTextSize(fpsBuf);

    int32 TotalActive = PSC->GetTotalActiveParticles();
    int32 EmitterCount = (int32)EditingParticleSystem->Emitters.size();
    char totalBuf[128];
    sprintf_s(totalBuf, "Total Particles: %d\nActive Emitters: %d", TotalActive, EmitterCount);
    ImVec2 totalSize = ImGui::CalcTextSize(totalBuf);

    float maxEmitterTextWidth = 0.0f;
    ImVec2 headerSize = ImGui::CalcTextSize("Emitter Stats");
    maxEmitterTextWidth = headerSize.x;

    for (int i = 0; i < EmitterCount; ++i)
    {
        UParticleEmitter* Emitter = EditingParticleSystem->Emitters[i];
        if (!Emitter) continue;

        char tempBuf[128];
        int32 count = PSC->GetActiveParticleCount(i);
        sprintf_s(tempBuf, "%s: %d", Emitter->GetEmitterName().ToString().c_str(), count);

        ImVec2 itemSize = ImGui::CalcTextSize(tempBuf);
        if (itemSize.x > maxEmitterTextWidth)
        {
            maxEmitterTextWidth = itemSize.x;
        }
    }

    // --- 2. 최종 패널 너비 결정 (Dynamic Width) ---
    float contentMaxWidth = fpsSize.x;
    if (totalSize.x > contentMaxWidth) contentMaxWidth = totalSize.x;
    if (maxEmitterTextWidth > contentMaxWidth) contentMaxWidth = maxEmitterTextWidth;

    float FinalPanelWidth = contentMaxWidth + PaddingX;
    if (FinalPanelWidth < 120.0f) FinalPanelWidth = 120.0f;


    // --- 3. 높이 및 시작 위치 계산 ---

    float fpsPanelHeight = fpsSize.y + 10.0f;
    float totalPanelHeight = totalSize.y + 10.0f;

    float listBodyHeight = 0.0f;
    if (EmitterCount > 0)
        listBodyHeight = (EmitterCount * TextHeight) + 5.0f;
    else
        listBodyHeight = 20.0f; // "No Emitters" or empty space

    float emitterTotalHeight = 20.0f + listBodyHeight; // Header(20) + Body

    float totalOverlayHeight = fpsPanelHeight + Spacing + totalPanelHeight + Spacing + emitterTotalHeight;

    ImVec2 CurrentPos;
    CurrentPos.x = (ViewportMin.x + ViewportSize.x) - FinalPanelWidth - Margin;
    CurrentPos.y = (ViewportMin.y + ViewportSize.y) - totalOverlayHeight - Margin;


    // --- 4. 실제 그리기 ---

    // [FPS Panel]
    {
        drawList->AddRectFilled(
            CurrentPos,
            ImVec2(CurrentPos.x + FinalPanelWidth, CurrentPos.y + fpsPanelHeight),
            IM_COL32(0, 0, 0, 150), 4.0f
        );
        drawList->AddText(
            ImVec2(CurrentPos.x + 5.0f, CurrentPos.y + 5.0f),
            IM_COL32(255, 255, 0, 255), fpsBuf
        );
        CurrentPos.y += fpsPanelHeight + Spacing;
    }

    // [Total Stats Panel]
    {
        drawList->AddRectFilled(
            CurrentPos,
            ImVec2(CurrentPos.x + FinalPanelWidth, CurrentPos.y + totalPanelHeight),
            IM_COL32(0, 0, 0, 150), 4.0f
        );
        drawList->AddText(
            ImVec2(CurrentPos.x + 5.0f, CurrentPos.y + 5.0f),
            IM_COL32(135, 206, 235, 255), totalBuf
        );
        CurrentPos.y += totalPanelHeight + Spacing;
    }

    // [Emitter List Panel]
    {
        ImVec4 emitterColors[] = {
            ImVec4(1.0f, 0.5f, 0.0f, 1.0f), ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
            ImVec4(0.3f, 0.3f, 0.3f, 1.0f), ImVec4(0.4f, 0.4f, 1.0f, 1.0f)
        };

        // Header
        drawList->AddRectFilled(CurrentPos, ImVec2(CurrentPos.x + FinalPanelWidth, CurrentPos.y + 20), IM_COL32(0, 0, 0, 200), 4.0f, ImDrawFlags_RoundCornersTop);
        drawList->AddText(ImVec2(CurrentPos.x + 5, CurrentPos.y + 2), IM_COL32(255, 255, 255, 255), "Emitter Stats");
        CurrentPos.y += 20;

        // List Body
        drawList->AddRectFilled(CurrentPos, ImVec2(CurrentPos.x + FinalPanelWidth, CurrentPos.y + listBodyHeight), IM_COL32(0, 0, 0, 150), 4.0f, ImDrawFlags_RoundCornersBottom);

        float TextY = CurrentPos.y + 2.0f;
        for (int i = 0; i < EmitterCount; ++i)
        {
            UParticleEmitter* Emitter = EditingParticleSystem->Emitters[i];
            if (!Emitter) continue;

            int32 count = PSC->GetActiveParticleCount(i);
            char buf[128];
            sprintf_s(buf, "%s: %d", Emitter->GetEmitterName().ToString().c_str(), count);

            ImU32 color = ImGui::ColorConvertFloat4ToU32(emitterColors[i % 4]);
            drawList->AddText(ImVec2(CurrentPos.x + 5, TextY), color, buf);
            TextY += TextHeight;
        }
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

void SParticleEditorWindow::OnUpdate(float DeltaSeconds)
{
	if (!PreviewState || !PreviewState->World)
	{
		return;
	}

	// ViewportClient Tick (카메라 입력 처리)
	if (PreviewState->Client)
	{
		PreviewState->Client->Tick(DeltaSeconds);
	}

	// PreviewWorld 업데이트
	PreviewState->World->Tick(DeltaSeconds);
}

void SParticleEditorWindow::OnRenderViewport()
{
	// ImGui::Image 방식으로 뷰포트 렌더링 (렌더 타겟에 렌더링하므로 항상 0,0 기준)
	if (PreviewState && PreviewState->Viewport &&
		PreviewViewportRect.GetWidth() > 0 && PreviewViewportRect.GetHeight() > 0)
	{
		const uint32 NewWidth = static_cast<uint32>(PreviewViewportRect.Right - PreviewViewportRect.Left);
		const uint32 NewHeight = static_cast<uint32>(PreviewViewportRect.Bottom - PreviewViewportRect.Top);

		// 렌더 타겟에 렌더링하므로 StartX, StartY는 항상 0
		PreviewState->Viewport->Resize(0, 0, NewWidth, NewHeight);
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
	UParticleEmitter* TestEmitter = NewObject<UParticleSpriteEmitter>();
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
	LODLevel->RequiredModule = RequiredModule;

	// Spawn 모듈 생성 및 추가
	UParticleModuleSpawn* SpawnModule = NewObject<UParticleModuleSpawn>();
	SpawnModule->Rate = 10.0f;
	SpawnModule->RateScale = 1.0f;
	LODLevel->SpawnModule = SpawnModule;

	// Lifetime 모듈 생성 및 추가
	UParticleModuleLifetime* LifetimeModule = NewObject<UParticleModuleLifetime>();
	LODLevel->Modules.Add(LifetimeModule);

	// LOD 레벨을 이미터에 추가
	TestEmitter->LODLevels.Add(LODLevel);

	// 이미터를 파티클 시스템에 추가
	EditingParticleSystem->Emitters.Add(TestEmitter);

	if (PreviewActor)
	{
		UParticleSystemComponent* Component = PreviewActor->GetParticleSystemComponent();
		if (Component)
		{
			// 이미터 추가시 인스턴스를 재생성
			Component->InitializeSystem();
		}
	}

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
				if (PreviewActor)
				{
					UParticleSystemComponent* Component = PreviewActor->GetParticleSystemComponent();
					if (Component)
					{
						// 모듈 추가시 인스턴스를 재생성
						Component->InitializeSystem();
					}
				}
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
	UParticleEmitter* NewEmitter = EditingParticleSystem->AddEmitter(UParticleSpriteEmitter::StaticClass());

	if (NewEmitter)
	{
		// 이미터 이름 설정
		char emitterName[64];
		sprintf_s(emitterName, "Emitter_%zu", EditingParticleSystem->Emitters.size() - 1);
		NewEmitter->SetEmitterName(FName(emitterName));

		UE_LOG("New emitter added: %s (Total emitters: %zu)", emitterName, EditingParticleSystem->Emitters.size());
	}
}

void SParticleEditorWindow::LoadParticleSystemFromFile()
{
	const FWideString BaseDir = UTF8ToWide(GDataDir) + L"/Particle";
	const FWideString Extension = L".particle";
	const FWideString Description = L"Particle Files";

	// 플랫폼 공용 다이얼로그 호출 (SelectedPath는 ABSOLUTE PATH)
	std::filesystem::path SelectedPath = FPlatformProcess::OpenLoadFileDialog(BaseDir, Extension, Description);

	if (!SelectedPath.empty())
	{
		FWideString AbsolutePath = SelectedPath.wstring();
		FString FinalPathStr = ResolveAssetRelativePath(WideToUTF8(AbsolutePath), "");

		// 선택 초기화 (댕글링 포인터 방지를 위해 먼저 초기화)
		SelectedModule = nullptr;
		SelectedModuleIndex = -1;
		SelectedEmitterIndex = 0;

		// Detail Widget 초기화
		if (DetailWidget)
		{
			DetailWidget->SetSelectedModule(nullptr);
			DetailWidget->SetSelectedEmitter(nullptr);
			DetailWidget->SetParticleSystem(nullptr);
		}

		// 기존 파티클 시스템 삭제
		if (EditingParticleSystem)
		{
			DeleteObject(EditingParticleSystem);
			EditingParticleSystem = nullptr;
		}

		// 새 파티클 시스템 생성
		EditingParticleSystem = NewObject<UParticleSystem>();

		// 파일 로드
		bool bLoadSuccess = EditingParticleSystem->LoadFromFile(UTF8ToWide(FinalPathStr));

		// 로드 성공/실패와 관계없이 파티클 시스템은 유효한 상태여야 함
		// Detail Widget에 파티클 시스템 설정
		if (DetailWidget)
		{
			DetailWidget->SetParticleSystem(EditingParticleSystem);
		}

		// 프리뷰 액터 업데이트
		UpdatePreviewActor();

		if (bLoadSuccess)
		{
			CurrentFilePath = UTF8ToWide(FinalPathStr);
			StatusMessage = "Loaded: " + FinalPathStr;
			UE_LOG("[ParticleEditor] Loaded from: %s", FinalPathStr.c_str());
		}
		else
		{
			StatusMessage = "Failed to load particle system";
			CurrentFilePath.clear();
			UE_LOG("[Error] Failed to load ParticleSystem: %S", AbsolutePath.c_str());
		}
	}
}

void SParticleEditorWindow::SaveParticleSystemToFile()
{
	if (!EditingParticleSystem)
	{
		StatusMessage = "No particle system to save";
		return;
	}

	// 현재 파일 경로가 없으면 Save As로 처리
	if (CurrentFilePath.empty())
	{
		SaveParticleSystemToFileAs();
		return;
	}

	// 파일 저장
	if (EditingParticleSystem->SaveToFile(CurrentFilePath))
	{
		StatusMessage = "Saved: " + WideToUTF8(CurrentFilePath);
		UE_LOG("[ParticleEditor] Saved to: %s", WideToUTF8(CurrentFilePath).c_str());
	}
	else
	{
		StatusMessage = "Failed to save particle system";
		UE_LOG("[Error] Failed to save ParticleSystem: %S", CurrentFilePath.c_str());
	}
}

void SParticleEditorWindow::SaveParticleSystemToFileAs()
{
	if (!EditingParticleSystem)
	{
		StatusMessage = "No particle system to save";
		return;
	}

	const FWideString BaseDir = UTF8ToWide(GDataDir) + L"/Particle";
	const FWideString Extension = L".particle";
	const FWideString Description = L"Particle Files";

	// 플랫폼 공용 다이얼로그 호출 (SelectedPath는 ABSOLUTE PATH)
	std::filesystem::path SelectedPath = FPlatformProcess::OpenSaveFileDialog(BaseDir, Extension, Description);

	if (!SelectedPath.empty())
	{
		// 절대 경로를 상대 경로로 변환
		FWideString AbsolutePath = SelectedPath.wstring();
		FString FinalPathStr = ResolveAssetRelativePath(WideToUTF8(AbsolutePath), "");

		// 저장
		if (EditingParticleSystem->SaveToFile(UTF8ToWide(FinalPathStr)))
		{
			CurrentFilePath = UTF8ToWide(FinalPathStr);
			StatusMessage = "Saved: " + FinalPathStr;
			UE_LOG("[ParticleEditor] Saved to: %s", FinalPathStr.c_str());
		}
		else
		{
			StatusMessage = "Failed to save particle system";
			UE_LOG("[Error] Failed to save ParticleSystem: %S", AbsolutePath.c_str());
		}
	}
}

void SParticleEditorWindow::UpdatePreviewActor()
{
	if (!PreviewActor)
		return;

	// 파티클 시스템 컴포넌트 가져오기
	UParticleSystemComponent* ParticleComp = Cast<UParticleSystemComponent>(PreviewActor->GetRootComponent());
	if (ParticleComp)
	{
		// 현재 편집 중인 파티클 시스템으로 템플릿 설정
		ParticleComp->SetTemplate(EditingParticleSystem);
	}
}
