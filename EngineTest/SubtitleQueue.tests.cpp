#include "pch.h"
#include "SubtitleQueue.h"

using RoguelikeGame::Subtitle;
using RoguelikeGame::SubtitleQueue;
using RoguelikeGame::SubtitleTimeFor;
using RoguelikeGame::SUBTITLE_MAX_LINES;

TEST(SubtitleQueueTest, WhatWasSaidIsOnTheScreen)
{
	SubtitleQueue queue;

	queue.Say("Охранник", "Вот ты где!", 3.f);

	ASSERT_EQ(queue.GetLines().size(), 1u);
	EXPECT_EQ(queue.GetLines()[0].speaker, "Охранник");
	EXPECT_EQ(queue.GetLines()[0].text, "Вот ты где!");
}

// Враг и напарник могут говорить разом: строки живут параллельно,
// а не ждут очереди.
TEST(SubtitleQueueTest, TwoVoicesLiveSideBySide)
{
	SubtitleQueue queue;

	queue.Say("Охранник", "Вот ты где!", 3.f);
	queue.Say("Радист", "Тревога!", 3.f);

	EXPECT_EQ(queue.GetLines().size(), 2u);
}

TEST(SubtitleQueueTest, EveryLineFadesByItsOwnTime)
{
	SubtitleQueue queue;

	queue.Say("Охранник", "Короткая", 1.f);
	queue.Say("Радист", "Длинная", 5.f);
	queue.Update(2.f);

	ASSERT_EQ(queue.GetLines().size(), 1u);
	EXPECT_EQ(queue.GetLines()[0].speaker, "Радист");
}

// В разговоре важнее последнее сказанное: когда мест нет, уходит самое старое.
TEST(SubtitleQueueTest, WhenThereIsNoRoomTheOldestGoes)
{
	SubtitleQueue queue;

	for (std::size_t said = 0; said < SUBTITLE_MAX_LINES + 2; said++)
	{
		queue.Say("Кто", "Фраза " + std::to_string(said), 5.f);
	}

	ASSERT_EQ(queue.GetLines().size(), SUBTITLE_MAX_LINES);
	EXPECT_EQ(queue.GetLines().front().text, "Фраза 2");
	EXPECT_EQ(queue.GetLines().back().text, "Фраза " + std::to_string(SUBTITLE_MAX_LINES + 1));
}

TEST(SubtitleQueueTest, NothingSaidIsNothingShown)
{
	SubtitleQueue queue;

	queue.Say("Кто", "", 3.f);
	queue.Say("Кто", "Фраза", 0.f);

	EXPECT_TRUE(queue.GetLines().empty());
}

TEST(SubtitleQueueTest, SilenceClearsTheScreen)
{
	SubtitleQueue queue;

	queue.Say("Кто", "Фраза", 5.f);
	queue.Clear();

	EXPECT_TRUE(queue.GetLines().empty());
}

TEST(SubtitleQueueTest, ALongerLineIsShownLonger)
{
	EXPECT_GT(SubtitleTimeFor("Чужой на базе, поднимайте всех"), SubtitleTimeFor("Стоять!"));
}

// Время считается по буквам, а не по байтам: в кириллице на букву два байта,
// и по байтам русская фраза висела бы вдвое дольше английской той же длины.
TEST(SubtitleQueueTest, RussianIsNotShownTwiceAsLongAsEnglish)
{
	EXPECT_FLOAT_EQ(SubtitleTimeFor("Стоять"), SubtitleTimeFor("Stoyat"));
}
