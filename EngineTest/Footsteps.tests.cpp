#include "pch.h"
#include "Footsteps.h"
#include "ProjectFiles.h"
#include <filesystem>

using RoguelikeGame::FOE_STEPS;
using RoguelikeGame::HERO_STEPS;
using RoguelikeGame::STEP_VARIANTS;
using RoguelikeGame::StepFilePath;
using RoguelikeGame::StepKey;
using RoguelikeGame::IsStepFrame;
using RoguelikeGame::LoseStep;
using RoguelikeGame::StepBeat;
using RoguelikeGame::TakeStep;

TEST(FootstepsTest, EveryVariantHasItsOwnKey)
{
	EXPECT_EQ(StepKey(HERO_STEPS, 1), "step_hero_1");
	EXPECT_EQ(StepKey(FOE_STEPS, STEP_VARIANTS), "step_boot_" + std::to_string(STEP_VARIANTS));
	EXPECT_NE(StepKey(HERO_STEPS, 1), StepKey(HERO_STEPS, 2));
}

// Свои шаги и чужие - разные файлы, иначе на слух их не разделить.
TEST(FootstepsTest, TheHeroAndTheFoeStepOnDifferentFiles)
{
	for (int variant = 1; variant <= STEP_VARIANTS; variant++)
	{
		EXPECT_NE(StepFilePath(HERO_STEPS, variant), StepFilePath(FOE_STEPS, variant));
	}
}

TEST(FootstepsTest, ThereIsNoVariantOutsideTheSet)
{
	EXPECT_TRUE(StepKey(HERO_STEPS, 0).empty());
	EXPECT_TRUE(StepKey(HERO_STEPS, STEP_VARIANTS + 1).empty());
	EXPECT_TRUE(StepFilePath(HERO_STEPS, -1).empty());
	EXPECT_TRUE(StepFilePath(nullptr, 1).empty());
}

TEST(FootstepsTest, TheKeyGoesIntoTheFileName)
{
	EXPECT_EQ(StepFilePath(FOE_STEPS, 2), "Resources/Audio/Steps/step_boot_2.wav");
}

// Восемь кадров ходьбы - это два шага, а не восемь: нога касается земли
// в начале каждой половины цикла.
TEST(FootstepsTest, TheFootTouchesTheGroundTwicePerCycle)
{
	int touches = 0;
	for (int frame = 0; frame < 8; frame++)
	{
		touches += IsStepFrame(frame, 8) ? 1 : 0;
	}

	EXPECT_EQ(touches, 2);
	EXPECT_TRUE(IsStepFrame(0, 8));
	EXPECT_TRUE(IsStepFrame(4, 8));
}

TEST(FootstepsTest, AFrameOutsideTheClipIsNoStep)
{
	EXPECT_FALSE(IsStepFrame(-1, 8));
	EXPECT_FALSE(IsStepFrame(8, 8));
	EXPECT_FALSE(IsStepFrame(0, 1));
}

// Кадр держится несколько тиков подряд: без памяти о прошлом кадре один шаг
// звучал бы очередью.
TEST(FootstepsTest, TheSameFrameStepsOnlyOnce)
{
	StepBeat beat;

	EXPECT_TRUE(TakeStep(beat, 0, 8));
	EXPECT_FALSE(TakeStep(beat, 0, 8));
	EXPECT_FALSE(TakeStep(beat, 0, 8));
}

TEST(FootstepsTest, TheWholeCycleGivesTwoSteps)
{
	StepBeat beat;

	int steps = 0;
	for (int round = 0; round < 2; round++)
	{
		for (int frame = 0; frame < 8; frame++)
		{
			steps += TakeStep(beat, frame, 8) ? 1 : 0;
		}
	}

	EXPECT_EQ(steps, 4);
}

// Остановился и пошёл заново - шаг обязан прозвучать, даже если кадр тот же.
TEST(FootstepsTest, AfterAStopTheFirstFrameStepsAgain)
{
	StepBeat beat;

	ASSERT_TRUE(TakeStep(beat, 0, 8));
	LoseStep(beat);

	EXPECT_TRUE(TakeStep(beat, 0, 8));
}

class ShippedStepsTest : public ProjectFiles::Test
{
};

// Вариантов должно быть столько, сколько обещает код: пропажа одного файла
// иначе всплывёт тишиной на каждом четвёртом шаге.
TEST_F(ShippedStepsTest, EveryVariantLiesOnDisk)
{
	ASSERT_TRUE(isFound) << previous.string();

	for (const char* set : {HERO_STEPS, FOE_STEPS})
	{
		for (int variant = 1; variant <= STEP_VARIANTS; variant++)
		{
			EXPECT_TRUE(std::filesystem::exists(StepFilePath(set, variant))) << StepFilePath(set, variant);
		}
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
		for (const char* set : {HERO_STEPS, FOE_STEPS})
		{
			for (int variant = 1; variant <= STEP_VARIANTS && !isKnown; variant++)
			{
				isKnown = path == StepFilePath(set, variant);
			}
		}

		EXPECT_TRUE(isKnown) << path << " не упоминается в коде - подключить или удалить";
		found++;
	}

	EXPECT_EQ(found, 2 * STEP_VARIANTS);
}
