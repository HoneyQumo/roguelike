#include "pch.h"
#include "SpeechCatalogLoader.h"
#include "SpeechDirector.h"
#include <sstream>

using RoguelikeGame::SoundPlace;
using RoguelikeGame::SpeechCatalog;
using RoguelikeGame::SpeechCatalogLoader;
using RoguelikeGame::SpeechDirector;
using RoguelikeGame::SubtitleQueue;

namespace
{
	const std::string SPEECH =
		"[line guard_1]\nspeaker Охранник\ntext Вот ты где!\n"
		"[line guard_2]\nspeaker Охранник\ntext Вижу его!\n"
		"[set guard_spotted]\nline guard_1\nline guard_2\n";

	SpeechCatalog ParseSpeech()
	{
		std::istringstream input(SPEECH);

		return SpeechCatalogLoader::Parse(input, "test");
	}
}

class SpeechDirectorTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		catalog = ParseSpeech();
		queue.Clear();

		SpeechDirector::Current().SetCatalog(&catalog);
		SpeechDirector::Current().SetQueue(&queue);
	}

	// Директор общий на программу: не убрать за собой значит протечь в соседей.
	void TearDown() override
	{
		SpeechDirector::Current().Silence();
		SpeechDirector::Current().SetCatalog(nullptr);
		SpeechDirector::Current().SetQueue(nullptr);
	}

	SpeechCatalog catalog;
	SubtitleQueue queue;
};

TEST_F(SpeechDirectorTest, TheSpokenLineReachesTheScreen)
{
	EXPECT_TRUE(SpeechDirector::Current().Say("guard_1", nullptr, SoundPlace::AtListener));

	ASSERT_EQ(queue.GetLines().size(), 1u);
	EXPECT_EQ(queue.GetLines()[0].speaker, "Охранник");
	EXPECT_EQ(queue.GetLines()[0].text, "Вот ты где!");
}

// Набор для врагов: любая из подходящих, чтобы не повторяться, но своя.
TEST_F(SpeechDirectorTest, ASetSpeaksOnlyItsOwnLines)
{
	for (int said = 0; said < 20; said++)
	{
		queue.Clear();
		ASSERT_TRUE(SpeechDirector::Current().SayFromSet("guard_spotted", nullptr, SoundPlace::InWorld));

		ASSERT_EQ(queue.GetLines().size(), 1u);
		const std::string& text = queue.GetLines()[0].text;
		EXPECT_TRUE(text == "Вот ты где!" || text == "Вижу его!") << text;
	}
}

TEST_F(SpeechDirectorTest, ASetEventuallySaysEachOfItsLines)
{
	bool heardFirst = false;
	bool heardSecond = false;

	for (int said = 0; said < 40 && !(heardFirst && heardSecond); said++)
	{
		queue.Clear();
		SpeechDirector::Current().SayFromSet("guard_spotted", nullptr, SoundPlace::InWorld);

		heardFirst = heardFirst || queue.GetLines()[0].text == "Вот ты где!";
		heardSecond = heardSecond || queue.GetLines()[0].text == "Вижу его!";
	}

	EXPECT_TRUE(heardFirst);
	EXPECT_TRUE(heardSecond);
}

// Пропавшую реплику иначе не найти: молчание выглядит как задумка.
TEST_F(SpeechDirectorTest, AnUnknownKeyIsRefusedAndNotShown)
{
	EXPECT_FALSE(SpeechDirector::Current().Say("nobody_says_this", nullptr, SoundPlace::AtListener));
	EXPECT_FALSE(SpeechDirector::Current().SayFromSet("no_such_set", nullptr, SoundPlace::InWorld));

	EXPECT_TRUE(queue.GetLines().empty());
}

TEST_F(SpeechDirectorTest, WithoutACatalogNobodySpeaks)
{
	SpeechDirector::Current().SetCatalog(nullptr);

	EXPECT_FALSE(SpeechDirector::Current().Say("guard_1", nullptr, SoundPlace::AtListener));
	EXPECT_TRUE(queue.GetLines().empty());
}

// Субтитр не должен пережить то, что его вызвало: сцену или локацию.
TEST_F(SpeechDirectorTest, SilenceClearsWhatWasSaid)
{
	SpeechDirector::Current().Say("guard_1", nullptr, SoundPlace::AtListener);
	ASSERT_FALSE(queue.GetLines().empty());

	SpeechDirector::Current().Silence();

	EXPECT_TRUE(queue.GetLines().empty());
}

TEST_F(SpeechDirectorTest, SpeakingWithoutAScreenIsNotACrash)
{
	SpeechDirector::Current().SetQueue(nullptr);

	EXPECT_TRUE(SpeechDirector::Current().Say("guard_1", nullptr, SoundPlace::AtListener));
}
