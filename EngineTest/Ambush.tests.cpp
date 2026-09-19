#include "pch.h"
#include "ActAssembler.h"
#include "AmbushComponent.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include "LevelIntegrity.h"
#include "LevelLoader.h"
#include "LevelZones.h"
#include <GameWorld.h>
#include <map>
#include <sstream>

using RoguelikeGame::AmbushComponent;
using RoguelikeGame::AmbushSpec;
using RoguelikeGame::AssembleAct;
using RoguelikeGame::BuildZones;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelGrid;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::LevelZone;
using RoguelikeGame::TileType;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;

namespace
{
	// Зона z занимает правый верхний угол, точка появления w лежит внутри неё.
	const std::string MAP =
		"[ambush]\n"
		"inner 1.0 g2\n"
		"\n"
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"z Zone:inner\n"
		"w WaveSpawn\n"
		"g GruntSpawn\n"
		"\n"
		"[map]\n"
		"########\n"
		"#@....zw\n"
		"#.....z#\n"
		"########\n";

	LevelData LevelOf(const std::string& text)
	{
		std::istringstream input(text);

		return LevelLoader::Parse(input, "ambush");
	}

	class AmbushTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			level = LevelOf(MAP);
			LevelGrid::SetCurrent(LevelGrid::Build(level));

			std::vector<LevelZone> zones = BuildZones(level);
			ASSERT_EQ(zones.size(), 1u);
			ASSERT_EQ(level.ambushes.size(), 1u);

			GameObject* object = GameWorld::Instance()->CreateGameObject("Ambush");
			ambush = object->AddComponent<AmbushComponent>();
			ambush->SetTargetName(RoguelikeGame::PLAYER_OBJECT_NAME);
			ambush->SetZone(zones[0]);
			ambush->SetSpec(level.ambushes[0]);
			ambush->SetPoints({{0.f, 0.f}});
			ambush->SetSpawner([this](TileType enemy, const XYZEngine::Vector2Df& place)
			{
				called++;

				return GameWorld::Instance()->CreateGameObject("Hunter");
			});

			player = GameWorld::Instance()->CreateGameObject(RoguelikeGame::PLAYER_OBJECT_NAME);
			StandAt(1, 1);
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		void StandAt(int column, int row)
		{
			player->GetTransform()->SetWorldPosition(LevelGrid::Current().ToWorld(column, row));
		}

		void Run(float seconds)
		{
			for (float passed = 0.f; passed < seconds; passed += 0.05f)
			{
				GameWorld::Instance()->Update(0.05f);
			}
		}

		LevelData level;
		AmbushComponent* ambush = nullptr;
		GameObject* player = nullptr;
		int called = 0;
	};
}

TEST_F(AmbushTest, TheAmbushWaitsWhileTheRoomIsEmpty)
{
	Run(3.f);

	EXPECT_FALSE(ambush->IsArmed());
	EXPECT_FALSE(ambush->IsSprung());
	EXPECT_EQ(called, 0);
}

TEST_F(AmbushTest, SteppingIntoTheZoneArmsIt)
{
	StandAt(6, 1);
	Run(0.1f);

	EXPECT_TRUE(ambush->IsArmed());
	EXPECT_FALSE(ambush->IsSprung()) << "засада не должна выскакивать раньше своей задержки";
}

TEST_F(AmbushTest, AfterTheDelayTheyComeOut)
{
	StandAt(6, 1);
	Run(1.5f);

	EXPECT_TRUE(ambush->IsSprung());
	EXPECT_EQ(called, 2);
	EXPECT_EQ(ambush->GetSpawnedCount(), 2);
}

// Один раз - и всё: вернуться в комнату за второй партией нельзя.
TEST_F(AmbushTest, TheAmbushIsSprungOnce)
{
	StandAt(6, 1);
	Run(1.5f);
	ASSERT_EQ(called, 2);

	StandAt(1, 1);
	Run(1.f);
	StandAt(6, 1);
	Run(2.f);

	EXPECT_EQ(called, 2);
}

// Уйти из комнаты, пока отсчёт идёт, уже поздно: их подняли.
TEST_F(AmbushTest, LeavingTheRoomDoesNotCallItOff)
{
	StandAt(6, 1);
	Run(0.1f);
	ASSERT_TRUE(ambush->IsArmed());

	StandAt(1, 1);
	Run(1.5f);

	EXPECT_TRUE(ambush->IsSprung());
}

TEST(AmbushFormatTest, TheAmbushLineReadsZoneDelayAndEnemies)
{
	LevelData level = LevelOf(MAP);

	ASSERT_EQ(level.ambushes.size(), 1u);
	EXPECT_EQ(level.ambushes[0].zoneId, "inner");
	EXPECT_FLOAT_EQ(level.ambushes[0].delay, 1.f);
	ASSERT_EQ(level.ambushes[0].entries.size(), 1u);
	EXPECT_EQ(level.ambushes[0].entries[0].count, 2);
}

TEST(AmbushFormatTest, ARoomCarriesItsAmbushIntoTheAct)
{
	LevelData room = LevelOf(MAP);
	std::map<std::string, LevelData> library = {{"cell", room}};

	RoguelikeGame::ActPlan plan;
	plan.info.title = "Акт";
	plan.rooms = {{"cell", 0, 0, 0, false}};

	LevelData act = AssembleAct(plan, [&library](const std::string& id) -> const LevelData*
	{
		auto found = library.find(id);

		return found == library.end() ? nullptr : &found->second;
	});

	ASSERT_EQ(act.ambushes.size(), 1u);
	EXPECT_EQ(act.ambushes[0].zoneId, "cell@0;0:inner") << "имя зоны обязано совпасть с именем после сборки";

	std::vector<LevelZone> zones = BuildZones(act);
	ASSERT_EQ(zones.size(), 1u);
	EXPECT_EQ(zones[0].id, act.ambushes[0].zoneId);
}

// Засада без точек внутри своей зоны молчит, и заметить это можно только
// в бою, которого не случилось. Пусть лучше скажет проверка карты.
TEST(AmbushFormatTest, AnAmbushWithoutSpotsInsideItsZoneIsAFault)
{
	LevelData level = LevelOf(
		"[ambush]\n"
		"inner 1.0 g2\n"
		"\n"
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"> Exit\n"
		"z Zone:inner\n"
		"g GruntSpawn\n"
		"\n"
		"[map]\n"
		"######\n"
		"#@..z#\n"
		"#>..z#\n"
		"######\n");

	RoguelikeGame::LevelReport report = RoguelikeGame::CheckLevel(level);

	bool found = false;
	for (const RoguelikeGame::LevelIssue& issue : report.issues)
	{
		found = found || issue.fault == RoguelikeGame::LevelFault::AmbushWithoutSpot;
	}

	EXPECT_TRUE(found) << report.Describe();
}
