#include "pch.h"
#include "GameSettings.h"
#include "ProjectFiles.h"
#include "SubtitleScreen.h"
#include <ResourceSystem.h>
#include <UiWidget.h>
#include <vector>

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

namespace
{
	// Экран раскладывается один раз при показе, а потом только по требованию -
	// ровно так, как это делает UiManager каждый кадр.
	void Show(SubtitleScreen& screen, float width = 1280.f, float height = 720.f)
	{
		screen.Resize({width, height});
		screen.Update(0.016f);
		screen.Relayout({width, height});
	}

	std::vector<sf::FloatRect> VisibleLines(const SubtitleScreen& screen)
	{
		std::vector<sf::FloatRect> shown;
		const XYZEngine::UiWidget& root = screen.GetRoot();

		for (std::size_t index = 0; index < root.GetChildrenCount(); index++)
		{
			if (root.GetChild(index).IsVisible())
			{
				shown.push_back(root.GetChild(index).GetBounds());
			}
		}

		return shown;
	}
}

// Пока раскладка шла только при показе экрана, строки ложились друг на друга
// у самого низа: SetOffset менял поле, а bounds оставались прежними.
TEST_F(SubtitleScreenTest, LinesStandOneAboveTheOtherAndNotInAHeap)
{
	ASSERT_TRUE(isFound) << previous.string();

	SubtitleQueue queue;
	SubtitleScreen screen;
	screen.SetQueue(&queue);

	queue.Say("Охранник", "Стой!", 3.f);
	queue.Say("Тяжёлый", "Он здесь!", 3.f);
	Show(screen);

	std::vector<sf::FloatRect> lines = VisibleLines(screen);
	ASSERT_EQ(lines.size(), 2u);
	EXPECT_LT(lines[0].top, lines[1].top) << "первая реплика должна быть выше последней";
	EXPECT_GE(lines[1].top - lines[0].top, lines[0].height) << "строки налезают друг на друга";
}

TEST_F(SubtitleScreenTest, EveryLineStaysOnTheScreen)
{
	ASSERT_TRUE(isFound) << previous.string();

	SubtitleQueue queue;
	SubtitleScreen screen;
	screen.SetQueue(&queue);

	for (int line = 0; line < RoguelikeGame::SUBTITLE_MAX_LINES; line++)
	{
		queue.Say("Радист",
			"Чужой на базе, поднимайте всех по тревоге и перекройте нижний коридор вместе со складом", 9.f);
	}
	Show(screen);

	for (const sf::FloatRect& line : VisibleLines(screen))
	{
		EXPECT_GE(line.top, 0.f);
		EXPECT_LE(line.top + line.height, 720.f);
	}
}
