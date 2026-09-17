#include "pch.h"
#include <GameWorld.h>
#include <VertexArrayRendererComponent.h>

using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::VertexArrayRendererComponent;
using XYZEngine::Vector2Df;

namespace
{
	class VertexArrayBoundsTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();
			owner = GameWorld::Instance()->CreateGameObject("Chunk");
			renderer = owner->AddComponent<VertexArrayRendererComponent>();
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		GameObject* owner = nullptr;
		VertexArrayRendererComponent* renderer = nullptr;
	};
}

TEST_F(VertexArrayBoundsTest, AnEmptyArrayCoversNothing)
{
	sf::FloatRect bounds = renderer->GetBounds();

	EXPECT_FLOAT_EQ(bounds.width, 0.f);
	EXPECT_FLOAT_EQ(bounds.height, 0.f);
}

TEST_F(VertexArrayBoundsTest, OneQuadIsCoveredExactly)
{
	renderer->AddQuad({100.f, 200.f}, {64.f, 64.f}, sf::Color::White);

	sf::FloatRect bounds = renderer->GetBounds();

	EXPECT_FLOAT_EQ(bounds.left, 68.f);
	EXPECT_FLOAT_EQ(bounds.top, 168.f);
	EXPECT_FLOAT_EQ(bounds.width, 64.f);
	EXPECT_FLOAT_EQ(bounds.height, 64.f);
}

TEST_F(VertexArrayBoundsTest, TheBoxGrowsOverEveryQuad)
{
	renderer->AddQuad({0.f, 0.f}, {64.f, 64.f}, sf::Color::White);
	renderer->AddQuad({640.f, 320.f}, {64.f, 64.f}, sf::Color::White);

	sf::FloatRect bounds = renderer->GetBounds();

	EXPECT_FLOAT_EQ(bounds.left, -32.f);
	EXPECT_FLOAT_EQ(bounds.top, -32.f);
	EXPECT_FLOAT_EQ(bounds.width, 704.f);
	EXPECT_FLOAT_EQ(bounds.height, 384.f);
}

TEST_F(VertexArrayBoundsTest, ACornerQuadIsCoveredByAllFourCorners)
{
	renderer->AddQuad({0.f, 0.f}, {10.f, -20.f}, {30.f, 40.f}, {-10.f, 5.f}, sf::Color::White);

	sf::FloatRect bounds = renderer->GetBounds();

	EXPECT_FLOAT_EQ(bounds.left, -10.f);
	EXPECT_FLOAT_EQ(bounds.top, -20.f);
	EXPECT_FLOAT_EQ(bounds.width, 40.f);
	EXPECT_FLOAT_EQ(bounds.height, 60.f);
}

TEST_F(VertexArrayBoundsTest, ClearingForgetsTheBox)
{
	renderer->AddQuad({1000.f, 1000.f}, {64.f, 64.f}, sf::Color::White);
	renderer->Clear();

	sf::FloatRect bounds = renderer->GetBounds();

	EXPECT_FLOAT_EQ(bounds.width, 0.f);
	EXPECT_FLOAT_EQ(bounds.height, 0.f);

	renderer->AddQuad({0.f, 0.f}, {64.f, 64.f}, sf::Color::White);

	EXPECT_FLOAT_EQ(renderer->GetBounds().left, -32.f) << "the box kept what was cleared away";
}
