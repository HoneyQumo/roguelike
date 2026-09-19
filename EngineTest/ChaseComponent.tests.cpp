#include "pch.h"
#include "Awareness.h"
#include "FactionComponent.h"
#include "Noise.h"
#include "AwarenessGaugeComponent.h"
#include "ChaseComponent.h"
#include "Footsteps.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include "LevelLoader.h"
#include "PathService.h"
#include <AimRotationComponent.h>
#include <GameWorld.h>
#include <MovementComponent.h>
#include "DoorComponent.h"
#include "EnemyAttackComponent.h"
#include "WeaponComponent.h"
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

	const std::string CORRIDOR =
		"[map]\n"
		"###############\n"
		"#.............#\n"
		"###############\n";

	// Два тайла по прямой, четырнадцать в обход - ровно тот случай, где тревога кончалась раньше пути.
	const std::string DETOUR =
		"[map]\n"
		"#########\n"
		"#.......#\n"
		"#######.#\n"
		"#.......#\n"
		"#########\n";

	// Стена без прохода: до точки справа не добраться никак.
	const std::string SPLIT =
		"[map]\n"
		"#######\n"
		"#..#..#\n"
		"#######\n";

	// Столб посреди зала: единственное укрытие на карте.
	const std::string PILLAR =
		"[map]\n"
		"#########\n"
		"#.......#\n"
		"#...#...#\n"
		"#.......#\n"
		"#########\n";

	constexpr float SEARCH_TIME = 4.f;
	constexpr float NOTICES_AT_ONCE = 100.f;
	constexpr float STEP = 0.05f;
	constexpr float LOOK_TIME = 1.f;

	// Укрытие стоит рядом со стрелком, а не между ним и целью: стрелять ему ничто не мешает.
	const std::string FIRING_RANGE =
		"[map]\n"
		"#############\n"
		"#.....#.....#\n"
		"#...........#\n"
		"#...........#\n"
		"#############\n";

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

		ChaseComponent* CreateEnemy(int column, int row, float degrees = 0.f)
		{
			GameObject* enemy = GameWorld::Instance()->CreateGameObject("Watcher");
			enemy->GetTransform()->SetWorldPosition(At(column, row));
			enemy->GetTransform()->SetWorldRotation(degrees);

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

		ChaseComponent* CreateGuard(int column, int row, float degrees, float shoutRadius)
		{
			ChaseComponent* chase = CreateEnemy(column, row, degrees);
			chase->SetShoutRadius(shoutRadius);
			chase->GetGameObject()->AddComponent<RoguelikeGame::FactionComponent>()
				->SetFaction(RoguelikeGame::Faction::Enemy);

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

	chase->Hear(At(3, 1), 1.f);
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

TEST_F(ChaseComponentTest, SpottingTheTargetCallsTheNeighbour)
{
	LoadMap(CORRIDOR);
	CreateHero(1, 1);
	CreateGuard(3, 1, 180.f, RoguelikeGame::SHOUT_RADIUS);
	ChaseComponent* neighbour = CreateGuard(8, 1, 0.f, 0.f);
	ChaseComponent* outOfRange = CreateGuard(13, 1, 0.f, 0.f);

	Run(0.2f);

	EXPECT_TRUE(neighbour->IsAlerted());
	EXPECT_FALSE(outOfRange->IsAlerted());
}

TEST_F(ChaseComponentTest, ASilentEnemyCallsNobody)
{
	LoadMap(CORRIDOR);
	CreateHero(1, 1);
	CreateGuard(3, 1, 180.f, 0.f);
	ChaseComponent* neighbour = CreateGuard(8, 1, 0.f, 0.f);

	Run(0.2f);

	EXPECT_FALSE(neighbour->IsAlerted());
}

TEST_F(ChaseComponentTest, TheOneWhoOnlyHeardDoesNotCallFurther)
{
	LoadMap(CORRIDOR);
	CreateHero(1, 1);
	CreateGuard(3, 1, 180.f, RoguelikeGame::SHOUT_RADIUS);
	ChaseComponent* neighbour = CreateGuard(8, 1, 0.f, RoguelikeGame::SHOUT_RADIUS);
	ChaseComponent* outOfRange = CreateGuard(13, 1, 0.f, 0.f);

	Run(0.2f);

	EXPECT_TRUE(neighbour->IsAlerted());
	EXPECT_FALSE(outOfRange->IsAlerted());
}

TEST_F(ChaseComponentTest, TheCallerItselfKeepsChasingInsteadOfInvestigating)
{
	LoadMap(CORRIDOR);
	CreateHero(1, 1);
	ChaseComponent* caller = CreateGuard(3, 1, 180.f, RoguelikeGame::SHOUT_RADIUS);

	Run(0.2f);

	EXPECT_TRUE(caller->IsChasing());
}

TEST_F(ChaseComponentTest, ACallSkipsTheOneWhoMadeIt)
{
	LoadMap(CORRIDOR);
	ChaseComponent* caller = CreateGuard(3, 1, 0.f, 0.f);
	ChaseComponent* neighbour = CreateGuard(8, 1, 0.f, 0.f);

	RoguelikeGame::Noise call;
	call.position = At(3, 1);
	call.radius = RoguelikeGame::SHOUT_RADIUS;
	call.from = RoguelikeGame::Faction::Enemy;
	call.kind = RoguelikeGame::NoiseKind::Call;
	RoguelikeGame::RaiseNoise(call, caller->GetGameObject());

	EXPECT_FALSE(caller->IsAlerted());
	EXPECT_TRUE(neighbour->IsAlerted());
}

TEST_F(ChaseComponentTest, ProvokedEnemyIsAlreadyEngagedFacingAway)
{
	GameObject* hero = CreateHero(1, 1);
	ChaseComponent* chase = CreateEnemy(3, 1, 0.f);

	Run(0.2f);
	ASSERT_FALSE(chase->IsChasing());

	chase->Provoke(hero->GetTransform()->GetWorldPosition());
	Run(0.1f);

	EXPECT_EQ(chase->GetAwarenessState(), RoguelikeGame::AwarenessState::Provoked);
	EXPECT_TRUE(chase->IsEngaged());
}

TEST_F(ChaseComponentTest, ProvokedEnemyWalksToTheGivenPlace)
{
	ChaseComponent* chase = CreateEnemy(1, 1, 0.f);
	GameObject* enemy = chase->GetGameObject();
	float before = enemy->GetTransform()->GetWorldPosition().x;

	chase->Provoke(At(5, 1));
	Run(0.5f);

	EXPECT_GT(enemy->GetTransform()->GetWorldPosition().x, before);
}

TEST_F(ChaseComponentTest, WithoutAChaseSpeedThePaceNeverChanges)
{
	ChaseComponent* chase = CreateEnemy(1, 1, 0.f);
	auto movement = chase->GetGameObject()->GetComponent<MovementComponent>();
	CreateHero(2, 1);

	Run(0.5f);

	ASSERT_TRUE(chase->IsChasing());
	EXPECT_FLOAT_EQ(movement->GetSpeed(), 120.f) << "an enemy without a chase speed sped up on its own";
}

TEST_F(ChaseComponentTest, SpottingTheTargetSwitchesTheWalkToARun)
{
	ChaseComponent* chase = CreateEnemy(1, 1, 0.f);
	chase->SetChaseSpeed(200.f);
	auto movement = chase->GetGameObject()->GetComponent<MovementComponent>();
	CreateHero(2, 1);

	Run(0.5f);

	ASSERT_TRUE(chase->IsChasing());
	EXPECT_FLOAT_EQ(movement->GetSpeed(), 200.f);
}

TEST_F(ChaseComponentTest, AnEnemyWhoSeesNobodyKeepsWalking)
{
	ChaseComponent* chase = CreateEnemy(1, 1, 0.f);
	chase->SetChaseSpeed(200.f);
	auto movement = chase->GetGameObject()->GetComponent<MovementComponent>();

	Run(0.5f);

	ASSERT_FALSE(chase->IsChasing());
	EXPECT_FLOAT_EQ(movement->GetSpeed(), chase->GetWalkSpeed()) << "the patrol is running for no reason";
}

TEST_F(ChaseComponentTest, ANegativeChaseSpeedIsRefused)
{
	ChaseComponent* chase = CreateEnemy(1, 1, 0.f);

	chase->SetChaseSpeed(-100.f);

	EXPECT_FLOAT_EQ(chase->GetChaseSpeed(), 0.f);
}

TEST_F(ChaseComponentTest, AnOrdinaryEnemyFacingAwayNeverNoticesAnybody)
{
	ChaseComponent* chase = CreateEnemy(1, 1, 180.f);
	CreateHero(5, 1);

	Run(2.f);

	EXPECT_FALSE(chase->IsChasing()) << "an enemy with its back turned saw through itself";
}

TEST_F(ChaseComponentTest, AHunterKnowsWhoItCameForWithoutLookingAtHim)
{
	ChaseComponent* chase = CreateEnemy(1, 1, 180.f);
	CreateHero(5, 1);
	chase->SetForcedChase(true);

	Run(2.f);

	EXPECT_TRUE(chase->IsChasing()) << "the hunter waits to be shown its target";
}

TEST_F(ChaseComponentTest, AHunterKeepsComingLongAfterTheAlarmWouldFade)
{
	ChaseComponent* chase = CreateEnemy(1, 1, 180.f);
	GameObject* hero = CreateHero(5, 1);
	chase->SetForcedChase(true);
	chase->Provoke(hero->GetTransform()->GetWorldPosition());

	// Заметно дольше, чем живёт обычная тревога: она бы давно выветрилась.
	Run(3.f * SEARCH_TIME);

	EXPECT_TRUE(chase->IsChasing()) << "the hunter lost the player and went back to patrol";
}

TEST_F(ChaseComponentTest, AHunterWalksTowardsThePlayerNotJustStandsProvoked)
{
	ChaseComponent* chase = CreateEnemy(1, 1, 180.f);
	CreateHero(9, 1);
	chase->SetForcedChase(true);

	GameObject* enemy = chase->GetGameObject();
	float before = enemy->GetTransform()->GetWorldPosition().x;

	Run(1.f);

	EXPECT_GT(enemy->GetTransform()->GetWorldPosition().x, before) << "the hunter knows the target but does not move";
}

TEST_F(ChaseComponentTest, ADeadEnemyIsNotPutBackOnItsFeet)
{
	ChaseComponent* chase = CreateEnemy(1, 1, 0.f);
	chase->SetChaseSpeed(200.f);

	GameObject* enemy = chase->GetGameObject();
	auto health = enemy->AddComponent<RoguelikeGame::HealthComponent>();
	health->SetMaxHealth(50.f);
	auto movement = enemy->GetComponent<MovementComponent>();
	CreateHero(2, 1);

	Run(0.2f);
	ASSERT_TRUE(chase->IsChasing());

	health->TakeDamage(1000.f);
	movement->SetSpeed(0.f);

	Run(0.5f);

	EXPECT_FLOAT_EQ(movement->GetSpeed(), 0.f) << "the corpse walked off along its route";
}

TEST_F(ChaseComponentTest, ALongDetourIsWalkedToTheEnd)
{
	LoadMap(DETOUR);

	ChaseComponent* chase = CreateEnemy(1, 1);
	GameObject* enemy = chase->GetGameObject();

	chase->Hear(At(1, 3), 1.f);
	Run(9.f);

	float left = (enemy->GetTransform()->GetWorldPosition() - At(1, 3)).GetLength();

	EXPECT_LT(left, RoguelikeGame::TILE_SIZE) << "the enemy turned back before reaching the point";
}

// Идущий на шум целился в точку и смотрел сквозь стену,
// а конус зрения едет за прицелом - обойти такого со спины было нельзя.
TEST_F(ChaseComponentTest, TheEnemyWalkingToANoiseLooksAlongTheRoad)
{
	LoadMap(DETOUR);

	ChaseComponent* chase = CreateEnemy(1, 1);
	GameObject* enemy = chase->GetGameObject();

	chase->Hear(At(1, 3), 1.f);
	Run(0.5f);

	Vector2Df before = enemy->GetTransform()->GetWorldPosition();
	Run(0.3f);
	Vector2Df after = enemy->GetTransform()->GetWorldPosition();

	Vector2Df moved = after - before;
	ASSERT_GT(moved.GetLength(), 10.f) << "враг не идёт на шум";

	Vector2Df toPoint = (At(1, 3) - after).Normalized();
	Vector2Df heading = moved.Normalized();

	ASSERT_LT(heading.DotProduct(toPoint), 0.7f) << "дорога не гнётся, тест ничего не проверяет";

	Vector2Df forward = enemy->GetTransform()->GetForward();

	EXPECT_GT(forward.DotProduct(heading), 0.9f) << "взгляд не по дороге";
	EXPECT_LT(forward.DotProduct(toPoint), 0.7f) << "смотрит на источник шума сквозь стену";
}

TEST_F(ChaseComponentTest, AnAlarmOutlivesTheWalkToIt)
{
	LoadMap(DETOUR);

	ChaseComponent* chase = CreateEnemy(1, 1);

	chase->Hear(At(1, 3), 1.f);
	Run(7.f);

	EXPECT_TRUE(chase->IsAlerted()) << "the alarm ran out while the enemy was still on its way";
}

TEST_F(ChaseComponentTest, APointBehindASolidWallIsNotEvenTakenUp)
{
	LoadMap(SPLIT);

	ChaseComponent* chase = CreateEnemy(1, 1);
	GameObject* enemy = chase->GetGameObject();
	Vector2Df before = enemy->GetTransform()->GetWorldPosition();

	chase->Hear(At(4, 1), 1.f);
	Run(3.f);

	EXPECT_TRUE(chase->IsAlerted()) << "the enemy heard the shot and ignored it";
	EXPECT_LT((enemy->GetTransform()->GetWorldPosition() - before).GetLength(), RoguelikeGame::TILE_SIZE)
		<< "the enemy is pushing into the wall towards a point it cannot reach";
}

TEST_F(ChaseComponentTest, AShooterBacksOffWhenTheHeroWalksIntoItsFace)
{
	LoadMap(CORRIDOR);

	ChaseComponent* chase = CreateEnemy(6, 1, 180.f);
	chase->SetStopDistance(220.f);
	chase->SetChaseSpeed(200.f);
	CreateHero(4, 1);

	GameObject* enemy = chase->GetGameObject();
	float before = enemy->GetTransform()->GetWorldPosition().x;

	Run(1.f);

	EXPECT_GT(enemy->GetTransform()->GetWorldPosition().x, before) << "the shooter let the hero walk right up to it";
	EXPECT_TRUE(chase->IsChasing()) << "backing off is not the same as losing the target";
}

TEST_F(ChaseComponentTest, AKnifemanDoesNotBackOffAtAll)
{
	LoadMap(CORRIDOR);

	ChaseComponent* chase = CreateEnemy(6, 1, 180.f);
	chase->SetChaseSpeed(200.f);
	GameObject* hero = CreateHero(4, 1);

	GameObject* enemy = chase->GetGameObject();
	enemy->GetTransform()->SetWorldPosition(At(4, 1) + Vector2Df{20.f, 0.f});

	auto gap = [enemy, hero] { return (enemy->GetTransform()->GetWorldPosition()
		- hero->GetTransform()->GetWorldPosition()).GetLength(); };
	float before = gap();

	Run(1.f);

	EXPECT_LE(gap(), before) << "the knife enemy gave up ground instead of holding it";
}

TEST_F(ChaseComponentTest, ABackingShooterStopsOnceItIsComfortableAgain)
{
	LoadMap(CORRIDOR);

	ChaseComponent* chase = CreateEnemy(6, 1, 180.f);
	chase->SetStopDistance(220.f);
	chase->SetChaseSpeed(200.f);
	GameObject* hero = CreateHero(4, 1);

	Run(3.f);

	GameObject* enemy = chase->GetGameObject();
	float gap = (enemy->GetTransform()->GetWorldPosition() - hero->GetTransform()->GetWorldPosition()).GetLength();

	EXPECT_GE(gap, 220.f - RoguelikeGame::ENEMY_COMFORT_DEAD_ZONE) << "the shooter never regained its distance";
	EXPECT_LE(gap, 220.f + RoguelikeGame::TILE_SIZE) << "the shooter ran away instead of holding its ground";
}

namespace
{
	RoguelikeGame::WeaponComponent* GiveAnEmptyGun(GameObject* enemy, float reloadTime)
	{
		auto weapon = enemy->AddComponent<RoguelikeGame::WeaponComponent>();
		weapon->SetMagazine(8, 0);
		weapon->SetAmmoInMagazine(0);
		weapon->SetReloadTime(reloadTime);

		return weapon;
	}
}

TEST_F(ChaseComponentTest, AReloadingShooterStepsBehindThePillar)
{
	LoadMap(PILLAR);

	ChaseComponent* chase = CreateEnemy(3, 2, 180.f);
	chase->SetChaseSpeed(150.f);
	GameObject* hero = CreateHero(1, 2);

	GameObject* enemy = chase->GetGameObject();
	auto weapon = GiveAnEmptyGun(enemy, 5.f);

	Run(0.1f);
	ASSERT_TRUE(weapon->TryReload());

	Run(4.f);

	EXPECT_TRUE(LevelGrid::Current().HasWallBetween(hero->GetTransform()->GetWorldPosition(),
		enemy->GetTransform()->GetWorldPosition())) << "the shooter reloaded in plain view";
}

TEST_F(ChaseComponentTest, AShooterComesBackOutOnceItIsLoaded)
{
	LoadMap(PILLAR);

	ChaseComponent* chase = CreateEnemy(3, 2, 180.f);
	chase->SetChaseSpeed(150.f);
	GameObject* hero = CreateHero(1, 2);

	GameObject* enemy = chase->GetGameObject();
	auto weapon = GiveAnEmptyGun(enemy, 2.f);

	Run(0.1f);
	ASSERT_TRUE(weapon->TryReload());

	Run(8.f);

	ASSERT_FALSE(weapon->IsReloading());
	EXPECT_FALSE(LevelGrid::Current().HasWallBetween(hero->GetTransform()->GetWorldPosition(),
		enemy->GetTransform()->GetWorldPosition())) << "the shooter stayed behind cover with a full magazine";
}

TEST_F(ChaseComponentTest, WithNoCoverAroundTheShooterFightsOnAsBefore)
{
	LoadMap(CORRIDOR);

	ChaseComponent* chase = CreateEnemy(8, 1, 180.f);
	chase->SetChaseSpeed(150.f);
	CreateHero(2, 1);

	GameObject* enemy = chase->GetGameObject();
	auto weapon = GiveAnEmptyGun(enemy, 5.f);

	Run(0.1f);
	ASSERT_TRUE(weapon->TryReload());

	float before = enemy->GetTransform()->GetWorldPosition().x;
	Run(2.f);

	EXPECT_LT(enemy->GetTransform()->GetWorldPosition().x, before) << "the shooter froze instead of closing in";
}

TEST_F(ChaseComponentTest, AKnifemanHasNothingToReloadAndNeverHides)
{
	LoadMap(PILLAR);

	ChaseComponent* chase = CreateEnemy(3, 2, 180.f);
	chase->SetChaseSpeed(150.f);
	GameObject* hero = CreateHero(1, 2);

	GameObject* enemy = chase->GetGameObject();

	Run(3.f);

	EXPECT_FALSE(LevelGrid::Current().HasWallBetween(hero->GetTransform()->GetWorldPosition(),
		enemy->GetTransform()->GetWorldPosition())) << "the knife enemy hid from a fight it should have joined";
}

TEST_F(ChaseComponentTest, AnEnemyIsTacticalUntilToldOtherwise)
{
	ChaseComponent* chase = CreateEnemy(1, 1);

	EXPECT_TRUE(chase->GetFightStyle().keepsDistance);
	EXPECT_TRUE(chase->GetFightStyle().takesCover);
}

TEST_F(ChaseComponentTest, ARelentlessShooterGivesNoGround)
{
	LoadMap(CORRIDOR);

	ChaseComponent* chase = CreateEnemy(6, 1, 180.f);
	chase->SetStopDistance(220.f);
	chase->SetChaseSpeed(200.f);
	chase->SetFightStyle(RoguelikeGame::RELENTLESS_FIGHT);
	CreateHero(4, 1);

	GameObject* enemy = chase->GetGameObject();
	float before = enemy->GetTransform()->GetWorldPosition().x;

	Run(1.f);

	EXPECT_LE(enemy->GetTransform()->GetWorldPosition().x, before) << "a pursuer gave ground instead of pushing";
}

TEST_F(ChaseComponentTest, ARelentlessShooterReloadsWhereItStands)
{
	LoadMap(PILLAR);

	ChaseComponent* chase = CreateEnemy(3, 2, 180.f);
	chase->SetChaseSpeed(150.f);
	chase->SetFightStyle(RoguelikeGame::RELENTLESS_FIGHT);
	GameObject* hero = CreateHero(1, 2);

	GameObject* enemy = chase->GetGameObject();
	auto weapon = GiveAnEmptyGun(enemy, 5.f);

	Run(0.1f);
	ASSERT_TRUE(weapon->TryReload());

	Run(4.f);

	EXPECT_FALSE(LevelGrid::Current().HasWallBetween(hero->GetTransform()->GetWorldPosition(),
		enemy->GetTransform()->GetWorldPosition())) << "a pursuer went looking for cover";
}

namespace
{
	struct Volley
	{
		int shots = 0;
		float walked = 0.f;
	};

	struct FightSetup
	{
		bool isTactical = true;
		int column = 7;
		float stopDistance = 384.f;
	};
}

/**
*	Сколько огня стоит осторожность.
*
*	Укрытие на 30% магазина забирает у врага время на дорогу туда и обратно.
*	Тест не сторожит точное число - он ловит тот день, когда тактика превратит
*	стрелка в бегуна, который почти не стреляет.
*/
class FiringRateTest : public ChaseComponentTest
{
protected:
	// Укрытие рядом со стрелком, а не между ним и целью: стрелять ему ничто не мешает.
	const std::string FIRING_RANGE =
		"[map]\n"
		"#############\n"
		"#.....#.....#\n"
		"#...........#\n"
		"#...........#\n"
		"#############\n";

	Volley Fire(const FightSetup& setup)
	{
		GameWorld::Instance()->Clear();
		LoadMap(FIRING_RANGE);

		ChaseComponent* chase = CreateEnemy(setup.column, 2, 180.f);
		chase->SetDetectionRadius(1200.f);
		chase->SetStopDistance(setup.stopDistance);
		chase->SetChaseSpeed(267.f);
		chase->SetFightStyle(setup.isTactical ? RoguelikeGame::TACTICAL_FIGHT : RoguelikeGame::RELENTLESS_FIGHT);
		CreateHero(1, 2);

		GameObject* enemy = chase->GetGameObject();

		// АК: магазин 30, откат 0.7, перезарядка 1.6.
		auto weapon = enemy->AddComponent<RoguelikeGame::WeaponComponent>();
		weapon->SetMagazine(30, 0);
		weapon->SetAmmoInMagazine(30);
		weapon->SetReloadTime(1.6f);
		weapon->SetCooldown(0.7f);

		Volley volley;
		weapon->SubscribeShot([&volley](const Vector2Df&, const Vector2Df&, float, float) { volley.shots++; });

		auto attack = enemy->AddComponent<RoguelikeGame::EnemyAttackComponent>();
		attack->SetTargetName("Hero");
		attack->SetAttackRange(900.f);

		Vector2Df was = enemy->GetTransform()->GetWorldPosition();
		for (float passed = 0.f; passed < MINUTE; passed += STEP)
		{
			GameWorld::Instance()->Update(STEP);

			Vector2Df now = enemy->GetTransform()->GetWorldPosition();
			volley.walked += (now - was).GetLength();
			was = now;
		}

		return volley;
	}

	static constexpr float MINUTE = 60.f;
};

TEST_F(FiringRateTest, CoverNextDoorCostsAlmostNothing)
{
	int relentless = Fire({false, 7, 384.f}).shots;
	Volley tactical = Fire({true, 7, 384.f});

	ASSERT_GT(relentless, 0);
	EXPECT_GT(tactical.walked, 0.f) << "the shooter never went for cover, so there is nothing to compare";
	EXPECT_GE(tactical.shots * 100 / relentless, 90)
		<< "cover right next to the shooter cost it " << relentless - tactical.shots << " shots a minute";
}

TEST_F(FiringRateTest, ALongWalkToCoverCostsMore)
{
	int relentless = Fire({false, 11, 640.f}).shots;
	Volley tactical = Fire({true, 11, 640.f});

	ASSERT_GT(relentless, 0);
	EXPECT_GE(tactical.shots * 100 / relentless, 80)
		<< "the shooter spends the fight walking: " << tactical.walked << " px, "
		<< relentless - tactical.shots << " shots a minute lost";
}

// Раньше уровень присваивался, и очередь из десяти выстрелов равнялась одному.
TEST_F(ChaseComponentTest, QuietNoisesAddUpToALoudOne)
{
	CreateHero(11, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	Run(0.1f);

	chase->Hear(At(3, 1), 0.2f);
	float once = chase->GetAwareness();

	chase->Hear(At(3, 1), 0.2f);
	float twice = chase->GetAwareness();

	EXPECT_GT(once, 0.f);
	EXPECT_GT(twice, once);
}

TEST_F(ChaseComponentTest, ANoiseRightUnderTheNoseRaisesMoreThanADistantOne)
{
	CreateHero(11, 1);
	ChaseComponent* loud = CreateEnemy(1, 1);
	ChaseComponent* faint = CreateEnemy(1, 3);
	Run(0.1f);

	loud->Hear(At(3, 1), 1.f);
	faint->Hear(At(3, 3), 0.15f);

	EXPECT_GT(loud->GetAwareness(), faint->GetAwareness());
}

// Выстрел в упор доводит до погони сразу, далёкий - только настораживает.
TEST_F(ChaseComponentTest, ANoiseRightUnderTheNoseProvokesAndADistantOneOnlyAlerts)
{
	CreateHero(11, 1);
	ChaseComponent* close = CreateEnemy(1, 1);
	ChaseComponent* far = CreateEnemy(1, 3);
	Run(0.1f);

	close->Hear(At(2, 1), 1.f);
	far->Hear(At(6, 3), 0.15f);

	EXPECT_EQ(close->GetAwarenessState(), RoguelikeGame::AwarenessState::Provoked);
	EXPECT_EQ(far->GetAwarenessState(), RoguelikeGame::AwarenessState::Alerted);
}

// Любой услышанный шум по-прежнему зовёт посмотреть: радиус для того и задан.
TEST_F(ChaseComponentTest, EvenAFaintNoiseStillSendsTheEnemyToLook)
{
	CreateHero(11, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	Vector2Df stood = chase->GetGameObject()->GetTransform()->GetWorldPosition();
	Run(0.1f);

	chase->Hear(At(5, 1), 0.15f);
	Run(1.5f);

	EXPECT_GT(chase->GetGameObject()->GetTransform()->GetWorldPosition().x, stood.x + 1.f);
}

// Бег оставляет след, но недалеко: числа шага должны иметь смысл в клетках,
// а не только в тестах на чистую функцию.
TEST_F(ChaseComponentTest, ARunningStepIsHeardCloseByAndNotAcrossTheRoom)
{
	CreateHero(11, 1);
	ChaseComponent* close = CreateEnemy(3, 1);
	ChaseComponent* far = CreateEnemy(11, 3);
	Run(0.1f);

	RoguelikeGame::Noise step;
	step.position = At(1, 1);
	step.radius = RoguelikeGame::RUN_NOISE_RADIUS;
	step.loudness = RoguelikeGame::RUN_NOISE_LOUDNESS;
	step.from = RoguelikeGame::Faction::Player;
	RaiseNoise(step);
	Run(0.1f);

	EXPECT_TRUE(close->IsEngaged()) << "бегущего не слышно в двух клетках";
	EXPECT_FALSE(far->IsEngaged()) << "бегущего слышно через всю комнату";
}

// Шаг не поднимает шума вовсе: на этом держится заход со спины.
TEST_F(ChaseComponentTest, AWalkingStepLeavesNoTrailAtAll)
{
	CreateHero(11, 1);
	ChaseComponent* chase = CreateEnemy(2, 1);
	Run(0.1f);

	float before = chase->GetAwareness();
	Run(1.f);

	EXPECT_EQ(chase->GetAwareness(), before) << "герой прошёл мимо, а враг что-то услышал";
}
