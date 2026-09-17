#include "pch.h"
#include "ActAssembler.h"
#include "CutscenePlayerComponent.h"
#include "CutsceneTimeline.h"
#include "GameWorld.h"
#include "LevelLoader.h"
#include "GameSettings.h"
#include <cmath>
#include "TileAtlas.h"
#include "ProjectFiles.h"
#include <sstream>

using RoguelikeGame::CutsceneBeat;
using RoguelikeGame::CutscenePlayerComponent;
using RoguelikeGame::CutsceneTimeline;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelLoader;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;

namespace
{
	constexpr float STEP = 0.1f;

	class CutscenePlayerTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			GameObject* holder = GameWorld::Instance()->CreateGameObject("Cutscene");
			scene = holder->AddComponent<CutscenePlayerComponent>();

			started.clear();
			finished = 0;
			driven = 0.f;

			scene->SubscribeBeatStarted([this](const std::string& beat) { started.push_back(beat); });
			scene->SubscribeFinished([this]() { finished++; });
			scene->SetHandler("drive", [this](float deltaTime) { driven += deltaTime; });
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		void Run(float seconds)
		{
			for (float passed = 0.f; passed < seconds; passed += STEP)
			{
				GameWorld::Instance()->Update(STEP);
			}
		}

		CutscenePlayerComponent* scene = nullptr;
		std::vector<std::string> started;
		int finished = 0;
		float driven = 0.f;
	};
}

TEST(CutsceneTimelineTests, AnEmptyTimelineIsOverBeforeItStarts)
{
	CutsceneTimeline timeline;
	timeline.SetBeats({});

	EXPECT_TRUE(timeline.IsOver());
	EXPECT_TRUE(timeline.IsEmpty());
	EXPECT_FALSE(timeline.Advance(1.f));
}

TEST(CutsceneTimelineTests, TheFirstStepEntersTheFirstBeat)
{
	CutsceneTimeline timeline;
	timeline.SetBeats({{"one", 1.f}, {"two", 1.f}});

	EXPECT_TRUE(timeline.Advance(0.f));
	EXPECT_EQ(timeline.GetCurrentAction(), "one");
	EXPECT_EQ(timeline.GetCurrent(), 0);
}

TEST(CutsceneTimelineTests, BeatsFollowEachOtherInOrder)
{
	CutsceneTimeline timeline;
	timeline.SetBeats({{"one", 1.f}, {"two", 1.f}, {"three", 1.f}});

	timeline.Advance(0.f);
	EXPECT_FALSE(timeline.Advance(0.5f));
	EXPECT_EQ(timeline.GetCurrentAction(), "one");

	EXPECT_TRUE(timeline.Advance(0.6f));
	EXPECT_EQ(timeline.GetCurrentAction(), "two");

	EXPECT_TRUE(timeline.Advance(1.f));
	EXPECT_EQ(timeline.GetCurrentAction(), "three");
}

TEST(CutsceneTimelineTests, OneLongStepCanCrossSeveralBeats)
{
	CutsceneTimeline timeline;
	timeline.SetBeats({{"one", 0.2f}, {"two", 0.2f}, {"three", 5.f}});

	timeline.Advance(0.f);
	timeline.Advance(0.5f);

	EXPECT_EQ(timeline.GetCurrentAction(), "three");
	EXPECT_FALSE(timeline.IsOver());
}

TEST(CutsceneTimelineTests, TheTimelineEndsAfterTheLastBeat)
{
	CutsceneTimeline timeline;
	timeline.SetBeats({{"only", 1.f}});

	timeline.Advance(0.f);
	timeline.Advance(1.5f);

	EXPECT_TRUE(timeline.IsOver());
}

TEST(CutsceneTimelineTests, ProgressGrowsInsideABeat)
{
	CutsceneTimeline timeline;
	timeline.SetBeats({{"only", 2.f}});

	timeline.Advance(0.f);
	EXPECT_FLOAT_EQ(timeline.GetBeatProgress(), 0.f);

	timeline.Advance(1.f);

	EXPECT_NEAR(timeline.GetBeatProgress(), 0.5f, 0.01f);
}

TEST_F(CutscenePlayerTest, NothingHappensUntilItIsPlayed)
{
	scene->SetBeats({{"drive", 1.f}});

	Run(1.f);

	EXPECT_TRUE(started.empty());
	EXPECT_FLOAT_EQ(driven, 0.f);
}

TEST_F(CutscenePlayerTest, BeatsAreAnnouncedInOrderAndTheEndIsReportedOnce)
{
	scene->SetBeats({{"board", 0.2f}, {"drive", 0.3f}, {"leave", 0.2f}});
	scene->Play();

	Run(2.f);

	ASSERT_EQ(started.size(), 3u);
	EXPECT_EQ(started[0], "board");
	EXPECT_EQ(started[1], "drive");
	EXPECT_EQ(started[2], "leave");
	EXPECT_EQ(finished, 1);
	EXPECT_FALSE(scene->IsPlaying());

	Run(1.f);

	EXPECT_EQ(finished, 1) << "the end was reported twice";
}

TEST_F(CutscenePlayerTest, TheHandlerRunsOnlyWhileItsBeatIsOn)
{
	scene->SetBeats({{"board", 0.5f}, {"drive", 0.5f}, {"leave", 0.5f}});
	scene->Play();

	Run(2.f);

	EXPECT_GT(driven, 0.2f) << "the handler never ran";
	EXPECT_LT(driven, 0.9f) << "the handler ran outside its beat";
}

TEST_F(CutscenePlayerTest, AnEmptySceneRefusesToPlay)
{
	scene->Play();

	Run(1.f);

	EXPECT_FALSE(scene->IsPlaying());
	EXPECT_EQ(finished, 0);
}

namespace
{
	class ShippedEscapeTest : public ProjectFiles::Test
	{
	};
}

TEST_F(ShippedEscapeTest, TheOnlyWayOffTheBridgeIsTheCar)
{
	ASSERT_TRUE(isFound) << previous.string();

	LevelData bridge = RoguelikeGame::LoadAct("Resources/Acts/act1_bridge.config");

	ASSERT_EQ(bridge.escapes.size(), 1u) << "the bridge needs exactly one escape car";

	int teleports = 0;
	for (int row = 0; row < bridge.height; row++)
	{
		for (int column = 0; column < static_cast<int>(bridge.tiles[row].size()); column++)
		{
			if (bridge.tiles[row][column] == RoguelikeGame::TileType::Exit)
			{
				teleports++;
			}
		}
	}

	// Переходом служит машина: второй выход рядом с ней сводил сцену на нет.
	EXPECT_EQ(teleports, 0) << "the bridge still has a teleport next to the car";

	const RoguelikeGame::FixturePlacement& car = bridge.escapes.front();

	// Машина - финиш забега, а не привал посередине моста.
	EXPECT_GT(car.column * 2, bridge.width) << "the car waits in the middle of the run";
}

/**
*	За машиной нужна полоса разгона на всю видимую часть побега.
*
*	Без неё машина проходит сквозь отбойник и остаток сцены едет по черноте.
*	Длина считается из самих таймингов сцены, а не забита числом: удлинишь
*	поездку - тест сразу потребует дороги.
*/
TEST_F(ShippedEscapeTest, TheCarHasRoadToDriveOffOn)
{
	ASSERT_TRUE(isFound) << previous.string();

	LevelData bridge = RoguelikeGame::LoadAct("Resources/Acts/act1_bridge.config");
	ASSERT_EQ(bridge.escapes.size(), 1u);

	const RoguelikeGame::FixturePlacement& car = bridge.escapes.front();

	float ramp = RoguelikeGame::ESCAPE_CAR_SPEED / RoguelikeGame::ESCAPE_CAR_PICKUP;
	float visible = RoguelikeGame::ESCAPE_BOARD_TIME + RoguelikeGame::ESCAPE_DRIVE_TIME;
	float distance = 0.5f * RoguelikeGame::ESCAPE_CAR_SPEED * ramp
		+ std::max(0.f, visible - ramp) * RoguelikeGame::ESCAPE_CAR_SPEED;
	int needed = static_cast<int>(std::ceil(distance / RoguelikeGame::TILE_SIZE));

	ASSERT_LT(car.column + needed, bridge.width)
		<< "the escape drives " << needed << " tiles and the bridge ends after " << bridge.width - car.column;

	for (int step = 0; step <= needed; step++)
	{
		EXPECT_TRUE(RoguelikeGame::IsRoadAt(bridge, car.column + step, car.row))
			<< "nothing to drive on at column " << car.column + step;
	}
}

TEST_F(ShippedEscapeTest, TheCarHasRoadToArriveOn)
{
	ASSERT_TRUE(isFound) << previous.string();

	LevelData bridge = RoguelikeGame::LoadAct("Resources/Acts/act1_bridge.config");
	ASSERT_EQ(bridge.escapes.size(), 1u);

	const RoguelikeGame::FixturePlacement& car = bridge.escapes.front();
	int runway = static_cast<int>(RoguelikeGame::ARRIVAL_ENTRY_OFFSET / RoguelikeGame::TILE_SIZE);

	// Машина начинает разгон в runway клетках восточнее своего места: без дороги
	// на всём этом отрезке она выезжает из-за отбойника или вовсе из-за края карты.
	ASSERT_LT(car.column + runway, bridge.width) << "the car starts its run beyond the edge of the map";

	for (int step = 0; step <= runway; step++)
	{
		EXPECT_TRUE(RoguelikeGame::IsRoadAt(bridge, car.column + step, car.row))
			<< "nothing to drive on at column " << car.column + step;
	}
}

TEST_F(ShippedEscapeTest, TheCarIsTheFinishOfAChaseAndNotAReward)
{
	ASSERT_TRUE(isFound) << previous.string();

	LevelData bridge = RoguelikeGame::LoadAct("Resources/Acts/act1_bridge.config");

	// Мост - беговая локация: гейтом служит расстояние, а не зачистка.
	ASSERT_FALSE(bridge.escapes.empty()) << "there is nothing to run towards";
	EXPECT_TRUE(bridge.waves.empty()) << "waves would make the player stop and clear";
	EXPECT_FALSE(bridge.pursuit.IsEmpty()) << "nobody chases the player across the bridge";
}
