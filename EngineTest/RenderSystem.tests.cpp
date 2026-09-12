#include "pch.h"
#include "RenderSystem.h"

using XYZEngine::RenderSystem;
using XYZEngine::SubscriptionId;

namespace
{
	class RenderSystemTest : public ::testing::Test
	{
	protected:
		RenderSystem* render = RenderSystem::Instance();

		void SetUp() override { render->HandleResize(1280, 720); }
		void TearDown() override { render->HandleResize(1280, 720); }
	};
}

TEST_F(RenderSystemTest, ResizeUpdatesWindowSize)
{
	render->HandleResize(1024, 768);

	EXPECT_EQ(render->GetWindowSize().x, 1024u);
	EXPECT_EQ(render->GetWindowSize().y, 768u);
}

TEST_F(RenderSystemTest, UiViewMatchesWindowInScreenPixels)
{
	render->HandleResize(800, 600);

	const sf::View& uiView = render->GetUiView();
	EXPECT_FLOAT_EQ(uiView.getSize().x, 800.f);
	EXPECT_FLOAT_EQ(uiView.getSize().y, 600.f);
	EXPECT_FLOAT_EQ(uiView.getCenter().x, 400.f);
	EXPECT_FLOAT_EQ(uiView.getCenter().y, 300.f);
}

TEST_F(RenderSystemTest, SubscriberGetsTheNewSize)
{
	unsigned int seenWidth = 0;
	unsigned int seenHeight = 0;
	SubscriptionId id = render->SubscribeResize([&seenWidth, &seenHeight](unsigned int width, unsigned int height)
	{
		seenWidth = width;
		seenHeight = height;
	});

	render->HandleResize(640, 480);

	EXPECT_EQ(seenWidth, 640u);
	EXPECT_EQ(seenHeight, 480u);

	render->UnsubscribeResize(id);
	render->HandleResize(320, 240);

	EXPECT_EQ(seenWidth, 640u);
	EXPECT_EQ(seenHeight, 480u);
}
