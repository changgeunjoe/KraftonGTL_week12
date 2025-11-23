#pragma once
#include "UIWindow.h"
#include "Source/Slate/Core/Panels/SVerticalBox.h"
#include "Source/Slate/Core/Panels/SHorizontalBox.h"
#include "Source/Slate/Core/Panels/SScrollBox.h"
#include "Source/Slate/Core/Panels/SGridPanel.h"
#include "Source/Slate/Core/Widgets/SButton.h"
#include "Source/Slate/Core/Widgets/STextBlock.h"
#include "Source/Slate/Core/Widgets/SEditableText.h"
#include "Source/Slate/Core/Widgets/SCheckBox.h"
#include "Source/Slate/Core/Widgets/SComboBox.h"
#include "Source/Slate/Core/Widgets/SImage.h"
#include "Source/Slate/Core/Widgets/SSeparator.h"
#include "Source/Slate/Core/Widgets/SSlider.h"
#include "Source/Slate/Core/Widgets/STreeView.h"
#include "Source/Slate/Core/Widgets/SListView.h"
#include "Source/Slate/Core/Widgets/SColorPicker.h"

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
	SGridPanel* GridPanel1 = nullptr;

	// Core Slate 위젯들
	SButton* Button1 = nullptr;
	SButton* Button2 = nullptr;
	SButton* Button3 = nullptr;
	STextBlock* TitleText = nullptr;
	STextBlock* StatusText = nullptr;
	STextBlock* CounterText = nullptr;
	SEditableText* NameInput = nullptr;

	// Phase 2 위젯들
	SCheckBox* CheckBox1 = nullptr;
	SCheckBox* CheckBox2 = nullptr;
	SComboBox* ComboBox1 = nullptr;
	SScrollBox* ScrollBox1 = nullptr;
	SSeparator* Separator1 = nullptr;
	SSeparator* Separator2 = nullptr;
	SSlider* Slider1 = nullptr;
	SSlider* Slider2 = nullptr;
	STextBlock* SliderValueText = nullptr;

	// Phase 3 고급 위젯들
	STreeView* TreeView1 = nullptr;
	SListView* ListView1 = nullptr;
	SColorPicker* ColorPicker1 = nullptr;

	// 상태
	uint32 ClickCount = 0;
	FString InputText = "Test Input";
	bool bCheckState1 = false;
	bool bCheckState2 = true;
	float SliderValue1 = 50.0f;
	float SliderValue2 = 75.0f;

	void CreateLayout();
	void OnButton1Clicked();
	void OnButton2Clicked();
	void OnButton3Clicked();
};
