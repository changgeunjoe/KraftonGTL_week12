#pragma once
#include "UIWindow.h"
#include "Source/Slate/Core/Panels/SVerticalBox.h"
#include "Source/Slate/Core/Panels/SHorizontalBox.h"
#include "Source/Slate/Core/Widgets/SButton.h"
#include "Source/Slate/Core/Widgets/STextBlock.h"

/**
 * Core Slate 위젯들을 테스트하기 위한 윈도우
 */
class USlateTestWindow : public UUIWindow
{
public:
	DECLARE_CLASS(USlateTestWindow, UUIWindow)

	USlateTestWindow();
	virtual ~USlateTestWindow();

	virtual void Initialize() override;
	virtual void RenderWidget() const override;  // Core Slate 렌더링

private:
	// Core Slate 레이아웃
	SVerticalBox* RootLayout = nullptr;
	SHorizontalBox* Toolbar = nullptr;
	SHorizontalBox* ButtonRow = nullptr;
	SVerticalBox* ContentArea = nullptr;

	// Core Slate 위젯들
	SButton* Button1 = nullptr;
	SButton* Button2 = nullptr;
	SButton* Button3 = nullptr;
	STextBlock* TitleText = nullptr;
	STextBlock* StatusText = nullptr;
	STextBlock* CounterText = nullptr;

	// 상태
	int32_t ClickCount = 0;

	void CreateLayout();
	void OnButton1Clicked();
	void OnButton2Clicked();
	void OnButton3Clicked();
};
