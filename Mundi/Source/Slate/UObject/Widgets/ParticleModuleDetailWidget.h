#pragma once
#include "Widget.h"

class UParticleModule;
class UParticleSystem;
class UParticleEmitter;

/**
 * @brief 파티클 모듈의 프로퍼티를 표시하는 Detail Widget
 */
class UParticleModuleDetailWidget : public UWidget
{
public:
	DECLARE_CLASS(UParticleModuleDetailWidget, UWidget)

	UParticleModuleDetailWidget();
	~UParticleModuleDetailWidget() override = default;

	virtual void Initialize() override;
	virtual void Update() override;
	virtual void RenderWidget() override;

	/**
	 * @brief 선택된 모듈 설정
	 */
	void SetSelectedModule(UParticleModule* InModule);

	/**
	 * @brief 파티클 시스템 설정
	 */
	void SetParticleSystem(UParticleSystem* InParticleSystem);

	/**
	 * @brief 선택된 이미터 설정
	 */
	void SetSelectedEmitter(UParticleEmitter* InEmitter);

private:
	// 선택된 모듈
	UParticleModule* SelectedModule = nullptr;

	// 선택된 이미터
	UParticleEmitter* SelectedEmitter = nullptr;

	// 파티클 시스템
	UParticleSystem* ParticleSystem = nullptr;

	// 검색 버퍼
	char SearchBuffer[256] = "";
};
