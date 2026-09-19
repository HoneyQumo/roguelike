#include "pch.h"
#include "ProjectFiles.h"
#include "SpeechCatalogLoader.h"
#include <filesystem>
#include <sstream>

using RoguelikeGame::SpeechCatalog;
using RoguelikeGame::SpeechCatalogLoader;
using RoguelikeGame::SpeechLine;

namespace
{
	SpeechCatalog ParseText(const std::string& text)
	{
		std::istringstream input(text);

		return SpeechCatalogLoader::Parse(input, "test");
	}

	const std::string SPEECH_FILE = "Resources/Speech/speech.config";
	const std::string VOICE_PATH = "Resources/Audio/Voice/";
}

TEST(SpeechCatalogTest, ALineKeepsWhoSaidItAndWhat)
{
	SpeechCatalog catalog = ParseText(
		"[line guard_hi]\n"
		"speaker Охранник\n"
		"text Вот ты где!\n"
		"sound voice_guard_1\n");

	const SpeechLine* line = catalog.FindLine("guard_hi");

	ASSERT_NE(line, nullptr);
	EXPECT_EQ(line->speaker, "Охранник");
	EXPECT_EQ(line->text, "Вот ты где!");
	EXPECT_EQ(line->sound, "voice_guard_1");
}

// Текст - это остаток строки целиком: в нём есть и пробелы, и запятые.
TEST(SpeechCatalogTest, TheTextKeepsItsSpacesAndCommas)
{
	SpeechCatalog catalog = ParseText(
		"[line radio_call]\n"
		"speaker Радист\n"
		"text Сюда, быстро!\n");

	ASSERT_NE(catalog.FindLine("radio_call"), nullptr);
	EXPECT_EQ(catalog.FindLine("radio_call")->text, "Сюда, быстро!");
}

TEST(SpeechCatalogTest, ALineWithoutSoundIsStillALine)
{
	SpeechCatalog catalog = ParseText(
		"[line silent]\n"
		"speaker Герой\n"
		"text Тихо.\n");

	ASSERT_NE(catalog.FindLine("silent"), nullptr);
	EXPECT_TRUE(catalog.FindLine("silent")->sound.empty());
}

TEST(SpeechCatalogTest, ASetGathersLinesForOneCue)
{
	SpeechCatalog catalog = ParseText(
		"[line a]\nspeaker Кто\ntext Раз\n"
		"[line b]\nspeaker Кто\ntext Два\n"
		"[set spotted]\nline a\nline b\n");

	const std::vector<std::string>* set = catalog.FindSet("spotted");

	ASSERT_NE(set, nullptr);
	ASSERT_EQ(set->size(), 2u);
	EXPECT_EQ((*set)[0], "a");
}

// Набор может ссылаться на реплику, которая объявлена ниже, поэтому
// ссылки проверяются в конце разбора, а не на месте.
TEST(SpeechCatalogTest, ASetMayPointAtALineDeclaredBelow)
{
	SpeechCatalog catalog = ParseText(
		"[set spotted]\nline later\n"
		"[line later]\nspeaker Кто\ntext Три\n");

	EXPECT_NE(catalog.FindSet("spotted"), nullptr);
}

TEST(SpeechCatalogTest, ASetPointingAtNothingIsAnError)
{
	EXPECT_THROW(ParseText("[set spotted]\nline missing\n"), std::runtime_error);
}

TEST(SpeechCatalogTest, ALineWithoutTextIsAnError)
{
	EXPECT_THROW(ParseText("[line mute]\nspeaker Кто\n"), std::runtime_error);
}

TEST(SpeechCatalogTest, TheSameKeyTwiceIsAnError)
{
	EXPECT_THROW(ParseText("[line a]\ntext Раз\n[line a]\ntext Два\n"), std::runtime_error);
	EXPECT_THROW(ParseText("[line a]\ntext Раз\n[set s]\nline a\n[set s]\nline a\n"), std::runtime_error);
}

TEST(SpeechCatalogTest, AnUnknownFieldIsAnError)
{
	EXPECT_THROW(ParseText("[line a]\nmood злой\ntext Раз\n"), std::runtime_error);
}

TEST(SpeechCatalogTest, AFieldOutsideOfABlockIsAnError)
{
	EXPECT_THROW(ParseText("text Раз\n"), std::runtime_error);
}

TEST(SpeechCatalogTest, CommentsAndBlankLinesAreSkipped)
{
	SpeechCatalog catalog = ParseText(
		"; каталог\n\n"
		"[line a]\n"
		"; кто говорит\n"
		"speaker Кто\n"
		"text Раз\n");

	EXPECT_EQ(catalog.GetLineCount(), 1u);
}

class ShippedSpeechTest : public ProjectFiles::Test
{
};

TEST_F(ShippedSpeechTest, TheCatalogOnDiskIsReadable)
{
	ASSERT_TRUE(isFound) << previous.string();

	SpeechCatalog catalog = SpeechCatalogLoader::Load(SPEECH_FILE);

	EXPECT_GT(catalog.GetLineCount(), 0u);
	EXPECT_GT(catalog.GetSetCount(), 0u);
}

// Реплика без файла - это тишина на месте голоса, и заметить её можно
// только на слух. Поэтому сверяем с диском.
TEST_F(ShippedSpeechTest, EverySoundNamedInTheCatalogLiesOnDisk)
{
	ASSERT_TRUE(isFound) << previous.string();

	SpeechCatalog catalog = SpeechCatalogLoader::Load(SPEECH_FILE);

	int checked = 0;
	for (const char* voice : {"guard", "heavy", "radio"})
	{
		const std::vector<std::string>* set = catalog.FindSet(std::string(voice) + "_spotted");
		ASSERT_NE(set, nullptr) << voice;

		for (const std::string& id : *set)
		{
			const SpeechLine* line = catalog.FindLine(id);
			ASSERT_NE(line, nullptr) << id;
			ASSERT_FALSE(line->sound.empty()) << id;

			EXPECT_TRUE(std::filesystem::exists(VOICE_PATH + line->sound + ".wav")) << line->sound;
			checked++;
		}
	}

	EXPECT_EQ(checked, 9);
}
