#include "pch.h"
#include "LevelBuilder.h"
#include "LevelExitComponent.h"
#include "LevelLoader.h"
#include "ProjectFiles.h"
#include <GameWorld.h>
#include <sstream>

using RoguelikeGame::Level;
using RoguelikeGame::LevelBuilder;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelExitComponent;
using RoguelikeGame::LevelLoader;

namespace
{
	const std::string MAP =
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"> Exit\n"
		"w WaveSpawn\n"
		"g GruntSpawn\n"
		"\n"
		"[map]\n"
		"#####\n"
		"#@.w#\n"
		"#..>#\n"
		"#####\n";

	const std::string WAVES =
		"[waves]\n"
		"wave 3 g2\n"
		"\n";

	class LevelBuilderTest : public ProjectFiles::Test
	{
	protected:
		void SetUp() override
		{
			ProjectFiles::Test::SetUp();
			XYZEngine::GameWorld::Instance()->Clear();
		}

		void TearDown() override
		{
			XYZEngine::GameWorld::Instance()->Clear();
			ProjectFiles::Test::TearDown();
		}

		static Level BuildFrom(const std::string& text)
		{
			std::istringstream input(text);

			return LevelBuilder::Build(LevelLoader::Parse(input, "builder"));
		}

		static LevelExitComponent* ExitOf(const Level& level)
		{
			return level.GetExit() != nullptr
				? level.GetExit()->GetComponent<LevelExitComponent>()
				: nullptr;
		}
	};
}

// Волны и босс рождаются в разных местах сборки, и запирать выход надо после
// обоих: пока проверка стояла раньше волн, карта с [waves] проходилась мимо них.
TEST_F(LevelBuilderTest, WavesLockTheExit)
{
	Level level = BuildFrom(WAVES + MAP);

	ASSERT_NE(level.GetWaveDirector(), nullptr);
	ASSERT_NE(ExitOf(level), nullptr);
	EXPECT_TRUE(ExitOf(level)->IsLocked());
}

TEST_F(LevelBuilderTest, WithoutAGuardTheExitIsOpen)
{
	Level level = BuildFrom(MAP);

	ASSERT_EQ(level.GetWaveDirector(), nullptr);
	ASSERT_NE(ExitOf(level), nullptr);
	EXPECT_FALSE(ExitOf(level)->IsLocked());
}
