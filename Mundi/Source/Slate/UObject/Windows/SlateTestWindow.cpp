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
		.FixedHeight(40.0f)  // Fixed 사용
		.SetPadding(10.0f)
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
		.AutoWidth()
		.SetPadding(5.0f)
		.AttachWidget(Button1);

	Toolbar->AddSlot()
		.AutoWidth()
		.SetPadding(5.0f)
		.AttachWidget(Button2);

	Toolbar->AddSlot()
		.AutoWidth()
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
	auto FooterText = new STextBlock("Core Slate v1.0 | SVerticalBox + SHorizontalBox + SButton + STextBlock");
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
