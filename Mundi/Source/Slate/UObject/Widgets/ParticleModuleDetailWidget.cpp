#include "pch.h"
#include "ParticleModuleDetailWidget.h"
#include "Source/Runtime/Engine/Particle/ParticleModule.h"
#include "Source/Runtime/Engine/Particle/ParticleSystem.h"
#include "PropertyRenderer.h"
#include "ImGui/imgui.h"

IMPLEMENT_CLASS(UParticleModuleDetailWidget)

UParticleModuleDetailWidget::UParticleModuleDetailWidget()
{
	SetName("ParticleModuleDetailWidget");
}

void UParticleModuleDetailWidget::Initialize()
{
	UE_LOG("UParticleModuleDetailWidget: Initialized");
}

void UParticleModuleDetailWidget::Update()
{
	// 업데이트 로직 (필요시 구현)
}

void UParticleModuleDetailWidget::RenderWidget()
{
	// Details header with tab
	if (ImGui::BeginTabBar("DetailsTabs", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("Details"))
		{
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	// Search box
	ImGui::SetNextItemWidth(-1);
	ImGui::InputTextWithHint("##DetailsSearch", "Search", SearchBuffer, IM_ARRAYSIZE(SearchBuffer));

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// 선택된 모듈이 있으면 해당 모듈의 프로퍼티 표시
	if (SelectedModule)
	{
		// 모듈 이름 표시
		ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "%s", SelectedModule->GetClass()->DisplayName);
		ImGui::Separator();
		ImGui::Spacing();

		// UPropertyRenderer를 사용하여 자동으로 프로퍼티 렌더링
		UPropertyRenderer::RenderAllPropertiesWithInheritance(SelectedModule);
	}
	// Particle System이 선택되어 있으면 Particle System 프로퍼티 표시
	else if (ParticleSystem)
	{
		// 시스템 이름 표시
		ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "Particle System");
		ImGui::Separator();
		ImGui::Spacing();

		UPropertyRenderer::RenderAllPropertiesWithInheritance(ParticleSystem);
	}
	else
	{
		ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No module selected");
		ImGui::Spacing();
		ImGui::TextWrapped("Select a module from the Emitters panel to view its properties");
	}
}

void UParticleModuleDetailWidget::SetSelectedModule(UParticleModule* InModule)
{
	SelectedModule = InModule;
}

void UParticleModuleDetailWidget::SetParticleSystem(UParticleSystem* InParticleSystem)
{
	ParticleSystem = InParticleSystem;
}
