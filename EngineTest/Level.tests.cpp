#include "pch.h"
#include "GameWorld.h"
#include "Level.h"

using RoguelikeGame::Level;
using RoguelikeGame::LevelInfo;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::Vector2Df;

namespace
{
	class LevelTest : public ::testing::Test
	{
	protected:
		void SetUp() override { GameWorld::Instance()->Clear(); }
		void TearDown() override { GameWorld::Instance()->Clear(); }
	};
}

TEST_F(LevelTest, EmptyLevelHasNoStartPoints)
{
	Level level;

	EXPECT_FALSE(level.GetPlayerSpawn().has_value());
	EXPECT_FALSE(level.GetEntrance().has_value());
	EXPECT_EQ(level.GetExit(), nullptr);
	EXPECT_EQ(level.GetObjectsCount(), 0u);
}

TEST_F(LevelTest, StartPositionFallsBackToPlayerSpawn)
{
	Level level;
	level.SetPlayerSpawn({100.f, 200.f});

	EXPECT_FLOAT_EQ(level.GetStartPosition().x, 100.f);
	EXPECT_FLOAT_EQ(level.GetStartPosition().y, 200.f);
}

TEST_F(LevelTest, EntranceWinsOverPlayerSpawn)
{
	Level level;
	level.SetPlayerSpawn({100.f, 200.f});
	level.SetEntrance({300.f, 400.f});

	EXPECT_FLOAT_EQ(level.GetStartPosition().x, 300.f);
	EXPECT_FLOAT_EQ(level.GetStartPosition().y, 400.f);
}

TEST_F(LevelTest, InfoIsStoredAsIs)
{
	Level level;
	LevelInfo info;
	info.title = "Gorod";
	info.nextLevelId = "arena";
	info.boss.bossId = "Boss";
	info.boss.healthScale = 2.f;

	level.SetInfo(info);

	EXPECT_EQ(level.GetInfo().title, "Gorod");
	EXPECT_EQ(level.GetInfo().nextLevelId, "arena");
	EXPECT_EQ(level.GetInfo().boss.bossId, "Boss");
	EXPECT_FLOAT_EQ(level.GetInfo().boss.healthScale, 2.f);
	EXPECT_FALSE(level.GetInfo().boss.IsEmpty());
}

TEST_F(LevelTest, ClearForgetsEverythingAboutTheLevel)
{
	Level level;
	GameObject* wall = GameWorld::Instance()->CreateGameObject("Wall");
	GameObject* exitObject = GameWorld::Instance()->CreateGameObject("LevelExit");

	level.Add(wall);
	level.Add(exitObject);
	level.SetExit(exitObject);
	level.SetPlayerSpawn({10.f, 10.f});
	level.SetEntrance({20.f, 20.f});

	LevelInfo info;
	info.nextLevelId = "arena";
	level.SetInfo(info);

	level.Clear();
	GameWorld::Instance()->LateUpdate();

	EXPECT_EQ(level.GetObjectsCount(), 0u);
	EXPECT_EQ(level.GetExit(), nullptr);
	EXPECT_FALSE(level.GetEntrance().has_value());
	EXPECT_TRUE(level.GetInfo().nextLevelId.empty());
	EXPECT_EQ(GameWorld::Instance()->GetObjectsCount(), 0u);
}

TEST_F(LevelTest, ClearedLevelLeavesNoObjectsInTheWorld)
{
	Level level;
	for (int index = 0; index < 20; index++)
	{
		level.Add(GameWorld::Instance()->CreateGameObject("Wall"));
	}

	GameObject* survivor = GameWorld::Instance()->CreateGameObject("Player");

	level.Clear();
	GameWorld::Instance()->LateUpdate();

	EXPECT_EQ(GameWorld::Instance()->GetObjectsCount(), 1u);
	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Player"), survivor);
}

TEST_F(LevelTest, MovedLevelKeepsItsData)
{
	Level source;
	source.Add(GameWorld::Instance()->CreateGameObject("Wall"));
	source.SetEntrance({5.f, 6.f});

	LevelInfo info;
	info.title = "Arena";
	source.SetInfo(info);

	Level moved = std::move(source);

	EXPECT_EQ(moved.GetObjectsCount(), 1u);
	EXPECT_TRUE(moved.GetEntrance().has_value());
	EXPECT_EQ(moved.GetInfo().title, "Arena");
	EXPECT_EQ(source.GetObjectsCount(), 0u);
	EXPECT_FALSE(source.GetEntrance().has_value());
}

TEST_F(LevelTest, RepeatedTransitionsLeaveNoObjectsBehind)
{
	GameObject* player = GameWorld::Instance()->CreateGameObject("Player");

	for (int transition = 0; transition < 3; transition++)
	{
		Level level;
		for (int index = 0; index < 50; index++)
		{
			level.Add(GameWorld::Instance()->CreateGameObject("Wall"));
		}

		EXPECT_EQ(GameWorld::Instance()->GetObjectsCount(), 51u);

		level.Clear();
		GameWorld::Instance()->LateUpdate();

		EXPECT_EQ(GameWorld::Instance()->GetObjectsCount(), 1u);
	}

	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Player"), player);
}

TEST_F(LevelTest, BossIsRememberedAndForgottenWithTheLevel)
{
	Level level;
	GameObject* boss = GameWorld::Instance()->CreateGameObject("Boss");
	level.Add(boss);
	level.SetBoss(boss);

	EXPECT_EQ(level.GetBoss(), boss);

	level.Clear();
	GameWorld::Instance()->LateUpdate();

	EXPECT_EQ(level.GetBoss(), nullptr);
}

TEST_F(LevelTest, MovedLevelKeepsItsBoss)
{
	Level source;
	GameObject* boss = GameWorld::Instance()->CreateGameObject("Boss");
	source.Add(boss);
	source.SetBoss(boss);

	Level moved = std::move(source);

	EXPECT_EQ(moved.GetBoss(), boss);
	EXPECT_EQ(source.GetBoss(), nullptr);
}

TEST_F(LevelTest, MoveAssignedLevelKeepsItsBoss)
{
	Level source;
	GameObject* boss = GameWorld::Instance()->CreateGameObject("Boss");
	source.Add(boss);
	source.SetBoss(boss);

	Level target;
	target.Add(GameWorld::Instance()->CreateGameObject("Wall"));
	target.SetBoss(GameWorld::Instance()->CreateGameObject("OldBoss"));

	target = std::move(source);

	EXPECT_EQ(target.GetBoss(), boss);
	EXPECT_EQ(source.GetBoss(), nullptr);
}

namespace
{
	using RoguelikeGame::LevelRole;

	constexpr std::size_t ROLES = static_cast<std::size_t>(LevelRole::Count);

	LevelRole RoleAt(std::size_t index)
	{
		return static_cast<LevelRole>(index);
	}

	std::string RoleName(std::size_t index)
	{
		return "role " + std::to_string(index) + " of " + std::to_string(ROLES);
	}

	// Каждой роли свой объект: так видно, если перенос перепутает их местами.
	void FillEveryRole(Level& level)
	{
		for (std::size_t index = 0u; index < ROLES; index++)
		{
			GameObject* marker = GameWorld::Instance()->CreateGameObject("Role" + std::to_string(index));
			level.Add(marker);
			level.Set(RoleAt(index), marker);
		}
	}
}

TEST_F(LevelTest, EveryRoleSurvivesTheMove)
{
	Level source;
	FillEveryRole(source);

	std::vector<GameObject*> before;
	for (std::size_t index = 0u; index < ROLES; index++)
	{
		before.push_back(source.Get(RoleAt(index)));
	}

	Level moved = std::move(source);

	for (std::size_t index = 0u; index < ROLES; index++)
	{
		EXPECT_EQ(moved.Get(RoleAt(index)), before[index]) << RoleName(index) << " was lost on the way";
		EXPECT_EQ(source.Get(RoleAt(index)), nullptr) << RoleName(index) << " stayed with the old level";
	}
}

TEST_F(LevelTest, EveryRoleSurvivesTheMoveAssignment)
{
	Level source;
	FillEveryRole(source);

	std::vector<GameObject*> before;
	for (std::size_t index = 0u; index < ROLES; index++)
	{
		before.push_back(source.Get(RoleAt(index)));
	}

	Level target;
	FillEveryRole(target);

	target = std::move(source);

	for (std::size_t index = 0u; index < ROLES; index++)
	{
		EXPECT_EQ(target.Get(RoleAt(index)), before[index]) << RoleName(index) << " was lost on the way";
		EXPECT_EQ(source.Get(RoleAt(index)), nullptr) << RoleName(index) << " stayed with the old level";
	}
}

TEST_F(LevelTest, ClearForgetsEveryRoleNotJustSome)
{
	Level level;
	FillEveryRole(level);

	level.Clear();

	for (std::size_t index = 0u; index < ROLES; index++)
	{
		EXPECT_EQ(level.Get(RoleAt(index)), nullptr) << RoleName(index) << " outlived the level";
	}
}

TEST_F(LevelTest, TheNamedGettersReadTheSameRoles)
{
	Level level;

	GameObject* exitObject = GameWorld::Instance()->CreateGameObject("Exit");
	GameObject* boss = GameWorld::Instance()->CreateGameObject("Boss");
	GameObject* director = GameWorld::Instance()->CreateGameObject("WaveDirector");
	GameObject* car = GameWorld::Instance()->CreateGameObject("EscapeCar");

	level.SetExit(exitObject);
	level.SetBoss(boss);
	level.SetWaveDirector(director);
	level.SetEscapeCar(car);

	EXPECT_EQ(level.Get(LevelRole::Exit), exitObject);
	EXPECT_EQ(level.Get(LevelRole::Boss), boss);
	EXPECT_EQ(level.Get(LevelRole::WaveDirector), director);
	EXPECT_EQ(level.Get(LevelRole::EscapeCar), car);
}

TEST_F(LevelTest, TheWaveDirectorAndTheCarComeAlongWithTheLevel)
{
	Level source;
	GameObject* director = GameWorld::Instance()->CreateGameObject("WaveDirector");
	GameObject* car = GameWorld::Instance()->CreateGameObject("EscapeCar");
	source.Add(director);
	source.Add(car);
	source.SetWaveDirector(director);
	source.SetEscapeCar(car);

	Level target;
	target = std::move(source);

	EXPECT_EQ(target.GetWaveDirector(), director) << "waves lose the hero and the hud on every level change";
	EXPECT_EQ(target.GetEscapeCar(), car) << "the escape car can never be unlocked";
}

TEST_F(LevelTest, AskingForNoRoleAtAllIsHarmless)
{
	Level level;

	level.Set(LevelRole::Count, GameWorld::Instance()->CreateGameObject("Nobody"));

	EXPECT_EQ(level.Get(LevelRole::Count), nullptr);
}
