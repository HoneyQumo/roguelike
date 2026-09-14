#include "pch.h"
#include "DoorComponent.h"
#include "InventoryComponent.h"
#include "LevelGrid.h"
#include "LevelLoader.h"
#include "PathService.h"
#include <BoxColliderComponent.h>
#include <GameWorld.h>
#include <sstream>

using namespace XYZEngine;
using RoguelikeGame::DoorComponent;
using RoguelikeGame::InventoryComponent;
using RoguelikeGame::ItemDefinition;
using RoguelikeGame::ItemEffectKind;
using RoguelikeGame::LevelGrid;
using RoguelikeGame::LevelLoader;
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

			std::istringstream input(ROOM);
			LevelGrid::SetCurrent(LevelGrid::Build(LevelLoader::Parse(input, "door")));
			PathService::Reset();
		}

		void TearDown() override
		{
			GameWorld::Instance()->Clear();
			LevelGrid::SetCurrent(LevelGrid());
			PathService::Reset();
		}

		DoorComponent* CreateDoor()
		{
			GameObject* door = GameWorld::Instance()->CreateGameObject("Door");
			door->GetTransform()->SetWorldPosition(LevelGrid::Current().ToWorld(2, 1));
			door->AddComponent<BoxColliderComponent>()->SetSize(64.f, 64.f);

			auto component = door->AddComponent<DoorComponent>();
			component->SetDoorId("door_exit");

			return component;
		}

		GameObject* CreateHero()
		{
			GameObject* hero = GameWorld::Instance()->CreateGameObject("Hero");
			hero->AddComponent<InventoryComponent>()->SetCapacity(4);

			return hero;
		}
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
	GameObject* hero = CreateHero();
	ItemDefinition key = MakeKey("key_rusty", "door_exit");
	auto inventory = hero->GetComponent<InventoryComponent>();
	inventory->TryAdd(key);
	GameWorld::Instance()->Update(0.016f);

	EXPECT_TRUE(door->TryOpenFor(hero));
	EXPECT_TRUE(door->IsOpen());
	EXPECT_FALSE(inventory->Contains("key_rusty"));
}

TEST_F(DoorTest, OpenedDoorLetsEveryoneThrough)
{
	DoorComponent* door = CreateDoor();
	GameObject* hero = CreateHero();
	ItemDefinition key = MakeKey("key_rusty", "door_exit");
	hero->GetComponent<InventoryComponent>()->TryAdd(key);
	GameWorld::Instance()->Update(0.016f);
	ASSERT_TRUE(door->TryOpenFor(hero));

	EXPECT_TRUE(LevelGrid::Current().IsPassable(2, 1));
	EXPECT_FALSE(LevelGrid::Current().BlocksSight(2, 1));
}

TEST_F(DoorTest, DoorStaysOpenForever)
{
	DoorComponent* door = CreateDoor();
	GameObject* hero = CreateHero();
	ItemDefinition key = MakeKey("key_rusty", "door_exit");
	auto inventory = hero->GetComponent<InventoryComponent>();
	inventory->TryAdd(key);
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
