#include "pch.h"
#include "Version.h"

using RoguelikeGame::FormatVersion;
using RoguelikeGame::GameTitle;
using RoguelikeGame::VersionString;

TEST(VersionTest, AReleaseIsThreeNumbers)
{
	EXPECT_EQ(FormatVersion(1, 4, 2, ""), "1.4.2");
	EXPECT_EQ(FormatVersion(1, 4, 2, nullptr), "1.4.2") << "пустая метка не должна оставлять дефис";
}

// Предрелиз отличается меткой через дефис - так требует semver.
TEST(VersionTest, APreReleaseCarriesItsStage)
{
	EXPECT_EQ(FormatVersion(0, 1, 0, "alpha"), "0.1.0-alpha");
	EXPECT_EQ(FormatVersion(0, 2, 0, "rc.1"), "0.2.0-rc.1");
}

TEST(VersionTest, ZeroIsAValidVersion)
{
	EXPECT_EQ(FormatVersion(0, 0, 0, ""), "0.0.0");
}

// Версия едет в заголовок окна и в лог: по ней узнают, какая сборка на руках.
TEST(VersionTest, TheTitleCarriesTheVersion)
{
	std::string title = GameTitle();

	EXPECT_NE(title.find(VersionString()), std::string::npos) << "в заголовке нет версии";
	EXPECT_NE(title.find(RoguelikeGame::GAME_NAME), std::string::npos) << "в заголовке нет имени игры";
}

/**
*	Число из заголовка читает не только компилятор.
*
*	Скрипт выпуска вытаскивает его регуляркой, чтобы поставить тег, поэтому
*	числа обязаны оставаться числами, а не выражениями.
*/
TEST(VersionTest, TheNumbersAreSaneForARelease)
{
	EXPECT_GE(RoguelikeGame::VERSION_MAJOR, 0);
	EXPECT_GE(RoguelikeGame::VERSION_MINOR, 0);
	EXPECT_GE(RoguelikeGame::VERSION_PATCH, 0);

	EXPECT_FALSE(VersionString().empty());
	EXPECT_NE(VersionString(), "0.0.0") << "выпускать нулевую версию нечего";
}
