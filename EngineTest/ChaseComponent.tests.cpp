#include "pch.h"
#include "Awareness.h"
#include "FactionComponent.h"
#include "Noise.h"
#include "AwarenessGaugeComponent.h"
#include "ChaseComponent.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include "LevelLoader.h"
#include "PathService.h"
#include <AimRotationComponent.h>
#include <GameWorld.h>
#include <MovementComponent.h>
#include "DoorComponent.h"
#include "EnemyAttackComponent.h"
#include "InventoryComponent.h"
#include <BoxColliderComponent.h>
#include <sstream>

using namespace XYZEngine;
using RoguelikeGame::ChaseComponent;
using RoguelikeGame::DoorComponent;
using RoguelikeGame::InventoryComponent;
using RoguelikeGame::ItemDefinition;
using RoguelikeGame::ItemEffectKind;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelGrid;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::PathService;

namespace
{
	const std::string ROOM =
		"[map]\n"
		"#############\n"
		"#...#.......#\n"
		"#...........#\n"
		"#...#.......#\n"
		"#############\n";

	const std::string DOORWAY =
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"+ Door:door_exit\n"
		"[map]\n"
		"#######\n"
		"#..+..#\n"
		"#######\n";

	constexpr float SEARCH_TIME = 4.f;
	constexpr float NOTICES_AT_ONCE = 100.f;
	constexpr float STEP = 0.05f;
	constexpr float LOOK_TIME = 1.f;

	class ChaseComponentTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();
			LoadMap(ROOM);
		}

		void LoadMap(const std::string& map)
		{
			std::istringstream input(map);
			LevelData level = LevelLoader::Parse(input, "chase");
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

		GameObject* CreateHero(int column, int row)
		{
			GameObject* hero = GameWorld::Instance()->CreateGameObject("Hero");
			hero->GetTransform()->SetWorldPosition(At(column, row));

			return hero;
		}

		ChaseComponent* CreateEnemy(int column, int row)
		{
			GameObject* enemy = GameWorld::Instance()->CreateGameObject("Watcher");
			enemy->GetTransform()->SetWorldPosition(At(column, row));
			enemy->GetTransform()->SetWorldRotation(0.f);

			enemy->AddComponent<MovementComponent>()->SetSpeed(120.f);
			enemy->AddComponent<AimRotationComponent>();

			auto chase = enemy->AddComponent<ChaseComponent>();
			chase->SetTargetName("Hero");
			chase->SetDetectionRadius(400.f);
			chase->SetStopDistance(40.f);
			chase->SetVisionHalfAngle(45.f);
			chase->SetAlertHalfAngle(60.f);
			chase->SetAlertTime(5.f);
			chase->SetSearchTime(SEARCH_TIME);
			chase->SetLook(LOOK_TIME, 40.f);
			chase->SetAwareness(NOTICES_AT_ONCE, NOTICES_AT_ONCE);

			return chase;
		}

		GameObject* CreateHeroWithKey(int column, int row)
		{
			GameObject* hero = CreateHero(column, row);
			hero->AddComponent<InventoryComponent>()->SetCapacity(4);
			hero->GetComponent<InventoryComponent>()->TryAdd(rustyKey);

			return hero;
		}

		DoorComponent* CreateDoor(int column, int row)
		{
			GameObject* door = GameWorld::Instance()->CreateGameObject("Door");
			door->GetTransform()->SetWorldPosition(At(column, row));
			door->AddComponent<BoxColliderComponent>()->SetSize(RoguelikeGame::TILE_SIZE, RoguelikeGame::TILE_SIZE);

			auto component = door->AddComponent<DoorComponent>();
			component->SetDoorId("door_exit");

			return component;
		}

		ItemDefinition MakeRustyKey() const
		{
			ItemDefinition item;
			item.id = "key_rusty";
			item.name = "key_rusty";
			item.effect.kind = ItemEffectKind::Unlock;
			item.effect.target = "door_exit";

			return item;
		}

		ItemDefinition rustyKey = MakeRustyKey();

		void Run(float seconds)
		{
			for (float passed = 0.f; passed < seconds; passed += STEP)
			{
				GameWorld::Instance()->Update(STEP);
			}
		}
	};
}

TEST_F(ChaseComponentTest, TargetInSightIsChased)
{
	CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);

	Run(0.2f);

	EXPECT_TRUE(chase->IsChasing());
	EXPECT_TRUE(chase->IsEngaged());
}

TEST_F(ChaseComponentTest, TargetBehindAWallIsNotNoticed)
{
	CreateHero(5, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);

	Run(1.f);

	EXPECT_FALSE(chase->IsChasing());
	EXPECT_FALSE(chase->IsAlerted());
}

TEST_F(ChaseComponentTest, TargetBehindTheBackIsNotNoticed)
{
	CreateHero(1, 1);
	ChaseComponent* chase = CreateEnemy(3, 1);

	Run(1.f);

	EXPECT_FALSE(chase->IsChasing());
	EXPECT_FALSE(chase->IsAlerted());
}

TEST_F(ChaseComponentTest, LostTargetLeavesTheEnemySearching)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	Run(0.2f);
	ASSERT_TRUE(chase->IsChasing());

	hero->GetTransform()->SetWorldPosition(At(5, 1));
	Run(0.2f);

	EXPECT_FALSE(chase->IsChasing());
	EXPECT_TRUE(chase->IsAlerted());
	EXPECT_TRUE(chase->IsEngaged());
}

TEST_F(ChaseComponentTest, EnemyWalksToThePlaceItLastSawTheTarget)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	Run(0.2f);
	ASSERT_TRUE(chase->IsChasing());

	Vector2Df lastSeen = hero->GetTransform()->GetWorldPosition();
	hero->GetTransform()->SetWorldPosition(At(5, 1));

	GameObject* enemy = chase->GetGameObject();
	float before = (lastSeen - enemy->GetTransform()->GetWorldPosition()).GetLength();
	ASSERT_GT(before, RoguelikeGame::ENEMY_ALERT_ARRIVE_DISTANCE);

	Run(2.f);
	float after = (lastSeen - enemy->GetTransform()->GetWorldPosition()).GetLength();

	EXPECT_LE(after, RoguelikeGame::ENEMY_ALERT_ARRIVE_DISTANCE);
}

TEST_F(ChaseComponentTest, SearchEndsWhenTheTimeRunsOut)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	Run(0.2f);
	ASSERT_TRUE(chase->IsChasing());

	hero->GetTransform()->SetWorldPosition(At(5, 1));
	Run(0.2f);
	ASSERT_TRUE(chase->IsAlerted());

	Run(SEARCH_TIME + 1.f);

	EXPECT_FALSE(chase->IsAlerted());
	EXPECT_FALSE(chase->IsEngaged());
}

TEST_F(ChaseComponentTest, EnemyThatNeverSawAnyoneStaysPut)
{
	CreateHero(5, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	GameObject* enemy = chase->GetGameObject();
	Vector2Df before = enemy->GetTransform()->GetWorldPosition();

	Run(2.f);

	Vector2Df after = enemy->GetTransform()->GetWorldPosition();
	EXPECT_FLOAT_EQ(after.x, before.x);
	EXPECT_FLOAT_EQ(after.y, before.y);
}

TEST_F(ChaseComponentTest, SeeingTheTargetAgainStartsANewSearch)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	Run(0.2f);
	ASSERT_TRUE(chase->IsChasing());

	hero->GetTransform()->SetWorldPosition(At(5, 1));
	Run(SEARCH_TIME);
	ASSERT_FALSE(chase->IsAlerted());

	hero->GetTransform()->SetWorldPosition(At(3, 1));
	Run(0.2f);
	ASSERT_TRUE(chase->IsChasing());

	hero->GetTransform()->SetWorldPosition(At(5, 1));
	Run(0.2f);

	EXPECT_TRUE(chase->IsAlerted());
}

TEST_F(ChaseComponentTest, EnemyWithoutSearchTimeForgetsAtOnce)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->SetSearchTime(0.f);
	Run(0.2f);
	ASSERT_TRUE(chase->IsChasing());

	hero->GetTransform()->SetWorldPosition(At(5, 1));
	Run(0.2f);

	EXPECT_FALSE(chase->IsAlerted());
}

TEST_F(ChaseComponentTest, EmptyLastSeenPlaceEndsTheSearchQuickly)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	Run(0.2f);
	ASSERT_TRUE(chase->IsChasing());

	GameWorld::Instance()->DestroyGameObject(hero);
	GameWorld::Instance()->LateUpdate();

	Run(0.2f);
	ASSERT_TRUE(chase->IsAlerted());

	Run(SEARCH_TIME - 1.f);

	EXPECT_FALSE(chase->IsAlerted());
}

TEST_F(ChaseComponentTest, EnemyWithSearchSpotsGoesToCheckThem)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* seeker = CreateEnemy(1, 1);
	seeker->SetSearchSpots(10, 2, 2);
	seeker->SetSearchLook(0.3f);
	seeker->SetSearchGap(1);
	Run(0.2f);
	ASSERT_TRUE(seeker->IsChasing());

	GameWorld::Instance()->DestroyGameObject(hero);
	GameWorld::Instance()->LateUpdate();
	Run(20.f);

	EXPECT_GE(seeker->GetSearchStep(), 1u);
}

TEST_F(ChaseComponentTest, AlertedEnemySeesFurther)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->SetDetectionRadius(100.f);
	chase->SetAlertRadiusScale(2.f);
	chase->GetGameObject()->GetComponent<MovementComponent>()->SetSpeed(0.f);

	Run(0.2f);
	EXPECT_FALSE(chase->IsChasing());

	hero->GetTransform()->SetWorldPosition(At(2, 1));
	Run(0.2f);
	ASSERT_TRUE(chase->IsChasing());

	hero->GetTransform()->SetWorldPosition(At(3, 1));
	Run(0.2f);

	EXPECT_TRUE(chase->IsChasing());
}

TEST_F(ChaseComponentTest, SeeingTheTargetDuringTheSearchResumesTheChase)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->SetSearchSpots(10, 2, 2);
	chase->SetSearchLook(2.f);
	Run(0.2f);
	ASSERT_TRUE(chase->IsChasing());

	hero->GetTransform()->SetWorldPosition(At(5, 1));
	Run(1.f);
	ASSERT_FALSE(chase->IsChasing());
	ASSERT_TRUE(chase->IsAlerted());

	hero->GetTransform()->SetWorldPosition(At(4, 2));
	Run(0.3f);

	EXPECT_TRUE(chase->IsChasing());
	EXPECT_EQ(chase->GetSearchStep(), 0u);
}

TEST_F(ChaseComponentTest, AlertedEnemyBehindAWallCannotSeeTheTarget)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->SetSearchSpots(10, 2, 2);
	chase->SetSearchLook(2.f);
	Run(0.2f);
	ASSERT_TRUE(chase->CanSeeTarget());

	hero->GetTransform()->SetWorldPosition(At(5, 1));
	Run(0.5f);

	EXPECT_TRUE(chase->IsAlerted());
	EXPECT_FALSE(chase->CanSeeTarget());
}

TEST_F(ChaseComponentTest, AlertedEnemyDoesNotAttackThroughAWall)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->SetSearchSpots(10, 2, 2);
	chase->SetSearchLook(2.f);

	GameObject* enemy = chase->GetGameObject();
	auto attack = enemy->AddComponent<RoguelikeGame::EnemyAttackComponent>();
	attack->SetTargetName("Hero");
	attack->SetAttackRange(1000.f);

	Run(0.2f);
	ASSERT_TRUE(chase->CanSeeTarget());
	EXPECT_TRUE(attack->IsAttacking());

	hero->GetTransform()->SetWorldPosition(At(5, 1));
	Run(0.5f);

	ASSERT_TRUE(chase->IsAlerted());
	ASSERT_FALSE(chase->CanSeeTarget());
	EXPECT_FALSE(attack->IsAttacking());
}

TEST_F(ChaseComponentTest, EnemyRemembersWhichWayTheTargetRan)
{
	GameObject* hero = CreateHero(2, 2);
	ChaseComponent* chase = CreateEnemy(1, 2);
	Run(0.2f);
	ASSERT_TRUE(chase->CanSeeTarget());
	ASSERT_FLOAT_EQ(chase->GetEscapeDirection().GetLength(), 0.f);

	hero->GetTransform()->SetWorldPosition(At(5, 2));
	Run(0.2f);
	ASSERT_TRUE(chase->CanSeeTarget());

	EXPECT_GT(chase->GetEscapeDirection().x, 0.f);
}

TEST_F(ChaseComponentTest, TargetRunningBackFlipsTheRememberedWay)
{
	GameObject* hero = CreateHero(3, 2);
	ChaseComponent* chase = CreateEnemy(1, 2);
	Run(0.2f);
	ASSERT_TRUE(chase->CanSeeTarget());

	hero->GetTransform()->SetWorldPosition(At(6, 2));
	Run(0.2f);
	ASSERT_GT(chase->GetEscapeDirection().x, 0.f);

	hero->GetTransform()->SetWorldPosition(At(3, 2));
	Run(0.2f);

	EXPECT_LT(chase->GetEscapeDirection().x, 0.f);
}

TEST_F(ChaseComponentTest, TargetBehindAClosedDoorIsNotNoticed)
{
	LoadMap(DOORWAY);
	CreateDoor(3, 1);
	CreateHero(5, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);

	Run(0.4f);

	EXPECT_FALSE(chase->IsChasing());
	EXPECT_FALSE(chase->IsEngaged());
	EXPECT_FALSE(chase->CanSeeTarget());
}

TEST_F(ChaseComponentTest, OpenedDoorLetsTheEnemySeeTheTarget)
{
	LoadMap(DOORWAY);
	DoorComponent* door = CreateDoor(3, 1);
	GameObject* hero = CreateHeroWithKey(5, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);

	Run(0.4f);
	ASSERT_FALSE(chase->IsChasing());

	ASSERT_TRUE(door->TryOpenFor(hero));
	Run(0.2f);

	EXPECT_TRUE(chase->IsChasing());
	EXPECT_TRUE(chase->CanSeeTarget());
}

TEST_F(ChaseComponentTest, ClosedDoorHidesTheTargetThatWasInSight)
{
	LoadMap(DOORWAY);
	GameObject* hero = CreateHero(2, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	Run(0.2f);
	ASSERT_TRUE(chase->CanSeeTarget());

	hero->GetTransform()->SetWorldPosition(At(5, 1));
	CreateDoor(3, 1);
	Run(0.2f);

	EXPECT_FALSE(chase->CanSeeTarget());
}

TEST_F(ChaseComponentTest, GaugeIsHiddenWhileNothingIsNoticed)
{
	CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->SetAwareness(1.f, 0.7f);
	chase->GetGameObject()->GetTransform()->SetWorldRotation(180.f);
	auto gauge = chase->GetGameObject()->AddComponent<RoguelikeGame::AwarenessGaugeComponent>();

	Run(1.f);

	EXPECT_FALSE(gauge->IsShown());
	EXPECT_FLOAT_EQ(gauge->GetShownPart(), 0.f);
}

TEST_F(ChaseComponentTest, GaugeFillsUpWhileTheEnemyPeers)
{
	CreateHero(1, 3);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->SetAwareness(1.f, 0.7f);
	chase->SetPeripheryHalfAngle(100.f);
	auto gauge = chase->GetGameObject()->AddComponent<RoguelikeGame::AwarenessGaugeComponent>();

	Run(0.6f);
	float early = gauge->GetShownPart();
	Run(0.6f);

	EXPECT_TRUE(gauge->IsShown());
	EXPECT_GT(early, 0.f);
	EXPECT_GT(gauge->GetShownPart(), early);
}

TEST_F(ChaseComponentTest, GaugeIsFullWhenTheChaseStarts)
{
	CreateHero(1, 3);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->SetAwareness(1.f, 0.7f);
	chase->SetPeripheryHalfAngle(100.f);
	auto gauge = chase->GetGameObject()->AddComponent<RoguelikeGame::AwarenessGaugeComponent>();

	Run(2.5f);

	ASSERT_TRUE(chase->IsChasing());
	EXPECT_FLOAT_EQ(gauge->GetShownPart(), 1.f);
}

TEST_F(ChaseComponentTest, TargetInFrontIsSpottedAtOnce)
{
	CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->SetAwareness(0.2f, 0.2f);

	Run(0.1f);

	EXPECT_TRUE(chase->IsChasing());
	EXPECT_EQ(chase->GetVisionBand(), RoguelikeGame::VisionBand::Focus);
}

TEST_F(ChaseComponentTest, TargetAtTheSideIsNotSpottedAtOnce)
{
	CreateHero(1, 3);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->SetAwareness(1.f, 0.7f);
	chase->SetPeripheryHalfAngle(100.f);

	Run(0.2f);

	EXPECT_FALSE(chase->IsChasing());
	EXPECT_EQ(chase->GetVisionBand(), RoguelikeGame::VisionBand::Periphery);
}

TEST_F(ChaseComponentTest, EnemyGrowsSuspiciousOfSomeoneAtTheSide)
{
	CreateHero(1, 3);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->SetAwareness(1.f, 0.7f);
	chase->SetPeripheryHalfAngle(100.f);

	Run(0.9f);

	EXPECT_TRUE(chase->IsSuspicious());
	EXPECT_FALSE(chase->IsChasing());
	EXPECT_TRUE(chase->IsEngaged());
}

TEST_F(ChaseComponentTest, SomeoneAtTheSideIsSpottedInTheEnd)
{
	CreateHero(1, 3);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->SetAwareness(1.f, 0.7f);
	chase->SetPeripheryHalfAngle(100.f);

	Run(3.f);

	EXPECT_TRUE(chase->IsChasing());
}

TEST_F(ChaseComponentTest, WithoutPeripheryTheSideIsNotSeenAtAll)
{
	CreateHero(1, 3);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->SetAwareness(1.f, 0.7f);

	Run(3.f);

	EXPECT_FALSE(chase->IsChasing());
	EXPECT_FLOAT_EQ(chase->GetAwareness(), 0.f);
}

TEST_F(ChaseComponentTest, SomeoneFarBehindIsNotNoticed)
{
	CreateHero(4, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->SetAwareness(1.f, 0.7f);
	chase->SetPeripheryHalfAngle(100.f);
	chase->GetGameObject()->GetTransform()->SetWorldRotation(180.f);

	Run(3.f);

	EXPECT_EQ(chase->GetVisionBand(), RoguelikeGame::VisionBand::None);
	EXPECT_FALSE(chase->IsChasing());
}

TEST_F(ChaseComponentTest, SomeoneRightBehindTheBackIsNoticed)
{
	CreateHero(2, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->SetAwareness(1.f, 0.7f);
	chase->SetPeripheryHalfAngle(100.f);
	chase->GetGameObject()->GetTransform()->SetWorldRotation(180.f);

	Run(0.1f);
	ASSERT_EQ(chase->GetVisionBand(), RoguelikeGame::VisionBand::Back);
	EXPECT_FALSE(chase->IsChasing());

	Run(3.f);

	EXPECT_TRUE(chase->IsChasing());
}

TEST_F(ChaseComponentTest, SideVisionReachesOnlyPartOfTheRadius)
{
	CreateHero(6, 2);
	ChaseComponent* chase = CreateEnemy(1, 2);
	chase->SetAwareness(1.f, 0.7f);
	chase->SetPeripheryHalfAngle(100.f);
	chase->GetGameObject()->GetTransform()->SetWorldRotation(90.f);

	Run(0.1f);

	EXPECT_EQ(chase->GetVisionBand(), RoguelikeGame::VisionBand::None);
	EXPECT_FLOAT_EQ(chase->GetAwareness(), 0.f);
}

TEST_F(ChaseComponentTest, CloserToTheSideIsStillSeen)
{
	CreateHero(4, 2);
	ChaseComponent* chase = CreateEnemy(1, 2);
	chase->SetAwareness(1.f, 0.7f);
	chase->SetPeripheryHalfAngle(100.f);
	chase->GetGameObject()->GetTransform()->SetWorldRotation(90.f);

	Run(0.1f);

	EXPECT_EQ(chase->GetVisionBand(), RoguelikeGame::VisionBand::Periphery);
}

TEST_F(ChaseComponentTest, HeardNoiseSendsTheEnemyToLook)
{
	CreateHero(11, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->GetGameObject()->GetTransform()->SetWorldRotation(180.f);
	Run(0.1f);
	ASSERT_FALSE(chase->IsEngaged());

	chase->Hear(At(3, 1));
	Run(0.3f);

	EXPECT_TRUE(chase->IsEngaged());
	EXPECT_GT(chase->GetGameObject()->GetTransform()->GetWorldPosition().x, At(1, 1).x);
}

TEST_F(ChaseComponentTest, NoiseNearbyIsHeardAndFarAwayIsNot)
{
	CreateHero(11, 1);
	ChaseComponent* close = CreateEnemy(1, 1);
	ChaseComponent* far = CreateEnemy(9, 1);
	close->GetGameObject()->GetTransform()->SetWorldRotation(180.f);
	far->GetGameObject()->GetTransform()->SetWorldRotation(180.f);
	Run(0.1f);

	RoguelikeGame::Noise noise;
	noise.position = At(2, 1);
	noise.radius = 200.f;
	noise.from = RoguelikeGame::Faction::Player;
	RaiseNoise(noise);
	Run(0.1f);

	EXPECT_TRUE(close->IsEngaged());
	EXPECT_FALSE(far->IsEngaged());
}

TEST_F(ChaseComponentTest, EnemyDoesNotTurnOnItsOwnSideShot)
{
	CreateHero(11, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->GetGameObject()->AddComponent<RoguelikeGame::FactionComponent>()->SetFaction(RoguelikeGame::Faction::Enemy);
	chase->GetGameObject()->GetTransform()->SetWorldRotation(180.f);
	Run(0.1f);

	RoguelikeGame::Noise noise;
	noise.position = At(2, 1);
	noise.radius = 400.f;
	noise.from = RoguelikeGame::Faction::Enemy;
	RaiseNoise(noise);
	Run(0.1f);

	EXPECT_FALSE(chase->IsEngaged());
}

TEST_F(ChaseComponentTest, BrokenCrateIsHeardByEverySide)
{
	CreateHero(11, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->GetGameObject()->AddComponent<RoguelikeGame::FactionComponent>()->SetFaction(RoguelikeGame::Faction::Enemy);
	chase->GetGameObject()->GetTransform()->SetWorldRotation(180.f);
	Run(0.1f);

	RoguelikeGame::Noise noise;
	noise.position = At(2, 1);
	noise.radius = 400.f;
	RaiseNoise(noise);
	Run(0.1f);

	EXPECT_TRUE(chase->IsEngaged());
}

TEST_F(ChaseComponentTest, NoiseBehindAWallDoesNotReach)
{
	CreateHero(11, 2);
	ChaseComponent* chase = CreateEnemy(5, 1);
	chase->GetGameObject()->GetTransform()->SetWorldRotation(180.f);
	Run(0.1f);

	RoguelikeGame::Noise noise;
	noise.position = At(2, 1);
	noise.radius = 300.f;
	noise.from = RoguelikeGame::Faction::Player;
	RaiseNoise(noise);
	Run(0.1f);

	EXPECT_FALSE(chase->IsEngaged());
}

TEST_F(ChaseComponentTest, TheSameNoiseInTheOpenIsHeard)
{
	CreateHero(11, 2);
	ChaseComponent* chase = CreateEnemy(5, 2);
	chase->GetGameObject()->GetTransform()->SetWorldRotation(180.f);
	Run(0.1f);

	RoguelikeGame::Noise noise;
	noise.position = At(2, 2);
	noise.radius = 300.f;
	noise.from = RoguelikeGame::Faction::Player;
	RaiseNoise(noise);
	Run(0.1f);

	EXPECT_TRUE(chase->IsEngaged());
}
