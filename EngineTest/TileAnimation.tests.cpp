#include "pch.h"
#include "GameSettings.h"
#include "GameWorld.h"
#include "TileAnimationComponent.h"
#include "TileAtlas.h"
#include "VertexArrayRendererComponent.h"
#include <chrono>
#include <iostream>

using RoguelikeGame::TileAnimationComponent;
using RoguelikeGame::TileFrameRect;
using RoguelikeGame::TILE_FLOOR_FRAMES;
using RoguelikeGame::TILE_WATER_ROW;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::VertexArrayRendererComponent;

namespace
{
	constexpr float FRAME_TIME = 0.2f;

	// Столько воды примерно лежит на мосту: 704 клетки в длину, 12 рядов.
	constexpr int BRIDGE_WATER = 704 * 12;
	constexpr int FRAMES = 600;

	class TileAnimationTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			GameObject* tiles = GameWorld::Instance()->CreateGameObject("LevelTiles");
			renderer = tiles->AddComponent<VertexArrayRendererComponent>();
			water = tiles->AddComponent<TileAnimationComponent>();
			water->SetRenderer(renderer);
			water->SetStrip(TILE_WATER_ROW, TILE_FLOOR_FRAMES, FRAME_TIME);
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		void Fill(int count)
		{
			for (int index = 0; index < count; index++)
			{
				renderer->AddQuad({index * 64.f, 0.f}, {64.f, 64.f}, TileFrameRect(TILE_WATER_ROW, 0));
				water->AddCell(static_cast<std::size_t>(index), static_cast<unsigned int>(index));
			}
		}

		float TopOf(int quad) const
		{
			return renderer->GetVertices()[static_cast<std::size_t>(quad) * 4u + 3u].texCoords.y;
		}

		float LeftOf(int quad) const
		{
			return renderer->GetVertices()[static_cast<std::size_t>(quad) * 4u].texCoords.x;
		}

		VertexArrayRendererComponent* renderer = nullptr;
		TileAnimationComponent* water = nullptr;
	};
}

TEST_F(TileAnimationTest, WithoutCellsNothingMoves)
{
	GameWorld::Instance()->Update(1.f);

	EXPECT_EQ(water->GetStep(), 0);
}

TEST_F(TileAnimationTest, TheStripAdvancesOnceItsFrameTimeIsUp)
{
	Fill(4);

	GameWorld::Instance()->Update(0.1f);
	EXPECT_EQ(water->GetStep(), 0) << "moved early";

	GameWorld::Instance()->Update(0.15f);

	EXPECT_EQ(water->GetStep(), 1);
}

TEST_F(TileAnimationTest, FramesGoRoundAndComeBack)
{
	Fill(2);

	for (int tick = 0; tick < TILE_FLOOR_FRAMES; tick++)
	{
		GameWorld::Instance()->Update(FRAME_TIME);
	}

	EXPECT_EQ(water->GetStep(), 0) << "the strip did not loop";
}

TEST_F(TileAnimationTest, ALongStallDoesNotRewindTheWater)
{
	Fill(2);

	GameWorld::Instance()->Update(FRAME_TIME * 2.5f);

	EXPECT_EQ(water->GetStep(), 2) << "a stall should skip ahead, not unwind";
}

TEST_F(TileAnimationTest, FramesStayInTheWaterRow)
{
	Fill(3);

	GameWorld::Instance()->Update(FRAME_TIME);

	for (int quad = 0; quad < 3; quad++)
	{
		EXPECT_FLOAT_EQ(TopOf(quad), static_cast<float>(TILE_WATER_ROW * RoguelikeGame::TILE_FRAME_SIZE) + 0.5f)
			<< "quad " << quad << " left its row";
	}
}

TEST_F(TileAnimationTest, NeighboursDoNotBlinkTogether)
{
	Fill(4);

	GameWorld::Instance()->Update(FRAME_TIME);

	EXPECT_NE(LeftOf(0), LeftOf(1)) << "the whole river blinks as one";
}

TEST_F(TileAnimationTest, BenchmarkBridgeSizedWater)
{
	Fill(BRIDGE_WATER);

	auto started = std::chrono::steady_clock::now();
	for (int frame = 0; frame < FRAMES; frame++)
	{
		GameWorld::Instance()->Update(0.016f);
	}
	auto elapsed = std::chrono::steady_clock::now() - started;

	double perFrame = std::chrono::duration<double, std::milli>(elapsed).count() / FRAMES;
	std::cout << "water animation over " << BRIDGE_WATER << " tiles: " << perFrame << " ms per frame" << std::endl;
}
