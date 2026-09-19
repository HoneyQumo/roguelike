#include "pch.h"
#include "Footsteps.h"
#include "ProjectFiles.h"
#include <filesystem>

using RoguelikeGame::STEP_VARIANTS;
using RoguelikeGame::StepFilePath;
using RoguelikeGame::StepKey;

TEST(FootstepsTest, EveryVariantHasItsOwnKey)
{
	EXPECT_EQ(StepKey(1), "step_1");
	EXPECT_EQ(StepKey(STEP_VARIANTS), "step_" + std::to_string(STEP_VARIANTS));
	EXPECT_NE(StepKey(1), StepKey(2));
}

TEST(FootstepsTest, ThereIsNoVariantOutsideTheSet)
{
	EXPECT_TRUE(StepKey(0).empty());
	EXPECT_TRUE(StepKey(STEP_VARIANTS + 1).empty());
	EXPECT_TRUE(StepFilePath(-1).empty());
}

TEST(FootstepsTest, TheKeyGoesIntoTheFileName)
{
	EXPECT_EQ(StepFilePath(2), "Resources/Audio/Steps/step_2.wav");
}

class ShippedStepsTest : public ProjectFiles::Test
{
};

// Вариантов должно быть столько, сколько обещает код: пропажа одного файла
// иначе всплывёт тишиной на каждом четвёртом шаге.
TEST_F(ShippedStepsTest, EveryVariantLiesOnDisk)
{
	ASSERT_TRUE(isFound) << previous.string();

	for (int variant = 1; variant <= STEP_VARIANTS; variant++)
	{
		EXPECT_TRUE(std::filesystem::exists(StepFilePath(variant))) << StepFilePath(variant);
	}
}

TEST_F(ShippedStepsTest, NothingElseLiesAmongTheSteps)
{
	ASSERT_TRUE(isFound) << previous.string();

	int found = 0;
	for (const auto& entry : std::filesystem::directory_iterator(RoguelikeGame::STEP_AUDIO_PATH))
	{
		std::string path = RoguelikeGame::STEP_AUDIO_PATH + entry.path().filename().string();

		bool isKnown = false;
		for (int variant = 1; variant <= STEP_VARIANTS && !isKnown; variant++)
		{
			isKnown = path == StepFilePath(variant);
		}

		EXPECT_TRUE(isKnown) << path << " не упоминается в коде - подключить или удалить";
		found++;
	}

	EXPECT_EQ(found, STEP_VARIANTS);
}
