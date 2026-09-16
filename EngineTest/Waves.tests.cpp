#include "pch.h"
#include "ActAssembler.h"
#include "ActPlan.h"
#include "EnemyCatalog.h"
#include "GameSettings.h"
#include "GameWorld.h"
#include "HealthComponent.h"
#include "LevelLoader.h"
#include "ProjectFiles.h"
#include "EscapeCarComponent.h"
#include "WaveDirectorComponent.h"
#include <sstream>

using RoguelikeGame::ActLoader;
using RoguelikeGame::ActPlan;
using RoguelikeGame::HealthComponent;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::TileType;
using RoguelikeGame::WaveDirectorComponent;
using RoguelikeGame::WaveSpec;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::Vector2Df;

namespace
{
	constexpr float STEP = 0.1f;

	LevelData LevelOf(const std::string& text)
	{
		std::istringstream input(text);

		return LevelLoader::Parse(input, "waves");
	}

	const char* HOLDOUT =
		"[level]\n"
		"kind holdout\n"
		"\n"
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"* WaveSpawn\n"
		"g MarauderSpawn\n"
		"a AssaultSpawn\n"
		"\n"
		"[waves]\n"
		"wave 2 g3 a1\n"
		"wave 4 a2\n"
		"\n"
		"[map]\n"
		"#######\n"
		"#*...*#\n"
		"#..@..#\n"
		"#*...*#\n"
		"#######\n";

	class WaveDirectorTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			GameObject* holder = GameWorld::Instance()->CreateGameObject("WaveDirector");
			director = holder->AddComponent<WaveDirectorComponent>();
			director->SetPoints({{0.f, 0.f}, {320.f, 0.f}, {0.f, 320.f}, {320.f, 320.f}});
			director->SetSpawner([this](TileType enemy, const Vector2Df& place) { return Spawn(enemy, place); });

			born.clear();
			places.clear();
			startedWaves.clear();
			clears = 0;

			director->SubscribeCleared([this]() { clears++; });
			director->SubscribeWaveStarted([this](int current, int) { startedWaves.push_back(current); });
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		GameObject* Spawn(TileType enemy, const Vector2Df& place)
		{
			GameObject* body = GameWorld::Instance()->CreateGameObject("Enemy");
			body->GetTransform()->SetWorldPosition(place);
			body->AddComponent<HealthComponent>()->SetMaxHealth(10.f);

			born.push_back(body);
			places.push_back(place);

			return body;
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

		void Run(float seconds)
		{
			for (float passed = 0.f; passed < seconds; passed += STEP)
			{
				GameWorld::Instance()->Update(STEP);
			}
		}

		WaveDirectorComponent* director = nullptr;
		std::vector<GameObject*> born;
		std::vector<Vector2Df> places;
		std::vector<int> startedWaves;
		int clears = 0;
	};
}

TEST(WaveFormatTests, WavesAreReadWithTheirDelaysAndCounts)
{
	LevelData level = LevelOf(HOLDOUT);

	ASSERT_EQ(level.waves.size(), 2u);
	EXPECT_FLOAT_EQ(level.waves[0].delay, 2.f);
	EXPECT_EQ(level.waves[0].Size(), 4);
	EXPECT_EQ(level.waves[0].entries[0].enemy, TileType::MarauderSpawn);
	EXPECT_EQ(level.waves[0].entries[0].count, 3);
	EXPECT_EQ(level.waves[1].Size(), 2);
}

TEST(WaveFormatTests, SpawnPointsAreOrdinaryFloorForEverythingElse)
{
	LevelData level = LevelOf(HOLDOUT);

	EXPECT_EQ(level.tiles[1][1], TileType::WaveSpawn);
}

TEST(WaveFormatTests, AWaveAskingForSomethingThatIsNotAnEnemyIsRefused)
{
	EXPECT_THROW(LevelOf(
		"[level]\n"
		"kind holdout\n"
		"\n"
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"\n"
		"[waves]\n"
		"wave 2 .3\n"
		"\n"
		"[map]\n"
		"###\n"
		"#.#\n"
		"###\n"), std::runtime_error);
}

TEST(WaveFormatTests, AWaveWithoutEnemiesIsRefused)
{
	EXPECT_THROW(LevelOf(
		"[level]\n"
		"kind holdout\n"
		"\n"
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"\n"
		"[waves]\n"
		"wave 2\n"
		"\n"
		"[map]\n"
		"###\n"
		"#.#\n"
		"###\n"), std::runtime_error);
}

TEST_F(WaveDirectorTest, TheFirstWaveWaitsOutItsDelay)
{
	director->SetWaves({{1.f, {{TileType::MarauderSpawn, 2}}}});

	Run(0.5f);
	EXPECT_TRUE(born.empty()) << "the wave came early";

	Run(1.f);

	EXPECT_EQ(born.size(), 2u);
	EXPECT_EQ(startedWaves.size(), 1u);
}

TEST_F(WaveDirectorTest, AWaveBringsExactlyWhatItPromises)
{
	director->SetWaves({{0.f, {{TileType::MarauderSpawn, 3}, {TileType::AssaultSpawn, 2}}}});

	Run(0.3f);

	EXPECT_EQ(born.size(), 5u);
}

TEST_F(WaveDirectorTest, TheNextWaveWaitsForTheLastEnemyOfThisOne)
{
	director->SetWaves({{0.f, {{TileType::MarauderSpawn, 2}}}, {0.f, {{TileType::AssaultSpawn, 1}}}});

	Run(0.3f);
	ASSERT_EQ(born.size(), 2u);

	Run(3.f);
	EXPECT_EQ(born.size(), 2u) << "the second wave did not wait";

	KillAll();
	Run(0.5f);

	EXPECT_EQ(born.size(), 3u);
	EXPECT_EQ(startedWaves.size(), 2u);
}

TEST_F(WaveDirectorTest, WhenTheLastWaveIsDownTheDirectorReportsItOnce)
{
	director->SetWaves({{0.f, {{TileType::MarauderSpawn, 1}}}});

	Run(0.3f);
	KillAll();
	Run(1.f);

	EXPECT_EQ(clears, 1);
	EXPECT_TRUE(director->IsCleared());

	Run(2.f);

	EXPECT_EQ(clears, 1) << "reported more than once";
}

TEST_F(WaveDirectorTest, PointsAreTakenInTurn)
{
	director->SetWaves({{0.f, {{TileType::MarauderSpawn, 4}}}});

	Run(0.3f);

	ASSERT_EQ(places.size(), 4u);
	for (std::size_t first = 0u; first < places.size(); first++)
	{
		for (std::size_t second = first + 1u; second < places.size(); second++)
		{
			EXPECT_GT((places[first] - places[second]).GetLength(), 1.f) << "two enemies share a point";
		}
	}
}

TEST_F(WaveDirectorTest, NobodyIsBornUnderTheHeroesFeet)
{
	GameObject* hero = GameWorld::Instance()->CreateGameObject("Hero");
	hero->GetTransform()->SetWorldPosition({0.f, 0.f});
	director->SetHero(hero);

	director->SetWaves({{0.f, {{TileType::MarauderSpawn, 1}}}});

	Run(0.3f);

	ASSERT_EQ(places.size(), 1u);
	EXPECT_GE(places[0].GetLength(), RoguelikeGame::WAVE_SPAWN_GAP);
}

TEST_F(WaveDirectorTest, WithoutWavesTheDirectorSitsStill)
{
	Run(2.f);

	EXPECT_TRUE(born.empty());
	EXPECT_EQ(clears, 0);
	EXPECT_FALSE(director->IsCleared());
}

namespace
{
	constexpr float NEAR_POINT = RoguelikeGame::WAVE_SPAWN_GAP + 64.f;
	constexpr float FAR_POINT = RoguelikeGame::WAVE_SPAWN_REACH * 4.f;
}

TEST_F(WaveDirectorTest, AWaveComesOutBesideTheHeroAndNotAcrossTheMap)
{
	GameObject* hero = GameWorld::Instance()->CreateGameObject("Hero");
	hero->GetTransform()->SetWorldPosition({0.f, 0.f});
	director->SetHero(hero);

	// Половина точек рядом, половина на другом конце моста.
	director->SetPoints({{FAR_POINT, 0.f}, {NEAR_POINT, 0.f}, {FAR_POINT * 2.f, 0.f}, {0.f, NEAR_POINT}});
	director->SetWaves({{0.f, {{TileType::MarauderSpawn, 2}}}});

	Run(0.3f);

	ASSERT_EQ(places.size(), 2u);
	for (const Vector2Df& place : places)
	{
		EXPECT_LE(place.GetLength(), RoguelikeGame::WAVE_SPAWN_REACH) << "the wave spawned where the hero is not";
		EXPECT_GE(place.GetLength(), RoguelikeGame::WAVE_SPAWN_GAP) << "the wave spawned on top of the hero";
	}
}

TEST_F(WaveDirectorTest, WithNothingNearbyTheWaveTakesTheClosestPointItCanFind)
{
	GameObject* hero = GameWorld::Instance()->CreateGameObject("Hero");
	hero->GetTransform()->SetWorldPosition({0.f, 0.f});
	director->SetHero(hero);

	director->SetPoints({{FAR_POINT * 3.f, 0.f}, {FAR_POINT, 0.f}});
	director->SetWaves({{0.f, {{TileType::MarauderSpawn, 1}}}});

	Run(0.3f);

	ASSERT_EQ(places.size(), 1u);
	EXPECT_FLOAT_EQ(places[0].x, FAR_POINT) << "the wave went to the far end when a nearer point existed";
}

TEST_F(WaveDirectorTest, AStragglerLeftBehindDoesNotHoldTheNextWave)
{
	GameObject* hero = GameWorld::Instance()->CreateGameObject("Hero");
	hero->GetTransform()->SetWorldPosition({0.f, 0.f});
	director->SetHero(hero);
	director->SetPoints({{NEAR_POINT, 0.f}});

	director->SetWaves({{0.f, {{TileType::MarauderSpawn, 1}}}, {0.f, {{TileType::AssaultSpawn, 1}}}});

	Run(0.3f);
	ASSERT_EQ(born.size(), 1u) << "the first wave never came";

	// Игрок уходит дальше по мосту, живой враг остаётся позади.
	hero->GetTransform()->SetWorldPosition({RoguelikeGame::WAVE_ADVANCE_STEP + 640.f, 0.f});
	Run(0.5f);

	EXPECT_EQ(born.size(), 2u) << "one enemy left behind froze the chase forever";
}

TEST_F(WaveDirectorTest, StandingStillTheHeroMustActuallyFightTheWaveOff)
{
	GameObject* hero = GameWorld::Instance()->CreateGameObject("Hero");
	hero->GetTransform()->SetWorldPosition({0.f, 0.f});
	director->SetHero(hero);
	director->SetPoints({{NEAR_POINT, 0.f}});

	director->SetWaves({{0.f, {{TileType::MarauderSpawn, 1}}}, {0.f, {{TileType::AssaultSpawn, 1}}}});

	Run(0.3f);
	ASSERT_EQ(born.size(), 1u);

	Run(2.f);

	EXPECT_EQ(born.size(), 1u) << "the next wave came while the previous one was still standing";

	KillAll();
	Run(0.5f);

	EXPECT_EQ(born.size(), 2u);
}

TEST_F(WaveDirectorTest, TheLastWaveIsNotOverWhileItBreathesDownTheNeck)
{
	GameObject* hero = GameWorld::Instance()->CreateGameObject("Hero");
	hero->GetTransform()->SetWorldPosition({0.f, 0.f});
	director->SetHero(hero);
	director->SetPoints({{NEAR_POINT, 0.f}});

	director->SetWaves({{0.f, {{TileType::MarauderSpawn, 1}}}});

	Run(0.3f);
	ASSERT_EQ(born.size(), 1u);

	// Игрок отбежал, но враг бежит следом - машина ещё не ждёт.
	hero->GetTransform()->SetWorldPosition({RoguelikeGame::WAVE_ADVANCE_STEP + 100.f, 0.f});
	born[0]->GetTransform()->SetWorldPosition({RoguelikeGame::WAVE_ADVANCE_STEP, 0.f});
	Run(0.5f);

	EXPECT_EQ(clears, 0) << "the escape opened with the chase still on the hero";

	KillAll();
	Run(0.5f);

	EXPECT_EQ(clears, 1);
	EXPECT_TRUE(director->IsCleared());
}

TEST_F(WaveDirectorTest, RunningAwayIsNotReportedAsAVictory)
{
	std::vector<int> beaten;
	director->SubscribeWaveCleared([&beaten](int current, int) { beaten.push_back(current); });

	GameObject* hero = GameWorld::Instance()->CreateGameObject("Hero");
	hero->GetTransform()->SetWorldPosition({0.f, 0.f});
	director->SetHero(hero);
	director->SetPoints({{NEAR_POINT, 0.f}});

	director->SetWaves({{0.f, {{TileType::MarauderSpawn, 1}}}, {0.f, {{TileType::AssaultSpawn, 1}}}});

	Run(0.3f);
	ASSERT_EQ(born.size(), 1u);

	hero->GetTransform()->SetWorldPosition({RoguelikeGame::WAVE_ADVANCE_STEP + 640.f, 0.f});
	Run(0.5f);

	ASSERT_EQ(born.size(), 2u) << "the chase did not move on";
	EXPECT_TRUE(beaten.empty()) << "the hero was congratulated for running away";
}

TEST_F(WaveDirectorTest, AWaveFoughtOffIsReportedOnce)
{
	std::vector<int> beaten;
	director->SubscribeWaveCleared([&beaten](int current, int) { beaten.push_back(current); });

	GameObject* hero = GameWorld::Instance()->CreateGameObject("Hero");
	hero->GetTransform()->SetWorldPosition({0.f, 0.f});
	director->SetHero(hero);
	director->SetPoints({{NEAR_POINT, 0.f}});

	director->SetWaves({{0.f, {{TileType::MarauderSpawn, 1}}}, {5.f, {{TileType::AssaultSpawn, 1}}}});

	Run(0.3f);
	ASSERT_EQ(born.size(), 1u);

	KillAll();
	Run(0.5f);

	ASSERT_EQ(beaten.size(), 1u);
	EXPECT_EQ(beaten[0], 1);
}

TEST_F(WaveDirectorTest, OnlyWhatIsStillOnTheHeroCountsAsAlive)
{
	GameObject* hero = GameWorld::Instance()->CreateGameObject("Hero");
	hero->GetTransform()->SetWorldPosition({0.f, 0.f});
	director->SetHero(hero);
	director->SetPoints({{NEAR_POINT, 0.f}});

	director->SetWaves({{0.f, {{TileType::MarauderSpawn, 1}}}});

	Run(0.3f);
	ASSERT_EQ(born.size(), 1u);

	EXPECT_EQ(director->CountAlive(), 1);
	EXPECT_EQ(director->CountAliveNearby(), 1);

	born[0]->GetTransform()->SetWorldPosition({RoguelikeGame::WAVE_KEEP_RANGE * 2.f, 0.f});

	EXPECT_EQ(director->CountAlive(), 1) << "the enemy is still alive, just far away";
	EXPECT_EQ(director->CountAliveNearby(), 0);
}

namespace
{
	class ShippedWavesTest : public ProjectFiles::Test
	{
	};
}

TEST_F(ShippedWavesTest, TheBridgeRunsOnAChaseAndNotOnWaves)
{
	ASSERT_TRUE(isFound) << previous.string();

	ActPlan plan = ActLoader::Load("Resources/Acts/act1_bridge.config");

	// Волны заставляют останавливаться и зачищать - беговой локации это противопоказано.
	EXPECT_TRUE(plan.waves.empty()) << "the bridge still has waves to stop and clear";
	ASSERT_FALSE(plan.pursuit.IsEmpty()) << "nobody chases the player across the bridge";

	EXPECT_GT(plan.pursuit.keep, 0);
	EXPECT_GT(plan.pursuit.grow, plan.pursuit.keep) << "standing still costs the player nothing";
	EXPECT_GT(plan.pursuit.respawn, 0.f) << "the dead are replaced within the same frame";
}

TEST_F(ShippedWavesTest, TheChaseIsCarriedThroughTheAssembledAct)
{
	ASSERT_TRUE(isFound) << previous.string();

	ActPlan plan = ActLoader::Load("Resources/Acts/act1_bridge.config");
	LevelData bridge = RoguelikeGame::LoadAct("Resources/Acts/act1_bridge.config");

	ASSERT_FALSE(bridge.pursuit.IsEmpty()) << "the pursuit was lost while assembling the act";
	EXPECT_EQ(bridge.pursuit.echelons.size(), plan.pursuit.echelons.size());
	EXPECT_EQ(bridge.pursuit.keep, plan.pursuit.keep);

	int points = 0;
	for (const auto& row : bridge.tiles)
	{
		for (TileType tile : row)
		{
			points += tile == TileType::WaveSpawn ? 1 : 0;
		}
	}

	EXPECT_GT(points, 0) << "the chase has nowhere to come from";
}

TEST_F(ShippedWavesTest, TheChaseCoversTheWholeBridgeFromTheStart)
{
	ASSERT_TRUE(isFound) << previous.string();

	ActPlan plan = ActLoader::Load("Resources/Acts/act1_bridge.config");
	ASSERT_GE(plan.pursuit.echelons.size(), 2u);

	// Первый эшелон обязан начинаться с нуля, иначе в начале моста гнаться некому.
	EXPECT_FLOAT_EQ(plan.pursuit.echelons.front().fromPart, 0.f);
	EXPECT_LE(plan.pursuit.echelons.back().fromPart, 0.9f) << "the last stretch is left without its own company";

	for (const RoguelikeGame::PursuitEchelon& echelon : plan.pursuit.echelons)
	{
		EXPECT_FALSE(echelon.entries.empty()) << "an echelon with nobody in it";
		EXPECT_GT(echelon.TotalWeight(), 0);
	}
}

TEST_F(ShippedWavesTest, TheChaseGetsHeavierTowardsTheCar)
{
	ASSERT_TRUE(isFound) << previous.string();

	ActPlan plan = ActLoader::Load("Resources/Acts/act1_bridge.config");
	ASSERT_GE(plan.pursuit.echelons.size(), 2u);

	auto Toughness = [](const RoguelikeGame::PursuitEchelon& echelon)
	{
		float total = 0.f;
		for (const RoguelikeGame::PursuitEntry& entry : echelon.entries)
		{
			const RoguelikeGame::EnemyConfig* config = RoguelikeGame::FindEnemyConfig(entry.enemy);
			total += config == nullptr ? 0.f : (config->maxHealth + config->armor) * entry.weight;
		}

		return total / static_cast<float>(echelon.TotalWeight());
	};

	EXPECT_GT(Toughness(plan.pursuit.echelons.back()), Toughness(plan.pursuit.echelons.front()))
		<< "the chase near the car is no tougher than at the prison gates";
}

TEST_F(ShippedWavesTest, SpawnPointsAreSpreadAlongTheWholeBridge)
{
	ASSERT_TRUE(isFound) << previous.string();

	LevelData bridge = RoguelikeGame::LoadAct("Resources/Acts/act1_bridge.config");

	int leftmost = bridge.width;
	int rightmost = 0;
	for (int row = 0; row < bridge.height; row++)
	{
		for (int column = 0; column < static_cast<int>(bridge.tiles[row].size()); column++)
		{
			if (bridge.tiles[row][column] != TileType::WaveSpawn)
			{
				continue;
			}

			leftmost = column < leftmost ? column : leftmost;
			rightmost = column > rightmost ? column : rightmost;
		}
	}

	ASSERT_LT(leftmost, rightmost) << "the bridge has no spawn points at all";

	// Между крайними точками не должно быть половины моста без единого места для волны.
	EXPECT_LE(leftmost * RoguelikeGame::TILE_SIZE, RoguelikeGame::WAVE_ADVANCE_STEP) << "the first stretch has nowhere to spawn";
	EXPECT_GE(rightmost, bridge.width - static_cast<int>(RoguelikeGame::WAVE_ADVANCE_STEP / RoguelikeGame::TILE_SIZE))
		<< "the last stretch has nowhere to spawn";
}

namespace
{
	class EscapeCarGateTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			GameObject* body = GameWorld::Instance()->CreateGameObject("EscapeCar");
			car = body->AddComponent<RoguelikeGame::EscapeCarComponent>();

			boarded = 0;
			car->SubscribeBoarded([this]() { boarded++; });

			GameWorld::Instance()->Update(0.016f);
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		RoguelikeGame::EscapeCarComponent* car = nullptr;
		int boarded = 0;
	};
}

TEST_F(EscapeCarGateTest, ACarThatIsNotReadyRefusesToTakeThePlayer)
{
	car->SetReady(false);

	EXPECT_FALSE(car->IsAvailable());
	EXPECT_FALSE(car->Interact(nullptr));
	EXPECT_EQ(boarded, 0);
}

TEST_F(EscapeCarGateTest, OnceReadyTheCarTakesThePlayerOnce)
{
	car->SetReady(true);

	EXPECT_TRUE(car->Interact(nullptr));
	EXPECT_EQ(boarded, 1);

	EXPECT_FALSE(car->Interact(nullptr)) << "boarded twice";
	EXPECT_EQ(boarded, 1);
}
