#include "pch.h"
#include "GameWorld.h"
#include "RenderSystem.h"
#include "UiButton.h"
#include "UiManager.h"
#include "UiPanel.h"
#include "UiScreen.h"

using XYZEngine::GameWorld;
using XYZEngine::RenderSystem;
using XYZEngine::UiAnchor;
using XYZEngine::UiButton;
using XYZEngine::UiManager;
using XYZEngine::UiPanel;
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

	class ButtonScreen : public UiScreen
	{
	public:
		ButtonScreen()
		{
			button = GetRoot().AddChild<UiButton>();
			button->SetAnchor(UiAnchor::TopLeft);
			button->SetPivot(UiAnchor::TopLeft);
			button->SetOffset({100.f, 100.f});
			button->SetSize({200.f, 60.f});
			button->SetOnClick([this]() { clicks++; });
		}

		UiButton* button = nullptr;
		int clicks = 0;
	};

	class UiScreenTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();
			UiManager::Instance()->Clear();
			RenderSystem::Instance()->HandleResize(1280, 720);
		}

		void TearDown() override
		{
			UiManager::Instance()->Clear();
			GameWorld::Instance()->Clear();
			RenderSystem::Instance()->HandleResize(1280, 720);
		}
	};
}

TEST_F(UiScreenTest, PushedScreenIsLaidOutRightAway)
{
	TestScreen screen;
	EXPECT_FLOAT_EQ(screen.panel->GetBounds().width, 0.f);

	UiManager::Instance()->Push(&screen);

	EXPECT_EQ(UiManager::Instance()->GetTop(), &screen);
	EXPECT_FLOAT_EQ(screen.panel->GetBounds().width, 1280.f);
	EXPECT_FLOAT_EQ(screen.panel->GetBounds().height, 720.f);
}

TEST_F(UiScreenTest, ResizeRelayoutsEveryScreenInTheStack)
{
	TestScreen bottom;
	TestScreen top;
	UiManager::Instance()->Push(&bottom);
	UiManager::Instance()->Push(&top);

	RenderSystem::Instance()->HandleResize(800, 600);

	EXPECT_FLOAT_EQ(bottom.panel->GetBounds().width, 800.f);
	EXPECT_FLOAT_EQ(top.panel->GetBounds().height, 600.f);
}

TEST_F(UiScreenTest, PopRemovesOnlyTheTopScreen)
{
	TestScreen bottom;
	TestScreen top;
	UiManager::Instance()->Push(&bottom);
	UiManager::Instance()->Push(&top);

	UiManager::Instance()->Pop();

	EXPECT_EQ(UiManager::Instance()->GetCount(), 1u);
	EXPECT_EQ(UiManager::Instance()->GetTop(), &bottom);
}

TEST_F(UiScreenTest, PushingTheSameScreenTwiceKeepsOneEntry)
{
	TestScreen screen;
	UiManager::Instance()->Push(&screen);
	UiManager::Instance()->Push(&screen);

	EXPECT_EQ(UiManager::Instance()->GetCount(), 1u);
}

TEST_F(UiScreenTest, OnlyTopScreenGetsPointer)
{
	ButtonScreen bottom;
	ButtonScreen top;
	UiManager::Instance()->Push(&bottom);
	UiManager::Instance()->Push(&top);

	top.HandlePointer({150.f, 120.f}, true, false);
	top.HandlePointer({150.f, 120.f}, false, true);

	EXPECT_EQ(top.clicks, 1);
	EXPECT_EQ(bottom.clicks, 0);
	EXPECT_EQ(bottom.button->GetState(), UiButton::State::Normal);
}

TEST_F(UiScreenTest, ScreenReportsWhetherPointerWasUsed)
{
	ButtonScreen screen;
	UiManager::Instance()->Push(&screen);

	EXPECT_TRUE(screen.HandlePointer({150.f, 120.f}, false, false));
	EXPECT_FALSE(screen.HandlePointer({10.f, 10.f}, false, false));
}

TEST_F(UiScreenTest, HiddenScreenKeepsItsLayout)
{
	TestScreen screen;
	UiManager::Instance()->Push(&screen);

	screen.SetVisible(false);

	EXPECT_FALSE(screen.IsVisible());
	EXPECT_FLOAT_EQ(screen.panel->GetBounds().width, 1280.f);
}

TEST_F(UiScreenTest, ClearedManagerForgetsEveryScreen)
{
	TestScreen first;
	TestScreen second;
	UiManager::Instance()->Push(&first);
	UiManager::Instance()->Push(&second);

	UiManager::Instance()->Clear();

	EXPECT_EQ(UiManager::Instance()->GetCount(), 0u);
	EXPECT_EQ(UiManager::Instance()->GetTop(), nullptr);
	EXPECT_FALSE(UiManager::Instance()->IsPointerCaptured());
}

TEST_F(UiScreenTest, ButtonGoesThroughHoverPressAndClicks)
{
	ButtonScreen screen;
	UiManager::Instance()->Push(&screen);

	EXPECT_FALSE(screen.HandlePointer({10.f, 10.f}, false, false));
	EXPECT_EQ(screen.button->GetState(), UiButton::State::Normal);

	EXPECT_TRUE(screen.HandlePointer({150.f, 120.f}, false, false));
	EXPECT_EQ(screen.button->GetState(), UiButton::State::Hovered);

	screen.HandlePointer({150.f, 120.f}, true, false);
	EXPECT_EQ(screen.button->GetState(), UiButton::State::Pressed);

	screen.HandlePointer({150.f, 120.f}, false, true);
	EXPECT_EQ(screen.button->GetState(), UiButton::State::Hovered);
	EXPECT_EQ(screen.clicks, 1);
}

TEST_F(UiScreenTest, ButtonDoesNotClickWhenReleasedOutside)
{
	ButtonScreen screen;
	UiManager::Instance()->Push(&screen);

	screen.HandlePointer({150.f, 120.f}, true, false);
	screen.HandlePointer({900.f, 500.f}, false, true);

	EXPECT_EQ(screen.clicks, 0);
	EXPECT_EQ(screen.button->GetState(), UiButton::State::Normal);
}

TEST_F(UiScreenTest, TopmostChildTakesThePointerFirst)
{
	UiScreen screen;
	UiButton* under = screen.GetRoot().AddChild<UiButton>();
	under->SetAnchor(UiAnchor::TopLeft);
	under->SetPivot(UiAnchor::TopLeft);
	under->SetSize({200.f, 200.f});

	UiButton* over = screen.GetRoot().AddChild<UiButton>();
	over->SetAnchor(UiAnchor::TopLeft);
	over->SetPivot(UiAnchor::TopLeft);
	over->SetSize({200.f, 200.f});

	UiManager::Instance()->Push(&screen);

	int underClicks = 0;
	int overClicks = 0;
	under->SetOnClick([&underClicks]() { underClicks++; });
	over->SetOnClick([&overClicks]() { overClicks++; });

	screen.HandlePointer({50.f, 50.f}, true, false);
	screen.HandlePointer({50.f, 50.f}, false, true);

	EXPECT_EQ(overClicks, 1);
	EXPECT_EQ(underClicks, 0);
}
