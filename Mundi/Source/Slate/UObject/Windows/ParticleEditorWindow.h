#pragma once
#include "Source/Slate/Core/Windows/SWindow.h"

class ViewerState;
class UParticleModule;
class UParticleSystem;
class UParticleEmitter;
class UParticleModuleDetailWidget;
class AParticleSystemActor;

class SParticleEditorWindow : public SWindow
{
public:
	SParticleEditorWindow();
	virtual ~SParticleEditorWindow();

	bool Initialize(float StartX, float StartY, float Width, float Height, class UWorld* InWorld, ID3D11Device* InDevice);

	virtual void OnRender() override;
	virtual void OnUpdate(float DeltaSeconds) override;
	virtual void OnMouseDown(FVector2D MousePos, uint32 Button) override;
	virtual void OnMouseUp(FVector2D MousePos, uint32 Button) override;
	virtual void OnMouseMove(FVector2D MousePos) override;

	void OnRenderViewport();

	bool IsOpen() const { return bIsOpen; }
	void Close() { bIsOpen = false; }

	void SetParticleSystem(UParticleSystem* InParticleSystem);

	// File operations
	void LoadParticleSystemFromFile();
	void SaveParticleSystemToFile();
	void SaveParticleSystemToFileAs();

private:
	// UI 렌더링
	void RenderTopToolbar();
	void RenderViewportPanel();
	void RenderDetailsPanel();
	void RenderEmittersPanel();
	void RenderCurveEditorPanel();
	void RenderStatsOverlay(const ImVec2& ViewportMin, const ImVec2& ViewportSize);

	// 헬퍼 함수
	UParticleModule* GetModuleFromCurrentEmitter(int32 ModuleIndex);
	void CreateTestParticleSystem();
	void ShowAddModuleContextMenu(int32 EmitterIndex);
	void AddNewEmitter();

	// Preview Viewport
	ViewerState* PreviewState = nullptr;
	FRect PreviewViewportRect = FRect(0, 0, 0, 0);
	ID3D11Device* Device = nullptr;

	// Preview Actor (파티클 미리보기용)
	AParticleSystemActor* PreviewActor = nullptr;

	// 프리뷰 업데이트 함수
	void UpdatePreviewActor();

	// State
	bool bIsOpen = true;
	bool bIsPlaying = false;
	bool bShowStatsOverlay = true;
	FString StatusMessage = "Ready";

	// Details Panel Widget
	UParticleModuleDetailWidget* DetailWidget = nullptr;

	// Selected Module
	UParticleModule* SelectedModule = nullptr;
	int32 SelectedModuleIndex = -1;

	// Particle System
	UParticleSystem* EditingParticleSystem = nullptr;

	// Current file path
	FWideString CurrentFilePath;

	// Selected Emitter
	int32 SelectedEmitterIndex = 0;

	// Event handlers
	void OnPlayClicked();
	void OnPauseClicked();
	void OnResetClicked();
	void OnRestartSimClicked();
	void OnRestartLevelClicked();
};
