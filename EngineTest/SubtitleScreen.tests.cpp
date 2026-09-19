#include "pch.h"
#include "GameSettings.h"
#include "ProjectFiles.h"
#include "SubtitleScreen.h"
#include <ResourceSystem.h>

using RoguelikeGame::SubtitleQueue;
using RoguelikeGame::SubtitleScreen;

class SubtitleScreenTest : public ProjectFiles::Test
{
protected:
	void SetUp() override
	{
		ProjectFiles::Test::SetUp();
		if (isFound)
		{
			XYZEngine::ResourceSystem::Instance()->LoadFont(RoguelikeGame::HUD_FONT, RoguelikeGame::HUD_FONT_FILE);
		}
	}
};

TEST_F(SubtitleScreenTest, AnEmptyQueueShowsNothing)
{
	ASSERT_TRUE(isFound) << previous.string();

	SubtitleQueue queue;
	SubtitleScreen screen;
	screen.SetQueue(&queue);

	screen.Update(0.016f);

	EXPECT_EQ(screen.GetShownLineCount(), 0);
}

TEST_F(SubtitleScreenTest, WhatWasSaidReachesTheScreen)
{
	ASSERT_TRUE(isFound) << previous.string();

	SubtitleQueue queue;
	SubtitleScreen screen;
	screen.SetQueue(&queue);

	queue.Say("Охранник", "Вот ты где!", 3.f);
	screen.Update(0.016f);

	EXPECT_EQ(screen.GetShownLineCount(), 1);
}

// Длинная реплика не уходит за край: экран режет её по ширине, и строк
// на экране становится больше, чем реплик в очереди.
TEST_F(SubtitleScreenTest, ALongLineIsBrokenIntoSeveral)
{
	ASSERT_TRUE(isFound) << previous.string();

	SubtitleQueue queue;
	SubtitleScreen screen;
	screen.SetQueue(&queue);

	queue.Say("Радист",
		"Чужой на базе, поднимайте всех по тревоге и перекройте нижний коридор вместе со складом", 9.f);
	screen.Update(0.016f);

	EXPECT_GT(screen.GetShownLineCount(), 1);
}

TEST_F(SubtitleScreenTest, WhenTheLineFadesTheScreenClears)
{
	ASSERT_TRUE(isFound) << previous.string();

	SubtitleQueue queue;
	SubtitleScreen screen;
	screen.SetQueue(&queue);

	queue.Say("Охранник", "Вот ты где!", 1.f);
	screen.Update(0.016f);
	ASSERT_GT(screen.GetShownLineCount(), 0);

	queue.Update(2.f);
	screen.Update(0.016f);

	EXPECT_EQ(screen.GetShownLineCount(), 0);
}

TEST_F(SubtitleScreenTest, AScreenWithoutAQueueIsNotACrash)
{
	ASSERT_TRUE(isFound) << previous.string();

	SubtitleScreen screen;

	screen.Update(0.016f);

	EXPECT_EQ(screen.GetShownLineCount(), 0);
}
