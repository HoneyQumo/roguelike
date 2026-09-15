#include "pch.h"
#include "ChaseComponent.h"
#include "LevelGrid.h"
#include "LevelLoader.h"
#include "LevelZones.h"
#include "PathService.h"
#include "RoomWakeComponent.h"
#include <AimRotationComponent.h>
#include <GameWorld.h>
#include <MovementComponent.h>
#include <sstream>

using namespace XYZEngine;
using RoguelikeGame::BuildZones;
using RoguelikeGame::ChaseComponent;
using RoguelikeGame::LevelGrid;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::LevelZone;
using RoguelikeGame::PathService;
using RoguelikeGame::RoomWakeComponent;

namespace
{
	constexpr float NOTICES_AT_ONCE = 100.f;
	const std::string HALL =
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"z Zone:vault\n"
		"[map]\n"
		"###########\n"
		"#....z....#\n"
		"#.........#\n"
		"#........z#\n"
		"###########\n";

	const std::string THREE_ROOMS =
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"a Zone:left\n"
		"b Zone:middle\n"
		"c Zone:right\n"
		"[map]\n"
		"###############\n"
		"#a...b....c...#\n"
		"#.............#\n"
		"#...a....b...c#\n"
		"###############\n";

	class TickCounterComponent : public Component
	{
	public:
		TickCounterComponent(GameObject* gameObject) : Component(gameObject) {}

		void Start() override { starts++; }
		void Update(float deltaTime) override { ticks++; }
		void Render() override {}

		int ticks = 0;
		int starts = 0;
	};

	class RoomWakeTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();
			LoadMap(HALL);
		}

		void LoadMap(const std::string& map)
		{
			std::istringstream input(map);
			level = LevelLoader::Parse(input, "wake");
			LevelGrid::SetCurrent(LevelGrid::Build(level));
			PathService::Reset();

			zones = BuildZones(level);
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

		RoomWakeComponent* CreateRooms()
		{
			GameObject* gameObject = GameWorld::Instance()->CreateGameObject("Rooms");
			auto rooms = gameObject->AddComponent<RoomWakeComponent>();
			rooms->SetTargetName("Hero");
			rooms->SetZones(zones);

			return rooms;
		}

		GameObject* CreateHero(int column, int row)
		{
			GameObject* hero = GameWorld::Instance()->CreateGameObject("Hero");
			hero->GetTransform()->SetWorldPosition(At(column, row));

			return hero;
		}

		GameObject* CreateEnemy(int column, int row)
		{
			GameObject* enemy = GameWorld::Instance()->CreateGameObject("Watcher");
			enemy->GetTransform()->SetWorldPosition(At(column, row));
			enemy->GetTransform()->SetWorldRotation(180.f);

			enemy->AddComponent<MovementComponent>()->SetSpeed(120.f);
			enemy->AddComponent<AimRotationComponent>();
			enemy->AddComponent<TickCounterComponent>();

			auto chase = enemy->AddComponent<ChaseComponent>();
			chase->SetTargetName("Hero");
			chase->SetDetectionRadius(600.f);
			chase->SetStopDistance(40.f);
			chase->SetVisionHalfAngle(60.f);
			chase->SetAlertHalfAngle(70.f);
			chase->SetAlertTime(5.f);
			chase->SetSearchTime(4.f);
			chase->SetLook(1.f, 40.f);
			chase->SetAwareness(NOTICES_AT_ONCE, NOTICES_AT_ONCE);

			return enemy;
		}

		void Run(int frames)
		{
			for (int frame = 0; frame < frames; frame++)
			{
				GameWorld::Instance()->Update(0.05f);
			}
		}

		RoguelikeGame::LevelData level;
		std::vector<LevelZone> zones;
	};
}

TEST_F(RoomWakeTest, MapGivesOneZoneOverTheFarSide)
{
	ASSERT_EQ(zones.size(), 1u);
	EXPECT_EQ(zones[0].id, "vault");
	EXPECT_TRUE(zones[0].Contains(7, 2));
	EXPECT_FALSE(zones[0].Contains(1, 2));
}

TEST_F(RoomWakeTest, SleepingEnemySpendsNoFrame)
{
	RoomWakeComponent* rooms = CreateRooms();
	CreateHero(1, 2);
	GameObject* enemy = CreateEnemy(7, 2);
	rooms->AddSleeper("vault", enemy);

	Run(10);

	auto counter = enemy->GetComponent<TickCounterComponent>();
	EXPECT_EQ(counter->ticks, 0);
	EXPECT_EQ(counter->starts, 0);
	EXPECT_FALSE(enemy->IsActive());
}

TEST_F(RoomWakeTest, SleepingEnemyDoesNotChangeState)
{
	RoomWakeComponent* rooms = CreateRooms();
	CreateHero(1, 2);
	GameObject* enemy = CreateEnemy(7, 2);
	rooms->AddSleeper("vault", enemy);
	Vector2Df before = enemy->GetTransform()->GetWorldPosition();

	Run(10);

	EXPECT_FALSE(enemy->GetComponent<ChaseComponent>()->IsChasing());
	EXPECT_EQ(enemy->GetTransform()->GetWorldPosition().x, before.x);
	EXPECT_EQ(enemy->GetTransform()->GetWorldPosition().y, before.y);
	EXPECT_TRUE(rooms->IsAsleep("vault"));
}

TEST_F(RoomWakeTest, EnteringTheZoneWakesTheEnemy)
{
	RoomWakeComponent* rooms = CreateRooms();
	GameObject* hero = CreateHero(1, 2);
	GameObject* enemy = CreateEnemy(7, 2);
	rooms->AddSleeper("vault", enemy);
	Run(5);
	ASSERT_FALSE(enemy->IsActive());

	hero->GetTransform()->SetWorldPosition(At(6, 2));
	Run(5);

	EXPECT_TRUE(enemy->IsActive());
	EXPECT_FALSE(rooms->IsAsleep("vault"));
	EXPECT_GT(enemy->GetComponent<TickCounterComponent>()->ticks, 0);
}

TEST_F(RoomWakeTest, WokenEnemyBehavesAsBefore)
{
	RoomWakeComponent* rooms = CreateRooms();
	GameObject* hero = CreateHero(1, 2);
	GameObject* enemy = CreateEnemy(7, 2);
	rooms->AddSleeper("vault", enemy);
	Run(5);

	hero->GetTransform()->SetWorldPosition(At(6, 2));
	Run(5);

	EXPECT_TRUE(enemy->GetComponent<ChaseComponent>()->IsChasing());
}

TEST_F(RoomWakeTest, ClearedRoomDoesNotFallAsleepAgain)
{
	RoomWakeComponent* rooms = CreateRooms();
	GameObject* hero = CreateHero(1, 2);
	GameObject* enemy = CreateEnemy(7, 2);
	rooms->AddSleeper("vault", enemy);
	Run(5);

	hero->GetTransform()->SetWorldPosition(At(6, 2));
	Run(5);
	ASSERT_TRUE(enemy->IsActive());

	hero->GetTransform()->SetWorldPosition(At(1, 2));
	Run(10);

	EXPECT_TRUE(enemy->IsActive());
	EXPECT_EQ(rooms->GetSleepingCount(), 0);
}

TEST_F(RoomWakeTest, RoomWakesOnlyOnce)
{
	RoomWakeComponent* rooms = CreateRooms();
	GameObject* hero = CreateHero(1, 2);
	GameObject* enemy = CreateEnemy(7, 2);
	rooms->AddSleeper("vault", enemy);

	int woken = 0;
	rooms->SubscribeWoken([&woken](const std::string&) { woken++; });

	hero->GetTransform()->SetWorldPosition(At(6, 2));
	Run(10);

	EXPECT_EQ(woken, 1);
}

TEST_F(RoomWakeTest, AnotherRoomKeepsSleeping)
{
	RoomWakeComponent* rooms = CreateRooms();
	CreateHero(1, 2);
	GameObject* enemy = CreateEnemy(7, 2);
	rooms->AddSleeper("cellar", enemy);

	Run(10);

	EXPECT_FALSE(enemy->IsActive());
	EXPECT_TRUE(rooms->IsAsleep("cellar"));
}

TEST_F(RoomWakeTest, EnemyOutsideAnyZoneIsNeverPutToSleep)
{
	RoomWakeComponent* rooms = CreateRooms();
	CreateHero(1, 2);
	GameObject* enemy = CreateEnemy(2, 2);

	Run(5);

	EXPECT_TRUE(enemy->IsActive());
	EXPECT_EQ(rooms->GetSleepingCount(), 0);
	EXPECT_GT(enemy->GetComponent<TickCounterComponent>()->ticks, 0);
}

TEST_F(RoomWakeTest, NextRoomWakesUpTogetherWithThisOne)
{
	LoadMap(THREE_ROOMS);
	RoomWakeComponent* rooms = CreateRooms();
	rooms->SetAhead(1);
	CreateHero(2, 2);
	GameObject* middle = CreateEnemy(7, 2);
	rooms->AddSleeper("middle", middle);

	Run(3);

	EXPECT_TRUE(middle->IsActive());
	EXPECT_FALSE(rooms->IsAsleep("middle"));
}

TEST_F(RoomWakeTest, RoomTwoDoorsAwayKeepsSleeping)
{
	LoadMap(THREE_ROOMS);
	RoomWakeComponent* rooms = CreateRooms();
	rooms->SetAhead(1);
	CreateHero(2, 2);
	GameObject* right = CreateEnemy(12, 2);
	rooms->AddSleeper("right", right);

	Run(3);

	EXPECT_FALSE(right->IsActive());
	EXPECT_TRUE(rooms->IsAsleep("right"));
}

TEST_F(RoomWakeTest, WalkingOnWakesTheRoomAfterNext)
{
	LoadMap(THREE_ROOMS);
	RoomWakeComponent* rooms = CreateRooms();
	rooms->SetAhead(1);
	GameObject* hero = CreateHero(2, 2);
	GameObject* right = CreateEnemy(12, 2);
	rooms->AddSleeper("right", right);
	Run(3);
	ASSERT_FALSE(right->IsActive());

	hero->GetTransform()->SetWorldPosition(At(7, 2));
	Run(3);

	EXPECT_TRUE(right->IsActive());
}

TEST_F(RoomWakeTest, WithoutLookingAheadOnlyTheOwnRoomWakes)
{
	LoadMap(THREE_ROOMS);
	RoomWakeComponent* rooms = CreateRooms();
	rooms->SetAhead(0);
	CreateHero(2, 2);
	GameObject* middle = CreateEnemy(7, 2);
	rooms->AddSleeper("middle", middle);

	Run(3);

	EXPECT_FALSE(middle->IsActive());
}
