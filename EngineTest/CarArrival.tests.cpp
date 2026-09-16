#include "pch.h"
#include "CarArrival.h"
#include "EscapeCarComponent.h"
#include "LevelIntegrity.h"
#include "LevelLoader.h"
#include <GameWorld.h>
#include <sstream>

using RoguelikeGame::ARRIVAL_CALL_RANGE;
using RoguelikeGame::ARRIVAL_ENTRY_OFFSET;
using RoguelikeGame::ArrivalPose;
using RoguelikeGame::CarPose;
using RoguelikeGame::CheckLevel;
using RoguelikeGame::EscapeCarComponent;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelFault;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::LevelReport;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::Vector2Df;

namespace
{
	const Vector2Df PARK = {2400.f, 600.f};

	LevelData LevelOf(const std::string& text)
	{
		std::istringstream input(text);

		return LevelLoader::Parse(input, "arrival");
	}
}

TEST(CarArrivalTest, TheCarWaitsOffTheMapNoseFirst)
{
	CarPose start = ArrivalPose(PARK, 0.f);

	EXPECT_FLOAT_EQ(start.place.x, PARK.x + ARRIVAL_ENTRY_OFFSET);
	EXPECT_FLOAT_EQ(start.place.y, PARK.y);
	EXPECT_GT(start.angle, 90.f) << "the car arrives driving backwards";
}

TEST(CarArrivalTest, ItStopsExactlyOnItsPlaceFacingTheWayOut)
{
	CarPose parked = ArrivalPose(PARK, 1.f);

	EXPECT_FLOAT_EQ(parked.place.x, PARK.x);
	EXPECT_FLOAT_EQ(parked.place.y, PARK.y) << "the car stayed sideways after the skid";
	EXPECT_FLOAT_EQ(parked.angle, 0.f);
}

TEST(CarArrivalTest, ItOnlyEverDrivesTowardsThePlace)
{
	float previous = ArrivalPose(PARK, 0.f).place.x;
	for (int step = 1; step <= 20; step++)
	{
		float now = ArrivalPose(PARK, step / 20.f).place.x;

		EXPECT_LE(now, previous) << "the car backed up at step " << step;
		previous = now;
	}
}

TEST(CarArrivalTest, ItBrakesInsteadOfCruisingIn)
{
	float half = ArrivalPose(PARK, 0.5f).place.x - PARK.x;

	// Половина времени - и почти весь путь позади: остальное машина уже тормозит.
	EXPECT_LT(half, ARRIVAL_ENTRY_OFFSET * 0.25f) << "the car crawled in at an even pace";
}

TEST(CarArrivalTest, TheSkidComesWithTheBrakingAndNotBefore)
{
	EXPECT_FLOAT_EQ(ArrivalPose(PARK, 0.3f).angle, ArrivalPose(PARK, 0.f).angle)
		<< "the car started turning while still far away";

	float turning = ArrivalPose(PARK, 0.8f).angle;

	EXPECT_LT(turning, ArrivalPose(PARK, 0.f).angle);
	EXPECT_GT(turning, 0.f);
}

TEST(CarArrivalTest, TheTailSlidesOutAndComesBack)
{
	float slide = ArrivalPose(PARK, 0.78f).place.y - PARK.y;

	EXPECT_GT(std::abs(slide), 1.f) << "the car braked without sliding at all";
	EXPECT_FLOAT_EQ(ArrivalPose(PARK, 1.f).place.y, PARK.y);
}

namespace
{
	class EscapeCarTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			hero = GameWorld::Instance()->CreateGameObject("Hero");
			hero->GetTransform()->SetWorldPosition({0.f, 0.f});

			GameObject* object = GameWorld::Instance()->CreateGameObject("EscapeCar");
			object->GetTransform()->SetWorldPosition(ArrivalPose(PARK, 0.f).place);

			car = object->AddComponent<EscapeCarComponent>();
			car->SetCarId("city_car");
			car->SetParkPlace(PARK);
			car->SetHero(hero);
			car->SetArrived(false);

			calls = 0;
			car->SubscribeCalled([this]() { calls++; });
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		void Run(float seconds)
		{
			for (float passed = 0.f; passed < seconds; passed += 0.05f)
			{
				GameWorld::Instance()->Update(0.05f);
			}
		}

		void PutHeroAt(float x) { hero->GetTransform()->SetWorldPosition({x, PARK.y}); }

		GameObject* hero = nullptr;
		EscapeCarComponent* car = nullptr;
		int calls = 0;
	};
}

TEST_F(EscapeCarTest, ACarThatHasNotArrivedCannotBeBoarded)
{
	car->SetReady(true);

	EXPECT_FALSE(car->IsAvailable());
	EXPECT_FALSE(car->Interact(hero)) << "the player drove away in a car that is not there";
}

TEST_F(EscapeCarTest, ItIsCalledOnlyWhenTheHeroIsNear)
{
	PutHeroAt(PARK.x - ARRIVAL_CALL_RANGE * 2.f);
	Run(0.3f);

	EXPECT_EQ(calls, 0) << "the car set off while the hero was still half a bridge away";

	PutHeroAt(PARK.x - ARRIVAL_CALL_RANGE * 0.5f);
	Run(0.3f);

	EXPECT_EQ(calls, 1);
}

TEST_F(EscapeCarTest, ItIsCalledByThePlaceAndNotByWhereTheCarWaits)
{
	// Герой стоит на самом месте парковки, а машина ждёт далеко справа.
	PutHeroAt(PARK.x);
	Run(0.2f);

	float toTheCar = (car->GetGameObject()->GetTransform()->GetWorldPosition()
		- hero->GetTransform()->GetWorldPosition()).GetLength();

	ASSERT_GT(toTheCar, ARRIVAL_CALL_RANGE) << "the car was near enough to be heard anyway";
	EXPECT_EQ(calls, 1);
}

TEST_F(EscapeCarTest, TheSceneStartsOnceAndNotOnEveryStep)
{
	PutHeroAt(PARK.x);
	Run(1.5f);

	EXPECT_EQ(calls, 1);
}

TEST_F(EscapeCarTest, OnceItHasArrivedItCanBeBoarded)
{
	car->SetReady(true);
	car->SetArrived(true);

	EXPECT_TRUE(car->IsAvailable());
	EXPECT_TRUE(car->Interact(hero));
	EXPECT_TRUE(car->IsBoarded());
}

TEST(CarArrivalTest, ALevelLeavesThroughItsCarWithoutAnExitTile)
{
	LevelData level = LevelOf(
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"E Escape:city_car\n"
		"[map]\n"
		"#######\n"
		"#@...E#\n"
		"#######\n");
	level.info.nextLevelId = "street";

	LevelReport report = CheckLevel(level);

	EXPECT_EQ(report.Count(LevelFault::NoExit), 0) << report.Describe();
}

TEST(CarArrivalTest, ACarWalledOffFromTheHeroIsReported)
{
	LevelData level = LevelOf(
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"E Escape:city_car\n"
		"[map]\n"
		"#########\n"
		"#@...#E.#\n"
		"#########\n");
	level.info.nextLevelId = "street";

	LevelReport report = CheckLevel(level);

	EXPECT_EQ(report.Count(LevelFault::ExitUnreachable), 1) << report.Describe();
}
