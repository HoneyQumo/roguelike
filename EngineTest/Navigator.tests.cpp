#include "pch.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include "LevelLoader.h"
#include "Navigator.h"
#include "PathService.h"
#include <BoxColliderComponent.h>
#include <GameWorld.h>
#include <MovementComponent.h>
#include <RigidbodyComponent.h>
#include <sstream>

using namespace XYZEngine;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelGrid;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::Navigator;
using RoguelikeGame::PathService;
using RoguelikeGame::TILE_SIZE;

namespace
{
	const std::string DOORWAY =
		"[map]\n"
		"#######\n"
		"#.....#\n"
		"###.###\n"
		"#.....#\n"
		"#######\n";

	constexpr float STEP = 0.02f;
	constexpr float SPEED = 150.f;

	class NavigatorTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			std::istringstream input(DOORWAY);
			LevelData level = LevelLoader::Parse(input, "doorway");
			LevelGrid::SetCurrent(LevelGrid::Build(level));
			PathService::Reset();
		}

		void TearDown() override
		{
			GameWorld::Instance()->Clear();
			LevelGrid::SetCurrent(LevelGrid());
			PathService::Reset();
		}

		Vector2Df At(int column, int row) const
		{
			return LevelGrid::Current().ToWorld(column, row);
		}

		GameObject* CreateWalker(const Vector2Df& at, float size)
		{
			GameObject* walker = GameWorld::Instance()->CreateGameObject("Walker");
			walker->GetTransform()->SetWorldPosition(at);
			walker->AddComponent<MovementComponent>()->SetSpeed(SPEED);
			walker->AddComponent<RigidbodyComponent>()->SetKinematic(false);
			walker->AddComponent<BoxColliderComponent>()->SetSize(size, size);

			return walker;
		}

		void CreateWall(int column, int row)
		{
			GameObject* wall = GameWorld::Instance()->CreateGameObject("Wall");
			wall->GetTransform()->SetWorldPosition(At(column, row));
			wall->AddComponent<BoxColliderComponent>()->SetSize(TILE_SIZE, TILE_SIZE);
		}

		void BuildWalls()
		{
			const LevelGrid& grid = LevelGrid::Current();
			for (int row = 0; row < grid.GetHeight(); row++)
			{
				for (int column = 0; column < grid.GetWidth(); column++)
				{
					if (grid.GetCell(column, row) == RoguelikeGame::LevelCell::Wall)
					{
						CreateWall(column, row);
					}
				}
			}
		}

		float Travel(GameObject* walker, Navigator& navigator, const Vector2Df& goal, float seconds)
		{
			auto movement = walker->GetComponent<MovementComponent>();
			float best = (goal - walker->GetTransform()->GetWorldPosition()).GetLength();

			for (float passed = 0.f; passed < seconds; passed += STEP)
			{
				Vector2Df position = walker->GetTransform()->GetWorldPosition();
				movement->SetDirection(navigator.Steer(position, goal, SPEED, STEP));

				GameWorld::Instance()->Update(STEP);
				GameWorld::Instance()->UpdatePhysics();

				best = std::min(best, (goal - walker->GetTransform()->GetWorldPosition()).GetLength());
			}

			return best;
		}
	};
}

TEST_F(NavigatorTest, OpenWayIsWalkedStraight)
{
	BuildWalls();
	GameObject* walker = CreateWalker(At(1, 1), 30.f);
	Navigator navigator;

	EXPECT_LT(Travel(walker, navigator, At(5, 1), 4.f), RoguelikeGame::ENEMY_ROUTE_ARRIVE_DISTANCE);
	EXPECT_FALSE(navigator.IsOnRoute());
}

TEST_F(NavigatorTest, WideBodySqueezesThroughTheDoorway)
{
	BuildWalls();
	GameObject* walker = CreateWalker(At(1, 3), 58.f);
	Navigator navigator;

	EXPECT_LT(Travel(walker, navigator, At(5, 1), 8.f), RoguelikeGame::ENEMY_ROUTE_ARRIVE_DISTANCE);
}

TEST_F(NavigatorTest, BodyPressedIntoAWallIsNoticed)
{
	BuildWalls();
	GameObject* walker = CreateWalker(At(1, 3), 58.f);
	Navigator navigator;

	Travel(walker, navigator, At(5, 1), 1.5f);

	EXPECT_TRUE(navigator.IsOnRoute());
}

TEST_F(NavigatorTest, ResetForgetsTheRoute)
{
	BuildWalls();
	GameObject* walker = CreateWalker(At(1, 3), 58.f);
	Navigator navigator;
	Travel(walker, navigator, At(5, 1), 1.5f);
	ASSERT_TRUE(navigator.IsOnRoute());

	navigator.Reset();

	EXPECT_FALSE(navigator.IsOnRoute());
	EXPECT_FALSE(navigator.IsBlocked());
}

TEST_F(NavigatorTest, WalkerPushedIntoAWallGetsOutAndCarriesOn)
{
	BuildWalls();
	GameObject* walker = CreateWalker(At(2, 2), 30.f);
	Navigator navigator;

	Vector2Df goal = At(5, 3);
	float left = Travel(walker, navigator, goal, 6.f);

	EXPECT_LT(left, RoguelikeGame::ENEMY_ROUTE_ARRIVE_DISTANCE);
}

TEST_F(NavigatorTest, BlockedWalkerStopsBeingBlockedOnceItMoves)
{
	BuildWalls();
	GameObject* walker = CreateWalker(At(1, 1), 30.f);
	Navigator navigator;

	Travel(walker, navigator, At(5, 1), 3.f);

	EXPECT_FALSE(navigator.IsBlocked());
}
