#include "pch.h"
#include "SlateTestWindow.h"

IMPLEMENT_CLASS(USlateTestWindow)

USlateTestWindow::USlateTestWindow()
{
	// 윈도우 설정
	FUIWindowConfig Config;
	Config.WindowTitle = "Slate Test Window";
	Config.DefaultSize = ImVec2(800, 600);
	Config.WindowFlags = ImGuiWindowFlags_None;
	SetConfig(Config);
}

USlateTestWindow::~USlateTestWindow()
{
	// Core Slate 위젯들 정리
	if (RootLayout)
	{
		delete RootLayout;
		RootLayout = nullptr;
	}
}

void USlateTestWindow::Initialize()
{
	UUIWindow::Initialize();
	CreateLayout();
}

void USlateTestWindow::CreateLayout()
{
	// 루트 레이아웃 생성
	RootLayout = new SVerticalBox();

	// ===== 헤더 (고정 높이) =====
	TitleText = new STextBlock("Core Slate Test Window");
	TitleText->SetColor(0xFFFFCC64); // 주황색
	TitleText->SetJustification(STextBlock::Center);

	RootLayout->AddSlot()
		.FixedHeight(30.0f)  // Fixed 사용 - 타이틀 높이 설정
		.SetPadding(0.0f)
		.AttachWidget(TitleText);

	// ===== 툴바 (고정 높이) =====
	Toolbar = new SHorizontalBox();

	// 버튼 1
	Button1 = new SButton();
	Button1->SetText("Increment");
	Button1->SetSize(FVector2D(100, 30));
	Button1->OnClicked.Add([this]() {
		OnButton1Clicked();
	});

	// 버튼 2
	Button2 = new SButton();
	Button2->SetText("Reset");
	Button2->SetSize(FVector2D(100, 30));
	Button2->OnClicked.Add([this]() {
		OnButton2Clicked();
	});

	// 버튼 3
	Button3 = new SButton();
	Button3->SetText("Test");
	Button3->SetSize(FVector2D(100, 30));
	Button3->OnClicked.Add([this]() {
		OnButton3Clicked();
	});

	// 툴바 구성
	Toolbar->AddSlot()
		.FixedWidth(100.0f)
		.SetPadding(5.0f)
		.AttachWidget(Button1);

	Toolbar->AddSlot()
		.FixedWidth(100.0f)
		.SetPadding(5.0f)
		.AttachWidget(Button2);

	Toolbar->AddSlot()
		.FixedWidth(100.0f)
		.SetPadding(5.0f)
		.AttachWidget(Button3);

	// Spacer
	Toolbar->AddSlot()
		.FillWidth(1.0f)
		.AttachWidget(new SPanel());

	// 상태 텍스트
	StatusText = new STextBlock("Ready");
	StatusText->SetColor(0xFF00FF00); // 초록색

	Toolbar->AddSlot()
		.AutoWidth()
		.SetPadding(10.0f, 5.0f)
		.VAlignment(SHorizontalBox::VAlign_Center)
		.AttachWidget(StatusText);

	RootLayout->AddSlot()
		.FixedHeight(50.0f)
		.SetPadding(10.0f, 0.0f)
		.AttachWidget(Toolbar);

	// ===== 메인 콘텐츠 영역 (남은 공간) =====
	ContentArea = new SVerticalBox();

	// 카운터 텍스트 (동적)
	CounterText = new STextBlock();
	CounterText->SetText([this]() -> FString {
		return "Click Count: " + std::to_string(ClickCount);
	});
	CounterText->SetColor(0xFFFFFFFF); // 흰색
	CounterText->SetJustification(STextBlock::Center);

	ContentArea->AddSlot()
		.AutoHeight()
		.SetPadding(20.0f)
		.AttachWidget(CounterText);

	// 설명 텍스트들
	auto Description1 = new STextBlock("This is a test window for Core Slate widgets.");
	Description1->SetColor(0xFFCCCCCC);
	Description1->SetJustification(STextBlock::Center);

	ContentArea->AddSlot()
		.AutoHeight()
		.SetPadding(20.0f, 10.0f)
		.AttachWidget(Description1);

	auto Description2 = new STextBlock("Click the buttons above to test functionality.");
	Description2->SetColor(0xFFCCCCCC);
	Description2->SetJustification(STextBlock::Center);

	ContentArea->AddSlot()
		.AutoHeight()
		.SetPadding(20.0f, 10.0f)
		.AttachWidget(Description2);

	// 텍스트 입력 테스트
	NameInput = new SEditableText();
	NameInput->SetText(InputText);
	NameInput->SetHintText("Enter your name...");
	NameInput->OnTextChanged.Add([this](const FString& NewText) {
		InputText = NewText;
		StatusText->SetText("Input: " + NewText);
	});
	NameInput->OnTextCommitted.Add([this](const FString& CommittedText) {
		StatusText->SetText("Committed: " + CommittedText);
	});

	ContentArea->AddSlot()
		.FixedHeight(30.0f)
		.SetPadding(20.0f, 10.0f)
		.AttachWidget(NameInput);

	// ===== Phase 2 위젯 테스트 영역 =====

	// 구분선 1
	Separator1 = new SSeparator(SSeparator::Horizontal);
	Separator1->SetThickness(2.0f);
	Separator1->SetColor(0xFFFFCC64);

	ContentArea->AddSlot()
		.FixedHeight(2.0f)
		.SetPadding(20.0f, 20.0f)
		.AttachWidget(Separator1);

	// Phase 2 타이틀
	auto Phase2Title = new STextBlock("Phase 2 Widgets Test");
	Phase2Title->SetColor(0xFFFFCC64);
	Phase2Title->SetJustification(STextBlock::Center);

	ContentArea->AddSlot()
		.AutoHeight()
		.SetPadding(20.0f, 10.0f)
		.AttachWidget(Phase2Title);

	// 체크박스 테스트
	auto CheckBoxRow = new SHorizontalBox();

	CheckBox1 = new SCheckBox("Enable Feature A", bCheckState1);
	CheckBox1->OnCheckStateChanged.Add([this](bool bChecked) {
		bCheckState1 = bChecked;
		StatusText->SetText(bChecked ? "Feature A Enabled" : "Feature A Disabled");
		StatusText->SetColor(bChecked ? 0xFF00FF00 : 0xFFFF0000);
	});

	CheckBox2 = new SCheckBox("Enable Feature B", bCheckState2);
	CheckBox2->OnCheckStateChanged.Add([this](bool bChecked) {
		bCheckState2 = bChecked;
		StatusText->SetText(bChecked ? "Feature B Enabled" : "Feature B Disabled");
		StatusText->SetColor(bChecked ? 0xFF00FF00 : 0xFFFF0000);
	});

	CheckBoxRow->AddSlot()
		.FillWidth(1.0f)
		.AttachWidget(new SPanel()); // Left Spacer

	CheckBoxRow->AddSlot()
		.AutoWidth()
		.SetPadding(10.0f)
		.AttachWidget(CheckBox1);

	CheckBoxRow->AddSlot()
		.AutoWidth()
		.SetPadding(10.0f)
		.AttachWidget(CheckBox2);

	CheckBoxRow->AddSlot()
		.FillWidth(1.0f)
		.AttachWidget(new SPanel()); // Right Spacer

	ContentArea->AddSlot()
		.FixedHeight(30.0f)
		.SetPadding(20.0f, 10.0f)
		.AttachWidget(CheckBoxRow);

	// 콤보박스 테스트
	auto ComboRow = new SHorizontalBox();

	auto ComboLabel = new STextBlock("Select Option:");
	ComboLabel->SetColor(0xFFCCCCCC);

	ComboBox1 = new SComboBox();
	ComboBox1->AddOption("Option 1: Fast");
	ComboBox1->AddOption("Option 2: Medium");
	ComboBox1->AddOption("Option 3: Slow");
	ComboBox1->SetSelectedIndex(0);
	ComboBox1->OnSelectionChanged.Add([this](uint32 Index, const FString& Value) {
		StatusText->SetText("Selected: " + Value);
		StatusText->SetColor(0xFF00FFFF);
	});

	ComboRow->AddSlot()
		.FillWidth(1.0f)
		.AttachWidget(new SPanel()); // Left Spacer

	ComboRow->AddSlot()
		.AutoWidth()
		.SetPadding(10.0f)
		.VAlignment(SHorizontalBox::VAlign_Center)
		.AttachWidget(ComboLabel);

	ComboRow->AddSlot()
		.AutoWidth()
		.SetPadding(10.0f)
		.AttachWidget(ComboBox1);

	ComboRow->AddSlot()
		.FillWidth(1.0f)
		.AttachWidget(new SPanel()); // Right Spacer

	ContentArea->AddSlot()
		.FixedHeight(30.0f)
		.SetPadding(20.0f, 10.0f)
		.AttachWidget(ComboRow);

	// 구분선 2
	Separator2 = new SSeparator(SSeparator::Horizontal);
	Separator2->SetThickness(1.0f);
	Separator2->SetColor(0xFF808080);

	ContentArea->AddSlot()
		.FixedHeight(1.0f)
		.SetPadding(20.0f, 10.0f)
		.AttachWidget(Separator2);

	// 슬라이더 테스트
	auto SliderRow1 = new SHorizontalBox();

	Slider1 = new SSlider(0.0f, 100.0f);
	Slider1->SetValue(SliderValue1);
	Slider1->SetLabel("Volume:");
	Slider1->SetWidth(250.0f);
	Slider1->SetFormat("%.0f%%");
	Slider1->OnValueChanged.Add([this](float NewValue) {
		SliderValue1 = NewValue;
		StatusText->SetText("Volume: " + std::to_string((int)NewValue) + "%");
		StatusText->SetColor(0xFFFFAA00);
	});
	Slider1->OnValueCommitted.Add([this](float FinalValue) {
		StatusText->SetText("Volume Set: " + std::to_string((int)FinalValue) + "%");
	});

	SliderRow1->AddSlot()
		.FillWidth(1.0f)
		.AttachWidget(new SPanel()); // Left Spacer

	SliderRow1->AddSlot()
		.AutoWidth()
		.SetPadding(10.0f)
		.VAlignment(SHorizontalBox::VAlign_Center)
		.AttachWidget(Slider1);

	SliderRow1->AddSlot()
		.FillWidth(1.0f)
		.AttachWidget(new SPanel()); // Right Spacer

	ContentArea->AddSlot()
		.FixedHeight(30.0f)
		.SetPadding(20.0f, 10.0f)
		.AttachWidget(SliderRow1);

	// 슬라이더 테스트 2 (범위: 0 ~ 1)
	auto SliderRow2 = new SHorizontalBox();

	Slider2 = new SSlider(0.0f, 1.0f);
	Slider2->SetValue(SliderValue2 / 100.0f);
	Slider2->SetLabel("Opacity:");
	Slider2->SetWidth(250.0f);
	Slider2->SetFormat("%.2f");
	Slider2->OnValueChanged.Add([this](float NewValue) {
		SliderValue2 = NewValue * 100.0f;
	});

	// 슬라이더 값 텍스트 (동적)
	SliderValueText = new STextBlock();
	SliderValueText->SetText([this]() -> FString {
		char Buffer[32];
		sprintf_s(Buffer, "%.0f%%", SliderValue2);
		return FString(Buffer);
	});
	SliderValueText->SetColor(0xFF00FFFF);

	SliderRow2->AddSlot()
		.FillWidth(1.0f)
		.AttachWidget(new SPanel()); // Left Spacer

	SliderRow2->AddSlot()
		.AutoWidth()
		.SetPadding(10.0f)
		.VAlignment(SHorizontalBox::VAlign_Center)
		.AttachWidget(Slider2);

	SliderRow2->AddSlot()
		.AutoWidth()
		.SetPadding(10.0f, 0.0f, 0.0f, 0.0f)
		.VAlignment(SHorizontalBox::VAlign_Center)
		.AttachWidget(SliderValueText);

	SliderRow2->AddSlot()
		.FillWidth(1.0f)
		.AttachWidget(new SPanel()); // Right Spacer

	ContentArea->AddSlot()
		.FixedHeight(30.0f)
		.SetPadding(20.0f, 10.0f)
		.AttachWidget(SliderRow2);

	// 그리드 패널 테스트
	auto GridTitle = new STextBlock("Grid Panel Test (Form Layout)");
	GridTitle->SetColor(0xFFFFCC64);
	GridTitle->SetJustification(STextBlock::Center);

	ContentArea->AddSlot()
		.AutoHeight()
		.SetPadding(20.0f, 20.0f, 20.0f, 10.0f)
		.AttachWidget(GridTitle);

	// 그리드 패널 생성
	GridPanel1 = new SGridPanel();

	// Row 0: Name
	auto NameLabel = new STextBlock("Name:");
	NameLabel->SetColor(0xFFCCCCCC);
	GridPanel1->AddSlot(0, 0)
		.SetPadding(5.0f)
		.HAlignment(SGridPanel::FSlot::HAlign_Right)
		.VAlignment(SGridPanel::FSlot::VAlign_Center)
		.AttachWidget(NameLabel);

	auto NameInput = new SEditableText("John Doe");
	GridPanel1->AddSlot(0, 1)
		.SetPadding(5.0f)
		.AttachWidget(NameInput);

	// Row 1: Email
	auto EmailLabel = new STextBlock("Email:");
	EmailLabel->SetColor(0xFFCCCCCC);
	GridPanel1->AddSlot(1, 0)
		.SetPadding(5.0f)
		.HAlignment(SGridPanel::FSlot::HAlign_Right)
		.VAlignment(SGridPanel::FSlot::VAlign_Center)
		.AttachWidget(EmailLabel);

	auto EmailInput = new SEditableText("john@example.com");
	GridPanel1->AddSlot(1, 1)
		.SetPadding(5.0f)
		.AttachWidget(EmailInput);

	// Row 2: Age (라벨 + 슬라이더)
	auto AgeLabel = new STextBlock("Age:");
	AgeLabel->SetColor(0xFFCCCCCC);
	GridPanel1->AddSlot(2, 0)
		.SetPadding(5.0f)
		.HAlignment(SGridPanel::FSlot::HAlign_Right)
		.VAlignment(SGridPanel::FSlot::VAlign_Center)
		.AttachWidget(AgeLabel);

	auto AgeSlider = new SSlider(18.0f, 100.0f);
	AgeSlider->SetValue(25.0f);
	AgeSlider->SetWidth(200.0f);
	AgeSlider->SetFormat("%.0f years");
	GridPanel1->AddSlot(2, 1)
		.SetPadding(5.0f)
		.AttachWidget(AgeSlider);

	// Row 3: Subscribe (ColumnSpan 2로 병합)
	auto SubscribeCheckBox = new SCheckBox("Subscribe to newsletter", false);
	GridPanel1->AddSlot(3, 0)
		.SetColumnSpan(2)
		.SetPadding(5.0f)
		.HAlignment(SGridPanel::FSlot::HAlign_Center)
		.AttachWidget(SubscribeCheckBox);

	// Row 4: Submit 버튼 (ColumnSpan 2로 병합)
	auto SubmitButton = new SButton();
	SubmitButton->SetText("Submit Form");
	SubmitButton->SetSize(FVector2D(150, 35));
	SubmitButton->OnClicked.Add([this]() {
		StatusText->SetText("Form Submitted!");
		StatusText->SetColor(0xFF00FF00);
	});
	GridPanel1->AddSlot(4, 0)
		.SetColumnSpan(2)
		.SetPadding(5.0f, 15.0f, 5.0f, 5.0f)
		.HAlignment(SGridPanel::FSlot::HAlign_Center)
		.AttachWidget(SubmitButton);

	ContentArea->AddSlot()
		.FixedHeight(200.0f)
		.SetPadding(40.0f, 10.0f)
		.AttachWidget(GridPanel1);

	// ===== Phase 3 고급 위젯 테스트 영역 =====

	// 구분선
	auto Separator3 = new SSeparator(SSeparator::Horizontal);
	Separator3->SetThickness(2.0f);
	Separator3->SetColor(0xFFFFCC64);

	ContentArea->AddSlot()
		.FixedHeight(2.0f)
		.SetPadding(20.0f, 20.0f)
		.AttachWidget(Separator3);

	// Phase 3 타이틀
	auto Phase3Title = new STextBlock("Phase 3 Advanced Widgets");
	Phase3Title->SetColor(0xFFFFCC64);
	Phase3Title->SetJustification(STextBlock::Center);

	ContentArea->AddSlot()
		.AutoHeight()
		.SetPadding(20.0f, 10.0f)
		.AttachWidget(Phase3Title);

	// 고급 위젯들을 가로로 배치
	auto AdvancedWidgetsRow = new SHorizontalBox();

	// TreeView 테스트
	auto TreeViewContainer = new SVerticalBox();

	auto TreeViewLabel = new STextBlock("TreeView:");
	TreeViewLabel->SetColor(0xFFCCCCCC);
	TreeViewContainer->AddSlot()
		.AutoHeight()
		.SetPadding(5.0f)
		.AttachWidget(TreeViewLabel);

	TreeView1 = new STreeView();
	auto RootNode = TreeView1->AddRootNode("Project");
	auto SourceNode = RootNode->AddChild("Source");
	SourceNode->AddChild("Main.cpp");
	SourceNode->AddChild("Utils.cpp");
	auto AssetsNode = RootNode->AddChild("Assets");
	AssetsNode->AddChild("Textures");
	AssetsNode->AddChild("Models");
	TreeView1->OnSelectionChanged.Add([this](STreeNode* Node) {
		if (Node)
		{
			StatusText->SetText("Selected: " + Node->GetLabel());
			StatusText->SetColor(0xFF00FFFF);
		}
	});

	TreeViewContainer->AddSlot()
		.FixedHeight(150.0f)
		.SetPadding(5.0f)
		.AttachWidget(TreeView1);

	AdvancedWidgetsRow->AddSlot()
		.FillWidth(1.0f)
		.SetPadding(10.0f)
		.AttachWidget(TreeViewContainer);

	// ListView 테스트
	auto ListViewContainer = new SVerticalBox();

	auto ListViewLabel = new STextBlock("ListView:");
	ListViewLabel->SetColor(0xFFCCCCCC);
	ListViewContainer->AddSlot()
		.AutoHeight()
		.SetPadding(5.0f)
		.AttachWidget(ListViewLabel);

	ListView1 = new SListView();
	ListView1->AddItem("Apple");
	ListView1->AddItem("Banana");
	ListView1->AddItem("Cherry");
	ListView1->AddItem("Date");
	ListView1->AddItem("Elderberry");
	ListView1->SetSelectionMode(SListView::Single);
	ListView1->OnSelectionChanged.Add([this](uint32 Index, const FString& Item) {
		StatusText->SetText("List Selected: " + Item);
		StatusText->SetColor(0xFF00FF00);
	});
	ListView1->OnItemDoubleClicked.Add([this](uint32 Index, const FString& Item) {
		StatusText->SetText("Double-clicked: " + Item);
		StatusText->SetColor(0xFFFF00FF);
	});

	ListViewContainer->AddSlot()
		.FixedHeight(150.0f)
		.SetPadding(5.0f)
		.AttachWidget(ListView1);

	AdvancedWidgetsRow->AddSlot()
		.FillWidth(1.0f)
		.SetPadding(10.0f)
		.AttachWidget(ListViewContainer);

	// ColorPicker 테스트
	auto ColorPickerContainer = new SVerticalBox();

	auto ColorPickerLabel = new STextBlock("Color Picker:");
	ColorPickerLabel->SetColor(0xFFCCCCCC);
	ColorPickerContainer->AddSlot()
		.AutoHeight()
		.SetPadding(5.0f)
		.AttachWidget(ColorPickerLabel);

	ColorPicker1 = new SColorPicker(0xFF0080FF); // Orange
	ColorPicker1->SetPickerMode(SColorPicker::Compact); // Full 모드
	ColorPicker1->SetAlphaEnabled(true);
	//ColorPicker1->SetSize(180.0f, 140.0f); // 원하는 크기 지정 (폭, 높이)
	ColorPicker1->OnColorChanged.Add([this](uint32 NewColor) {
		// 색상을 16진수로 표시
		char Buffer[32];
		sprintf_s(Buffer, "Color: #%08X", NewColor);
		StatusText->SetText(Buffer);
		StatusText->SetColor(NewColor);
	});

	ColorPickerContainer->AddSlot()
		.FixedHeight(140.0f) // 컨테이너 높이도 맞춰줌
		.SetPadding(5.0f)
		.AttachWidget(ColorPicker1);

	AdvancedWidgetsRow->AddSlot()
		.FillWidth(1.0f)
		.SetPadding(10.0f)
		.AttachWidget(ColorPickerContainer);

	ContentArea->AddSlot()
		.FixedHeight(220.0f)
		.SetPadding(20.0f, 10.0f)
		.AttachWidget(AdvancedWidgetsRow);

	// 버튼 행 (수평 배치)
	ButtonRow = new SHorizontalBox();

	auto TestButton1 = new SButton();
	TestButton1->SetText("Button A");
	TestButton1->SetSize(FVector2D(120, 40));
	TestButton1->OnClicked.Add([this]() {
		StatusText->SetText("Button A clicked!");
	});

	auto TestButton2 = new SButton();
	TestButton2->SetText("Button B");
	TestButton2->SetSize(FVector2D(120, 40));
	TestButton2->OnClicked.Add([this]() {
		StatusText->SetText("Button B clicked!");
	});

	auto TestButton3 = new SButton();
	TestButton3->SetText("Button C");
	TestButton3->SetSize(FVector2D(120, 40));
	TestButton3->OnClicked.Add([this]() {
		StatusText->SetText("Button C clicked!");
	});

	ButtonRow->AddSlot()
		.FillWidth(1.0f)
		.AttachWidget(new SPanel()); // Left Spacer

	ButtonRow->AddSlot()
		.AutoWidth()
		.SetPadding(10.0f)
		.AttachWidget(TestButton1);

	ButtonRow->AddSlot()
		.AutoWidth()
		.SetPadding(10.0f)
		.AttachWidget(TestButton2);

	ButtonRow->AddSlot()
		.AutoWidth()
		.SetPadding(10.0f)
		.AttachWidget(TestButton3);

	ButtonRow->AddSlot()
		.FillWidth(1.0f)
		.AttachWidget(new SPanel()); // Right Spacer

	ContentArea->AddSlot()
		.FixedHeight(60.0f)
		.SetPadding(20.0f)
		.AttachWidget(ButtonRow);

	RootLayout->AddSlot()
		.FillHeight(1.0f)
		.AttachWidget(ContentArea);

	// ===== 푸터 (고정 높이) =====
	auto FooterText = new STextBlock("Core Slate v3.0 | Panels: VBox, HBox, Grid, ScrollBox | Basic: Button, Text, Input | Advanced: CheckBox, ComboBox, Slider, TreeView, ListView, ColorPicker");
	FooterText->SetColor(0xFF808080); // 회색
	FooterText->SetJustification(STextBlock::Center);

	RootLayout->AddSlot()
		.FixedHeight(30.0f)
		.SetPadding(10.0f)
		.AttachWidget(FooterText);

	// 초기 렌더링을 위해 Invalidate 호출
	RootLayout->Invalidate();
}

void USlateTestWindow::RenderWidget() const
{
	// Core Slate 레이아웃 렌더링
	if (RootLayout)
	{
		// 매 프레임 Invalidate 호출 (렌더링을 위해 필수!)
		RootLayout->Invalidate();

		// ContentRegion 기준으로 설정
		ImVec2 ContentMin = ImGui::GetCursorScreenPos();
		ImVec2 ContentRegionAvail = ImGui::GetContentRegionAvail();

		RootLayout->SetRect(
			ContentMin.x,
			ContentMin.y,
			ContentMin.x + ContentRegionAvail.x,
			ContentMin.y + ContentRegionAvail.y
		);

		RootLayout->OnRender();
	}

	// // ButtonRow 영역 강제 렌더링 테스트
	// ImGui::Separator();
	// ImGui::Text("Direct ButtonRow Test:");
	// if (ButtonRow)
	// {
	// 	ImVec2 ButtonRowMin = ImGui::GetCursorScreenPos();
	// 	ImVec2 ButtonRowRegion = ImGui::GetContentRegionAvail();
	//
	// 	ButtonRow->SetRect(
	// 		ButtonRowMin.x,
	// 		ButtonRowMin.y,
	// 		ButtonRowMin.x + ButtonRowRegion.x,
	// 		ButtonRowMin.y + 60.0f
	// 	);
	//
	// 	ButtonRow->Invalidate();
	// 	ButtonRow->OnRender();
	//
	// 	ImGui::Dummy(ImVec2(0, 60.0f)); // 공간 확보
	// }
}

void USlateTestWindow::OnButton1Clicked()
{
	ClickCount++;
	StatusText->SetText("Incremented!");
	StatusText->SetColor(0xFF00FF00); // 초록색
}

void USlateTestWindow::OnButton2Clicked()
{
	ClickCount = 0;
	StatusText->SetText("Reset to 0");
	StatusText->SetColor(0xFFFFFF00); // 노란색
}

void USlateTestWindow::OnButton3Clicked()
{
	StatusText->SetText("Test button clicked!");
	StatusText->SetColor(0xFF00FFFF); // 시안색
}
