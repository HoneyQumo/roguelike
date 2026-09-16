#include "pch.h"
#include "ActPlan.h"
#include "GameSettings.h"
#include "GameWorld.h"
#include "HealthComponent.h"
#include "PursuitComponent.h"
#include <sstream>

using RoguelikeGame::ActLoader;
using RoguelikeGame::ActPlan;
using RoguelikeGame::HealthComponent;
using RoguelikeGame::PursuitComponent;
using RoguelikeGame::PursuitSpec;
using RoguelikeGame::TileType;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::Vector2Df;

namespace
{
	constexpr float STEP = 0.1f;
	constexpr float BEHIND = -(RoguelikeGame::PURSUIT_SPAWN_GAP + 128.f);
	constexpr float ROUTE_LENGTH = 40000.f;

	ActPlan PlanOf(const std::string& text)
	{
		std::istringstream input(text);

		return ActLoader::Parse(input, "pursuit");
	}

	const char* CHASE_ACT =
		"[act]\n"
		"title Most\n"
		"\n"
		"[library]\n"
		"room Resources/Rooms/act1_bridge_01.config\n"
		"\n"
		"[pursuit]\n"
		"keep 4\n"
		"grow 9\n"
		"respawn 2.5\n"
		"from 0.0 m3 g2\n"
		"from 0.5 a2 h1\n"
		"\n"
		"[rooms]\n"
		"room 0 0\n";

	class PursuitTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			hero = GameWorld::Instance()->CreateGameObject("Hero");
			hero->GetTransform()->SetWorldPosition({0.f, 0.f});

			GameObject* holder = GameWorld::Instance()->CreateGameObject("Pursuit");
			pursuit = holder->AddComponent<PursuitComponent>();
			pursuit->SetHero(hero);
			pursuit->SetRoute({0.f, 0.f}, {ROUTE_LENGTH, 0.f});
			pursuit->SetSpawner([this](TileType enemy, const Vector2Df& place) { return Spawn(enemy, place); });

			// Точки и позади игрока, и впереди: погоня обязана выбрать только задние.
			pursuit->SetPoints({{BEHIND, 0.f}, {BEHIND - 256.f, 0.f}, {-BEHIND, 0.f}, {-BEHIND + 256.f, 0.f}});

			born.clear();
			places.clear();
			kinds.clear();
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		GameObject* Spawn(TileType enemy, const Vector2Df& place)
		{
			GameObject* body = GameWorld::Instance()->CreateGameObject("Hunter");
			body->GetTransform()->SetWorldPosition(place);
			body->AddComponent<HealthComponent>()->SetMaxHealth(10.f);

			born.push_back(body);
			places.push_back(place);
			kinds.push_back(enemy);

			return body;
		}

		PursuitSpec MakeSpec(int keep, int grow, float respawn) const
		{
			PursuitSpec spec;
			spec.keep = keep;
			spec.grow = grow;
			spec.respawn = respawn;
			spec.echelons.push_back({0.f, {{TileType::MarauderSpawn, 1}}});

			return spec;
		}

		void KillAll()
		{
			for (GameObject* enemy : born)
			{
				auto health = enemy->GetComponent<HealthComponent>();
				if (health != nullptr && health->IsAlive())
				{
					health->TakeDamage(100.f);
				}
			}
		}

		void MoveHeroTo(float x)
		{
			hero->GetTransform()->SetWorldPosition({x, 0.f});
		}

		void Run(float seconds)
		{
			for (float passed = 0.f; passed < seconds; passed += STEP)
			{
				GameWorld::Instance()->Update(STEP);
			}
		}

		GameObject* hero = nullptr;
		PursuitComponent* pursuit = nullptr;
		std::vector<GameObject*> born;
		std::vector<Vector2Df> places;
		std::vector<TileType> kinds;
	};
}

TEST(PursuitFormatTests, TheChaseIsReadWithItsPressureAndEchelons)
{
	ActPlan plan = PlanOf(CHASE_ACT);

	ASSERT_FALSE(plan.pursuit.IsEmpty());
	EXPECT_EQ(plan.pursuit.keep, 4);
	EXPECT_EQ(plan.pursuit.grow, 9);
	EXPECT_FLOAT_EQ(plan.pursuit.respawn, 2.5f);

	ASSERT_EQ(plan.pursuit.echelons.size(), 2u);
	EXPECT_FLOAT_EQ(plan.pursuit.echelons[0].fromPart, 0.f);
	EXPECT_EQ(plan.pursuit.echelons[0].entries[0].enemy, TileType::MarauderSpawn);
	EXPECT_EQ(plan.pursuit.echelons[0].entries[0].weight, 3);
	EXPECT_EQ(plan.pursuit.echelons[0].TotalWeight(), 5);
}

TEST(PursuitFormatTests, EchelonsThatGoBackAlongTheWayAreRefused)
{
	EXPECT_THROW(PlanOf(
		"[act]\n"
		"title Most\n"
		"\n"
		"[library]\n"
		"room Resources/Rooms/act1_bridge_01.config\n"
		"\n"
		"[pursuit]\n"
		"keep 4\n"
		"from 0.5 m1\n"
		"from 0.2 a1\n"
		"\n"
		"[rooms]\n"
		"room 0 0\n"), std::runtime_error);
}

TEST(PursuitFormatTests, AnEchelonWithNobodyInItIsRefused)
{
	EXPECT_THROW(PlanOf(
		"[act]\n"
		"title Most\n"
		"\n"
		"[library]\n"
		"room Resources/Rooms/act1_bridge_01.config\n"
		"\n"
		"[pursuit]\n"
		"keep 4\n"
		"from 0.0\n"
		"\n"
		"[rooms]\n"
		"room 0 0\n"), std::runtime_error);
}

TEST_F(PursuitTest, WithoutASpecNobodyChases)
{
	Run(3.f);

	EXPECT_TRUE(born.empty());
	EXPECT_EQ(pursuit->CountChasing(), 0);
}

TEST_F(PursuitTest, TheChaseFillsUpToWhatItKeeps)
{
	pursuit->SetSpec(MakeSpec(3, 6, 0.f));

	Run(1.f);

	EXPECT_EQ(pursuit->CountChasing(), 3) << "the chase is short-handed";
	EXPECT_EQ(born.size(), 3u) << "the chase sent more than it keeps";
}

TEST_F(PursuitTest, TheDeadAreReplacedButOnlyAfterTheirTime)
{
	pursuit->SetSpec(MakeSpec(2, 4, 1.f));
	Run(3.f);
	ASSERT_EQ(born.size(), 2u);

	KillAll();
	Run(0.2f);

	EXPECT_EQ(born.size(), 2u) << "the replacement arrived the same moment";

	Run(2.5f);

	EXPECT_GT(born.size(), 2u) << "nobody came to replace the fallen";
	EXPECT_EQ(pursuit->CountChasing(), 2) << "the chase forgot how many it keeps";
}

TEST_F(PursuitTest, NobodyComesOutInFrontOfTheRunner)
{
	pursuit->SetSpec(MakeSpec(4, 8, 0.f));

	Run(2.f);

	ASSERT_FALSE(places.empty());
	for (const Vector2Df& place : places)
	{
		EXPECT_LT(place.x, 0.f) << "a chaser was waiting ahead instead of running behind";
	}
}

TEST_F(PursuitTest, NobodyComesOutUnderTheFeetOrOverTheHorizon)
{
	pursuit->SetSpec(MakeSpec(4, 8, 0.f));

	Run(2.f);

	ASSERT_FALSE(places.empty());
	for (const Vector2Df& place : places)
	{
		float distance = place.GetLength();

		EXPECT_GE(distance, RoguelikeGame::PURSUIT_SPAWN_GAP) << "a chaser grew out of thin air next to the player";
		EXPECT_LE(distance, RoguelikeGame::PURSUIT_SPAWN_REACH) << "a chaser was sent too far to ever catch up";
	}
}

TEST_F(PursuitTest, StandingStillDrawsMoreOfThemIn)
{
	pursuit->SetSpec(MakeSpec(2, 6, 0.f));
	Run(1.f);
	ASSERT_EQ(pursuit->GetPressure(), 2);

	Run(RoguelikeGame::PURSUIT_GROW_TIME * 2.5f);

	EXPECT_GT(pursuit->GetPressure(), 2) << "loitering costs the player nothing";
	EXPECT_LE(pursuit->GetPressure(), 6) << "the chase grew past its own limit";
}

TEST_F(PursuitTest, RunningAheadTakesThePressureOff)
{
	pursuit->SetSpec(MakeSpec(2, 6, 0.f));
	Run(RoguelikeGame::PURSUIT_GROW_TIME * 2.5f);
	ASSERT_GT(pursuit->GetPressure(), 2);

	MoveHeroTo(RoguelikeGame::PURSUIT_RELAX_STEP * 2.f);
	Run(0.3f);

	EXPECT_EQ(pursuit->GetPressure(), 2) << "getting away gained the player nothing";
}

TEST_F(PursuitTest, ThePressureNeverGrowsPastItsCeiling)
{
	pursuit->SetSpec(MakeSpec(2, 3, 0.f));

	Run(RoguelikeGame::PURSUIT_GROW_TIME * 10.f);

	EXPECT_EQ(pursuit->GetPressure(), 3);
}

TEST_F(PursuitTest, ProgressIsMeasuredAlongTheRouteNotSideways)
{
	pursuit->SetSpec(MakeSpec(1, 2, 0.f));

	EXPECT_FLOAT_EQ(pursuit->GetProgress(), 0.f);

	MoveHeroTo(ROUTE_LENGTH * 0.5f);
	EXPECT_NEAR(pursuit->GetProgress(), 0.5f, 0.01f);

	hero->GetTransform()->SetWorldPosition({ROUTE_LENGTH * 0.5f, 5000.f});
	EXPECT_NEAR(pursuit->GetProgress(), 0.5f, 0.01f) << "wandering across the bridge counted as progress";
}

TEST_F(PursuitTest, ProgressStaysWithinTheRoute)
{
	pursuit->SetSpec(MakeSpec(1, 2, 0.f));

	MoveHeroTo(-ROUTE_LENGTH);
	EXPECT_FLOAT_EQ(pursuit->GetProgress(), 0.f) << "walking back made the progress negative";

	MoveHeroTo(ROUTE_LENGTH * 3.f);
	EXPECT_FLOAT_EQ(pursuit->GetProgress(), 1.f) << "the progress ran past the car";
}

TEST_F(PursuitTest, WhoComesOutDependsOnHowFarTheRunnerGot)
{
	PursuitSpec spec = MakeSpec(1, 2, 0.f);
	spec.echelons.clear();
	spec.echelons.push_back({0.f, {{TileType::MarauderSpawn, 1}}});
	spec.echelons.push_back({0.6f, {{TileType::HeavySpawn, 1}}});
	pursuit->SetSpec(spec);

	EXPECT_EQ(pursuit->PickEnemy(), TileType::MarauderSpawn);

	MoveHeroTo(ROUTE_LENGTH * 0.8f);

	EXPECT_EQ(pursuit->PickEnemy(), TileType::HeavySpawn) << "the bridge ends with the same company it starts with";
}

TEST_F(PursuitTest, AtTheVeryStartTheFirstEchelonIsUsed)
{
	PursuitSpec spec = MakeSpec(1, 2, 0.f);
	spec.echelons.clear();

	// Первый эшелон начинается не с нуля - гнаться в начале всё равно должен кто-то.
	spec.echelons.push_back({0.3f, {{TileType::ShieldSpawn, 1}}});
	pursuit->SetSpec(spec);

	EXPECT_EQ(pursuit->PickEnemy(), TileType::ShieldSpawn);
}

TEST_F(PursuitTest, TheChaseKnowsWhenItIsBreathingDownTheNeck)
{
	pursuit->SetSpec(MakeSpec(1, 2, 0.f));
	Run(1.f);
	ASSERT_EQ(born.size(), 1u);

	born[0]->GetTransform()->SetWorldPosition({RoguelikeGame::PURSUIT_CLOSE_RANGE * 0.5f, 0.f});
	EXPECT_TRUE(pursuit->IsCloseBehind());

	born[0]->GetTransform()->SetWorldPosition({RoguelikeGame::PURSUIT_CLOSE_RANGE * 3.f, 0.f});
	EXPECT_FALSE(pursuit->IsCloseBehind()) << "the player is told the chase is near when it is not";
}

TEST_F(PursuitTest, TheChaseNeverEndsOnItsOwn)
{
	pursuit->SetSpec(MakeSpec(2, 4, 0.f));

	for (int round = 0; round < 5; round++)
	{
		Run(1.f);
		KillAll();
		Run(0.5f);
	}

	EXPECT_GE(pursuit->CountChasing(), 2) << "the chase can be wiped out and walked away from";
	EXPECT_GT(born.size(), 6u);
}
