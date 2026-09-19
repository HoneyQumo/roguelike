#include "pch.h"
#include "ActAssembler.h"
#include "ActRoutes.h"
#include "ItemCatalogLoader.h"
#include "LevelIntegrity.h"
#include "LevelLoader.h"
#include "ProjectFiles.h"
#include "PropCatalog.h"

using RoguelikeGame::CheckLevel;
using RoguelikeGame::ItemCatalog;
using RoguelikeGame::ItemCatalogLoader;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelReport;
using RoguelikeGame::LoadAct;
using RoguelikeGame::PropCatalog;
using RoguelikeGame::TileType;

namespace
{
	const std::string STREET = "Resources/Acts/act1_street.config";

	class StreetRoutesTest : public ProjectFiles::Test
	{
	protected:
		void SetUp() override
		{
			ProjectFiles::Test::SetUp();
			if (!isFound)
			{
				return;
			}

			items = ItemCatalogLoader::Load("Resources/Items/items.config");
			props = PropCatalog::Load("Resources/Props/props.config");
			street = LoadAct(STREET);
		}

		ActRoutes::Closer Closer() { return ActRoutes::Closer(street); }

		void CloseKeyWay()
		{
			Closer().CloseDoor("door_shutter");
			Closer().DropItem("key_shutter");
		}

		void CloseBreachWay()
		{
			Closer().CloseProps("window_boarded");
		}

		void CloseServiceWay()
		{
			Closer().CloseDoor("door_service");
			Closer().DropLever("door_service");
		}

		LevelReport Check() const { return CheckLevel(street, items, props); }

		ItemCatalog items;
		PropCatalog props;
		LevelData street;
	};
}

TEST_F(StreetRoutesTest, TheShutterStillNeedsItsKey)
{
	ASSERT_TRUE(isFound) << previous.string();

	EXPECT_TRUE(ActRoutes::HasDoor(street, "door_shutter"));

	bool hasKey = false;
	for (const auto& placement : street.items)
	{
		const RoguelikeGame::ItemDefinition* item = items.Find(placement.itemId);
		hasKey = hasKey || (item != nullptr && item->effect.target == "door_shutter");
	}

	EXPECT_TRUE(hasKey) << "ставню нечем открыть";
}

// Окно - тот же приём, что треснувшая стена в тюрьме: стоит стеной, ломается,
// открывает клетку. Значит к нему нужен подход, иначе здоровье ничего не значит.
TEST_F(StreetRoutesTest, TheBoardedWindowCanBeBrokenAndWalkedThrough)
{
	ASSERT_TRUE(isFound) << previous.string();

	const RoguelikeGame::PropPlacement* window = ActRoutes::FindProp(street, "window_boarded");
	ASSERT_NE(window, nullptr) << "ломать нечего";

	const RoguelikeGame::PropDefinition* definition = props.Find("window_boarded");
	ASSERT_NE(definition, nullptr);
	EXPECT_TRUE(definition->IsDestructible());
	EXPECT_TRUE(definition->isSolid) << "целое окно обязано держать клетку закрытой";
	EXPECT_FALSE(definition->leavesWreck) << "остов не даст пройти там, где сломали";

	EXPECT_EQ(TileAt(street, window->column, window->row - 1), TileType::Floor) << "к окну не подойти";
	EXPECT_EQ(TileAt(street, window->column, window->row + 1), TileType::Floor) << "за окном не пол";
}

TEST_F(StreetRoutesTest, TheServiceDoorOpensByHoldingTheSwitch)
{
	ASSERT_TRUE(isFound) << previous.string();

	ASSERT_TRUE(ActRoutes::HasDoor(street, "door_service"));

	const RoguelikeGame::FixturePlacement* lever = ActRoutes::FindFixture(street.levers, "door_service");
	ASSERT_NE(lever, nullptr) << "служебную дверь нечем открыть";
	EXPECT_GT(lever->holdTime, 0.f) << "рубильник обязан стоить времени, иначе это вторая ставня без ключа";
}

// Каждый путь обязан выводить сам по себе.
TEST_F(StreetRoutesTest, TheKeyWayAloneIsEnough)
{
	ASSERT_TRUE(isFound) << previous.string();

	CloseBreachWay();
	CloseServiceWay();

	EXPECT_TRUE(Check().IsClean()) << Check().Describe();
}

TEST_F(StreetRoutesTest, TheBreachWayAloneIsEnough)
{
	ASSERT_TRUE(isFound) << previous.string();

	CloseKeyWay();
	CloseServiceWay();

	EXPECT_TRUE(Check().IsClean()) << Check().Describe();
}

TEST_F(StreetRoutesTest, TheServiceWayAloneIsEnough)
{
	ASSERT_TRUE(isFound) << previous.string();

	CloseKeyWay();
	CloseBreachWay();

	EXPECT_TRUE(Check().IsClean()) << Check().Describe();
}

// Без этого три теста выше проходили бы, ничего не закрывая.
TEST_F(StreetRoutesTest, WithAllThreeShutTheExitIsGone)
{
	ASSERT_TRUE(isFound) << previous.string();

	CloseKeyWay();
	CloseBreachWay();
	CloseServiceWay();

	EXPECT_FALSE(Check().IsClean()) << "закладки ничего не закрывают";
}

TEST_F(StreetRoutesTest, EveryRouteHasItsOwnTrouble)
{
	ASSERT_TRUE(isFound) << previous.string();

	int traps = 0;
	for (const auto& prop : street.props)
	{
		const RoguelikeGame::PropDefinition* definition = props.Find(prop.propId);
		traps += definition != nullptr && definition->IsTrap() ? 1 : 0;
	}

	EXPECT_GE(traps, 3) << "по ловушке на путь - минимум";
	EXPECT_GE(street.ambushes.size(), 3u) << "по засаде на путь - минимум";
}

TEST_F(StreetRoutesTest, TheStreetIsSoundAsItShips)
{
	ASSERT_TRUE(isFound) << previous.string();

	EXPECT_TRUE(Check().IsClean()) << Check().Describe();
}
