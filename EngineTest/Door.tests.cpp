#include "pch.h"
#include "DoorComponent.h"
#include "DoorHinge.h"
#include "GameSettings.h"
#include "InventoryComponent.h"
#include "LevelGrid.h"
#include "LevelLoader.h"
#include "PathService.h"
#include <BoxColliderComponent.h>
#include <GameWorld.h>
#include <sstream>

using namespace XYZEngine;
using RoguelikeGame::DoorComponent;
using RoguelikeGame::DoorHinge;
using RoguelikeGame::InventoryComponent;
using RoguelikeGame::ItemDefinition;
using RoguelikeGame::ItemEffectKind;
using RoguelikeGame::LevelGrid;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::LinkDoors;
using RoguelikeGame::PathService;

namespace
{
	const std::string ROOM =
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"+ Door:door_exit\n"
		"[map]\n"
		"#####\n"
		"#.+.#\n"
		"#####\n";

	const std::string DOUBLE_DOOR =
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"+ Door:door_exit\n"
		"[map]\n"
		"#####\n"
		"#.#.#\n"
		"#.+.#\n"
		"#.+.#\n"
		"#.#.#\n"
		"#####\n";

	ItemDefinition MakeKey(const std::string& id, const std::string& target)
	{
		ItemDefinition item;
		item.id = id;
		item.name = id;
		item.effect.kind = ItemEffectKind::Unlock;
		item.effect.target = target;

		return item;
	}

	class DoorTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();
			LoadMap(ROOM);
		}

		void TearDown() override
		{
			GameWorld::Instance()->Clear();
			LevelGrid::SetCurrent(LevelGrid());
			PathService::Reset();
		}

		void LoadMap(const std::string& map)
		{
			std::istringstream input(map);
			LevelGrid::SetCurrent(LevelGrid::Build(LevelLoader::Parse(input, "door")));
			PathService::Reset();
		}

		DoorComponent* CreateDoorAt(int column, int row)
		{
			GameObject* door = GameWorld::Instance()->CreateGameObject("Door");
			door->GetTransform()->SetWorldPosition(LevelGrid::Current().ToWorld(column, row));
			door->AddComponent<BoxColliderComponent>()->SetSize(64.f, 64.f);

			auto component = door->AddComponent<DoorComponent>();
			component->SetDoorId("door_exit");

			return component;
		}

		DoorComponent* CreateDoor()
		{
			return CreateDoorAt(2, 1);
		}

		GameObject* CreateHero()
		{
			GameObject* hero = GameWorld::Instance()->CreateGameObject("Hero");
			hero->AddComponent<InventoryComponent>()->SetCapacity(4);

			return hero;
		}

		GameObject* CreateHeroWithKey()
		{
			GameObject* hero = CreateHero();
			hero->GetComponent<InventoryComponent>()->TryAdd(rustyKey);

			return hero;
		}

		ItemDefinition rustyKey = MakeKey("key_rusty", "door_exit");
	};
}

TEST_F(DoorTest, ClosedDoorBlocksTheWay)
{
	CreateDoor();

	EXPECT_FALSE(LevelGrid::Current().IsPassable(2, 1));
	EXPECT_TRUE(LevelGrid::Current().BlocksSight(2, 1));
}

TEST_F(DoorTest, WithoutAKeyTheDoorStaysShut)
{
	DoorComponent* door = CreateDoor();
	GameObject* hero = CreateHero();
	GameWorld::Instance()->Update(0.016f);

	EXPECT_FALSE(door->TryOpenFor(hero));
	EXPECT_FALSE(door->IsOpen());
	EXPECT_FALSE(LevelGrid::Current().IsPassable(2, 1));
}

TEST_F(DoorTest, WrongKeyDoesNotOpenTheDoor)
{
	DoorComponent* door = CreateDoor();
	GameObject* hero = CreateHero();
	ItemDefinition other = MakeKey("key_vault", "door_vault");
	hero->GetComponent<InventoryComponent>()->TryAdd(other);
	GameWorld::Instance()->Update(0.016f);

	EXPECT_FALSE(door->TryOpenFor(hero));
	EXPECT_FALSE(door->IsOpen());
}

TEST_F(DoorTest, RightKeyOpensTheDoorAndIsSpent)
{
	DoorComponent* door = CreateDoor();
	GameObject* hero = CreateHeroWithKey();
	GameWorld::Instance()->Update(0.016f);

	EXPECT_TRUE(door->TryOpenFor(hero));
	EXPECT_TRUE(door->IsOpen());
	EXPECT_FALSE(hero->GetComponent<InventoryComponent>()->Contains("key_rusty"));
}

TEST_F(DoorTest, OpenedDoorLetsEveryoneThrough)
{
	DoorComponent* door = CreateDoor();
	GameObject* hero = CreateHeroWithKey();
	GameWorld::Instance()->Update(0.016f);
	ASSERT_TRUE(door->TryOpenFor(hero));

	EXPECT_TRUE(LevelGrid::Current().IsPassable(2, 1));
	EXPECT_FALSE(LevelGrid::Current().BlocksSight(2, 1));
}

TEST_F(DoorTest, DoorStaysOpenForever)
{
	DoorComponent* door = CreateDoor();
	GameObject* hero = CreateHeroWithKey();
	GameWorld::Instance()->Update(0.016f);
	ASSERT_TRUE(door->TryOpenFor(hero));

	EXPECT_FALSE(door->TryOpenFor(hero));
	EXPECT_TRUE(door->IsOpen());
}

TEST_F(DoorTest, DoorTellsWhenItOpensAndWhenItRefuses)
{
	DoorComponent* door = CreateDoor();
	GameObject* hero = CreateHero();
	GameWorld::Instance()->Update(0.016f);

	int opened = 0;
	int refused = 0;
	door->SubscribeOpened([&opened](const Vector2Df&) { opened++; });
	door->SubscribeRefused([&refused](const Vector2Df&) { refused++; });

	door->TryOpenFor(hero);
	EXPECT_EQ(refused, 1);
	EXPECT_EQ(opened, 0);

	ItemDefinition key = MakeKey("key_rusty", "door_exit");
	hero->GetComponent<InventoryComponent>()->TryAdd(key);
	door->TryOpenFor(hero);

	EXPECT_EQ(opened, 1);
}

TEST_F(DoorTest, SomeoneWithoutAnInventoryIsIgnored)
{
	DoorComponent* door = CreateDoor();
	GameObject* stone = GameWorld::Instance()->CreateGameObject("Stone");
	GameWorld::Instance()->Update(0.016f);

	EXPECT_FALSE(door->TryOpenFor(stone));
	EXPECT_FALSE(door->IsOpen());
}

TEST_F(DoorTest, LockedDoorAsksForItsKeyByName)
{
	DoorComponent* door = CreateDoor();
	door->SetKeyName("rusty key");
	GameObject* hero = CreateHero();
	GameWorld::Instance()->Update(0.016f);

	EXPECT_EQ(door->GetPrompt(hero), std::string(RoguelikeGame::DOOR_LOCKED_PREFIX) + "rusty key");
	EXPECT_EQ(door->GetRefusal(hero), std::string(RoguelikeGame::DOOR_LOCKED_PREFIX) + "rusty key");
}

TEST_F(DoorTest, DoorWithoutAKnownKeyStillSaysItIsLocked)
{
	DoorComponent* door = CreateDoor();
	GameObject* hero = CreateHero();
	GameWorld::Instance()->Update(0.016f);

	EXPECT_EQ(door->GetRefusal(hero), std::string(RoguelikeGame::DOOR_LOCKED_PREFIX) + RoguelikeGame::DOOR_UNKNOWN_KEY_NAME);
}

TEST_F(DoorTest, WithTheKeyTheDoorOffersToOpen)
{
	DoorComponent* door = CreateDoor();
	GameObject* hero = CreateHeroWithKey();
	GameWorld::Instance()->Update(0.016f);

	EXPECT_EQ(door->GetPrompt(hero), RoguelikeGame::DOOR_OPEN_PROMPT);
	EXPECT_TRUE(door->GetRefusal(hero).empty());
	EXPECT_TRUE(door->IsAvailable());
}

TEST_F(DoorTest, OpenDoorHasNothingToOffer)
{
	DoorComponent* door = CreateDoor();
	GameObject* hero = CreateHeroWithKey();
	GameWorld::Instance()->Update(0.016f);
	ASSERT_TRUE(door->TryOpenFor(hero));

	EXPECT_TRUE(door->GetPrompt(hero).empty());
	EXPECT_FALSE(door->IsAvailable());
}

TEST_F(DoorTest, PressingTheButtonOpensTheDoor)
{
	DoorComponent* door = CreateDoor();
	GameObject* hero = CreateHeroWithKey();
	GameWorld::Instance()->Update(0.016f);

	EXPECT_TRUE(door->Interact(hero));
	EXPECT_TRUE(door->IsOpen());
}

TEST_F(DoorTest, OneKeyOpensBothLeavesOfADoubleDoor)
{
	LoadMap(DOUBLE_DOOR);
	DoorComponent* upper = CreateDoorAt(2, 2);
	DoorComponent* lower = CreateDoorAt(2, 3);
	LinkDoors({upper, lower});

	GameObject* hero = CreateHeroWithKey();
	GameWorld::Instance()->Update(0.016f);

	ASSERT_TRUE(upper->TryOpenFor(hero));

	EXPECT_TRUE(lower->IsOpen());
	EXPECT_TRUE(LevelGrid::Current().IsPassable(2, 2));
	EXPECT_TRUE(LevelGrid::Current().IsPassable(2, 3));
	EXPECT_FALSE(hero->GetComponent<InventoryComponent>()->Contains("key_rusty"));
}

TEST_F(DoorTest, LinkedDoorsWithAnotherIdStayShut)
{
	LoadMap(DOUBLE_DOOR);
	DoorComponent* upper = CreateDoorAt(2, 2);
	DoorComponent* lower = CreateDoorAt(2, 3);
	lower->SetDoorId("door_vault");
	LinkDoors({upper, lower});

	GameObject* hero = CreateHeroWithKey();
	GameWorld::Instance()->Update(0.016f);

	ASSERT_TRUE(upper->TryOpenFor(hero));

	EXPECT_FALSE(lower->IsOpen());
	EXPECT_FALSE(LevelGrid::Current().IsPassable(2, 3));
}

TEST_F(DoorTest, ClosedLeafStandsAtItsClosedAngle)
{
	DoorComponent* door = CreateDoor();
	DoorHinge hinge;
	hinge.closedAngle = 0.f;
	hinge.openAngle = -90.f;
	door->SetHinge(hinge);

	EXPECT_NEAR(door->GetLeafAngle(), 0.f, 0.01f);
	EXPECT_FALSE(door->IsSwinging());
}

TEST_F(DoorTest, LeafTurnsAllTheWayWhileTheDoorSwings)
{
	DoorComponent* door = CreateDoor();
	DoorHinge hinge;
	hinge.closedAngle = 0.f;
	hinge.openAngle = -90.f;
	door->SetHinge(hinge);

	GameObject* hero = CreateHeroWithKey();
	GameWorld::Instance()->Update(0.016f);
	ASSERT_TRUE(door->TryOpenFor(hero));

	EXPECT_NEAR(door->GetLeafAngle(), 0.f, 0.01f);
	EXPECT_TRUE(door->IsSwinging());

	GameWorld::Instance()->Update(0.5f * RoguelikeGame::DOOR_SWING_TIME);
	float halfway = door->GetLeafAngle();
	EXPECT_LT(halfway, -0.01f);
	EXPECT_GT(halfway, -89.99f);

	GameWorld::Instance()->Update(RoguelikeGame::DOOR_SWING_TIME);

	EXPECT_NEAR(door->GetLeafAngle(), -90.f, 0.01f);
	EXPECT_FALSE(door->IsSwinging());
}

TEST_F(DoorTest, LeafStopsAtTheOpenAngle)
{
	DoorComponent* door = CreateDoor();
	DoorHinge hinge;
	hinge.closedAngle = 0.f;
	hinge.openAngle = -90.f;
	door->SetHinge(hinge);

	GameObject* hero = CreateHeroWithKey();
	GameWorld::Instance()->Update(0.016f);
	ASSERT_TRUE(door->TryOpenFor(hero));

	for (int step = 0; step < 20; step++)
	{
		GameWorld::Instance()->Update(RoguelikeGame::DOOR_SWING_TIME);
	}

	EXPECT_NEAR(door->GetLeafAngle(), -90.f, 0.01f);
}
