#include "pch.h"
#include "GameWorld.h"
#include "RenderSystem.h"
#include "UiButton.h"
#include "UiPanel.h"
#include "UiRootComponent.h"
#include "UiScreen.h"

using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::RenderSystem;
using XYZEngine::UiAnchor;
using XYZEngine::UiButton;
using XYZEngine::UiPanel;
using XYZEngine::UiRootComponent;
using XYZEngine::UiScreen;

namespace
{
	class TestScreen : public UiScreen
	{
	public:
		TestScreen()
		{
			panel = GetRoot().AddChild<UiPanel>();
			panel->SetStretch(true, true);
		}

		UiPanel* panel = nullptr;
	};

	class UiScreenTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();
			RenderSystem::Instance()->HandleResize(1280, 720);
		}

		void TearDown() override
		{
			GameWorld::Instance()->Clear();
			RenderSystem::Instance()->HandleResize(1280, 720);
		}
	};
}

TEST_F(UiScreenTest, ScreenIsLaidOutRightOnAdd)
{
	TestScreen screen;
	EXPECT_FLOAT_EQ(screen.panel->GetBounds().width, 0.f);

	GameObject* ui = GameWorld::Instance()->CreateGameObject("Ui");
	ui->AddComponent<UiRootComponent>()->AddScreen(&screen);

	EXPECT_FLOAT_EQ(screen.panel->GetBounds().width, 1280.f);
	EXPECT_FLOAT_EQ(screen.panel->GetBounds().height, 720.f);
}

TEST_F(UiScreenTest, ResizeRelayoutsScreensWithoutWorldUpdate)
{
	TestScreen screen;
	GameObject* ui = GameWorld::Instance()->CreateGameObject("Ui");
	ui->AddComponent<UiRootComponent>()->AddScreen(&screen);

	RenderSystem::Instance()->HandleResize(800, 600);

	EXPECT_FLOAT_EQ(screen.panel->GetBounds().width, 800.f);
	EXPECT_FLOAT_EQ(screen.panel->GetBounds().height, 600.f);
}

TEST_F(UiScreenTest, DestroyedRootStopsListeningToResize)
{
	TestScreen screen;
	GameObject* ui = GameWorld::Instance()->CreateGameObject("Ui");
	ui->AddComponent<UiRootComponent>()->AddScreen(&screen);

	GameWorld::Instance()->Clear();
	RenderSystem::Instance()->HandleResize(640, 480);

	EXPECT_FLOAT_EQ(screen.panel->GetBounds().width, 1280.f);
}

TEST_F(UiScreenTest, HiddenScreenKeepsItsLayout)
{
	TestScreen screen;
	GameObject* ui = GameWorld::Instance()->CreateGameObject("Ui");
	ui->AddComponent<UiRootComponent>()->AddScreen(&screen);

	screen.SetVisible(false);

	EXPECT_FALSE(screen.IsVisible());
	EXPECT_FLOAT_EQ(screen.panel->GetBounds().width, 1280.f);
}

TEST_F(UiScreenTest, ButtonGoesThroughHoverPressAndClicks)
{
	UiScreen screen;
	UiButton* button = screen.GetRoot().AddChild<UiButton>();
	button->SetAnchor(UiAnchor::TopLeft);
	button->SetPivot(UiAnchor::TopLeft);
	button->SetOffset({100.f, 100.f});
	button->SetSize({200.f, 60.f});
	screen.Resize({1280.f, 720.f});

	int clicks = 0;
	button->SetOnClick([&clicks]() { clicks++; });

	EXPECT_FALSE(button->HandlePointer({10.f, 10.f}, false, false));
	EXPECT_EQ(button->GetState(), UiButton::State::Normal);

	EXPECT_TRUE(button->HandlePointer({150.f, 120.f}, false, false));
	EXPECT_EQ(button->GetState(), UiButton::State::Hovered);

	button->HandlePointer({150.f, 120.f}, true, false);
	EXPECT_EQ(button->GetState(), UiButton::State::Pressed);

	button->HandlePointer({150.f, 120.f}, false, true);
	EXPECT_EQ(button->GetState(), UiButton::State::Hovered);
	EXPECT_EQ(clicks, 1);
}

TEST_F(UiScreenTest, ButtonDoesNotClickWhenReleasedOutside)
{
	UiScreen screen;
	UiButton* button = screen.GetRoot().AddChild<UiButton>();
	button->SetAnchor(UiAnchor::TopLeft);
	button->SetPivot(UiAnchor::TopLeft);
	button->SetSize({200.f, 60.f});
	screen.Resize({1280.f, 720.f});

	int clicks = 0;
	button->SetOnClick([&clicks]() { clicks++; });

	button->HandlePointer({50.f, 30.f}, true, false);
	button->HandlePointer({500.f, 500.f}, false, true);

	EXPECT_EQ(clicks, 0);
	EXPECT_EQ(button->GetState(), UiButton::State::Normal);
}

TEST_F(UiScreenTest, ButtonBuildsBackgroundAndLabel)
{
	UiScreen screen;
	UiButton* button = screen.GetRoot().AddChild<UiButton>();
	button->SetSize({120.f, 40.f});
	screen.Resize({640.f, 480.f});

	ASSERT_NE(button->GetLabel(), nullptr);
	EXPECT_EQ(button->GetChildrenCount(), 2u);
	EXPECT_FLOAT_EQ(button->GetLabel()->GetBounds().width, 120.f);
}
