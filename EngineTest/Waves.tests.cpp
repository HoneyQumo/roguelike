#include "pch.h"
#include "ActAssembler.h"
#include "ActPlan.h"
#include "GameSettings.h"
#include "GameWorld.h"
#include "HealthComponent.h"
#include "LevelLoader.h"
#include "ProjectFiles.h"
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
	class ShippedWavesTest : public ProjectFiles::Test
	{
	};
}

TEST_F(ShippedWavesTest, TheBridgeHasWavesAndSomewhereToPutThem)
{
	ASSERT_TRUE(isFound) << previous.string();

	ActPlan plan = ActLoader::Load("Resources/Acts/act1_bridge.config");

	ASSERT_FALSE(plan.waves.empty()) << "the bridge has no waves";

	for (const WaveSpec& wave : plan.waves)
	{
		EXPECT_GT(wave.Size(), 0) << "an empty wave";
		EXPECT_GT(wave.delay, 0.f) << "a wave with no breather before it";
	}

	LevelData bridge = RoguelikeGame::LoadAct("Resources/Acts/act1_bridge.config");
	int points = 0;
	for (const auto& row : bridge.tiles)
	{
		for (TileType tile : row)
		{
			points += tile == TileType::WaveSpawn ? 1 : 0;
		}
	}

	EXPECT_GT(points, 0) << "waves have nowhere to spawn";
	EXPECT_EQ(bridge.waves.size(), plan.waves.size()) << "waves were lost while assembling the act";
}
