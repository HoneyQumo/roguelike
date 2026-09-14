#include "pch.h"
#include "GameWorld.h"
#include "PropCatalog.h"
#include "PropVisualComponent.h"
#include "RectangleRendererComponent.h"
#include <sstream>

using RoguelikeGame::PropCatalog;
using RoguelikeGame::PropDefinition;
using RoguelikeGame::PropVisualComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;

namespace
{
	PropCatalog ParseProps(const std::string& text)
	{
		std::istringstream input(text);

		return PropCatalog::Parse(input, "props.config");
	}

	class PropVisualTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			prop = GameWorld::Instance()->CreateGameObject("Prop_crate");
			renderer = prop->AddComponent<XYZEngine::RectangleRendererComponent>();
			renderer->SetSize(48.f, 48.f);
			renderer->SetColor({150, 110, 60});
			visual = prop->AddComponent<PropVisualComponent>();
			visual->SetSize(48.f);

			GameWorld::Instance()->Update(0.016f);
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		GameObject* prop = nullptr;
		XYZEngine::RectangleRendererComponent* renderer = nullptr;
		PropVisualComponent* visual = nullptr;
	};
}

TEST_F(PropVisualTest, FreshPropIsNotSpent)
{
	EXPECT_FALSE(visual->IsSpent());
	EXPECT_EQ(renderer->GetColor().r, 150);
}

TEST_F(PropVisualTest, WithoutAFrameTheColorCarriesTheSpentLook)
{
	visual->SetSpentColor({80, 60, 35});

	visual->ShowSpent();

	EXPECT_TRUE(visual->IsSpent());
	EXPECT_EQ(renderer->GetColor().r, 80);
	EXPECT_EQ(renderer->GetColor().g, 60);
	EXPECT_EQ(renderer->GetColor().b, 35);
}

TEST_F(PropVisualTest, SecondCallChangesNothing)
{
	visual->SetSpentColor({80, 60, 35});
	visual->ShowSpent();

	visual->SetSpentColor({10, 10, 10});
	visual->ShowSpent();

	EXPECT_EQ(renderer->GetColor().r, 80);
}

TEST(PropCatalogFrameTest, FrameAndSpentFrameAreParsed)
{
	PropCatalog catalog = ParseProps(
		"[prop crate_wood]\n"
		"name Crate\n"
		"size 48\n"
		"frame Resources/Textures/props.png 0 0 64 64\n"
		"spentFrame Resources/Textures/props.png 64 0 64 64\n");

	const PropDefinition* crate = catalog.Find("crate_wood");

	ASSERT_NE(crate, nullptr);
	EXPECT_TRUE(crate->HasFrame());
	EXPECT_TRUE(crate->HasSpentFrame());
	EXPECT_EQ(crate->texturePath, "Resources/Textures/props.png");
	EXPECT_EQ(crate->frame.left, 0);
	EXPECT_EQ(crate->spentFrame.left, 64);
	EXPECT_EQ(crate->spentFrame.width, 64);
}

TEST(PropCatalogFrameTest, PropWithoutAFrameKeepsWorking)
{
	PropCatalog catalog = ParseProps(
		"[prop barrel]\n"
		"name Barrel\n"
		"size 44\n"
		"color 120 90 70\n");

	const PropDefinition* barrel = catalog.Find("barrel");

	ASSERT_NE(barrel, nullptr);
	EXPECT_FALSE(barrel->HasFrame());
	EXPECT_FALSE(barrel->HasSpentFrame());
}

TEST(PropCatalogFrameTest, FrameWithoutASpentFrameIsAllowed)
{
	PropCatalog catalog = ParseProps(
		"[prop barrel]\n"
		"name Barrel\n"
		"size 44\n"
		"frame Resources/Textures/props.png 128 0 64 64\n");

	const PropDefinition* barrel = catalog.Find("barrel");

	ASSERT_NE(barrel, nullptr);
	EXPECT_TRUE(barrel->HasFrame());
	EXPECT_FALSE(barrel->HasSpentFrame());
}

TEST(PropCatalogFrameTest, BrokenFrameLineIsRejected)
{
	EXPECT_THROW(ParseProps("[prop crate]\nframe Resources/Textures/props.png 0 0 64\n"), std::runtime_error);
	EXPECT_THROW(ParseProps("[prop crate]\nframe Resources/Textures/props.png 0 0 0 64\n"), std::runtime_error);
}
