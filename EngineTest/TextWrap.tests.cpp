#include "pch.h"
#include "ProjectFiles.h"
#include "TextUtils.h"
#include "TextWrap.h"
#include <SFML/Graphics/Text.hpp>

using XYZEngine::FromUtf8;
using XYZEngine::TextWidth;
using XYZEngine::WrapText;

namespace
{
	constexpr unsigned int SIZE = 22;
}

class TextWrapTest : public ProjectFiles::Test
{
protected:
	void SetUp() override
	{
		ProjectFiles::Test::SetUp();
		isFontReady = isFound && font.loadFromFile("Resources/Fonts/Roboto-Medium.ttf");
	}

	sf::Font font;
	bool isFontReady = false;
};

// Считать ширину надо тем же способом, каким рисует SFML: иначе перенос ляжет
// не там, где видно обрыв, и разойдётся с картинкой на пару букв.
TEST_F(TextWrapTest, TheWidthMatchesWhatSfmlDraws)
{
	ASSERT_TRUE(isFontReady);

	sf::String line = FromUtf8(u8"Стой, кто идёт");
	sf::Text drawn(line, font, SIZE);

	float counted = TextWidth(line, font, SIZE);

	// Совпадение не побайтовое: getLocalBounds не считает вынос первого и
	// последнего глифа за их шаг. Важно, что счёт не уезжает.
	EXPECT_NEAR(counted, drawn.getLocalBounds().width, 0.03f * counted);
}

TEST_F(TextWrapTest, ShortTextStaysOneLine)
{
	ASSERT_TRUE(isFontReady);

	auto lines = WrapText(FromUtf8(u8"Стой"), font, SIZE, 400.f);

	ASSERT_EQ(lines.size(), 1u);
	EXPECT_EQ(lines[0], FromUtf8(u8"Стой"));
}

TEST_F(TextWrapTest, LongTextBreaksOnSpaces)
{
	ASSERT_TRUE(isFontReady);

	sf::String said = FromUtf8(u8"Здесь кто-то был, проверьте нижний коридор и склад");
	auto lines = WrapText(said, font, SIZE, 200.f);

	ASSERT_GT(lines.size(), 1u);
	for (const sf::String& line : lines)
	{
		EXPECT_FALSE(line.isEmpty());
		EXPECT_NE(line[0], U' ') << "строка начинается с пробела";
	}
}

TEST_F(TextWrapTest, NoLineGrowsPastTheWidth)
{
	ASSERT_TRUE(isFontReady);

	sf::String said = FromUtf8(u8"Здесь кто-то был, проверьте нижний коридор и склад");
	auto lines = WrapText(said, font, SIZE, 200.f);

	for (const sf::String& line : lines)
	{
		EXPECT_LE(TextWidth(line, font, SIZE), 200.f) << "строка шире, чем разрешено";
	}
}

// Рубить слово посередине хуже, чем вылезти: половина слова на одной строке
// и половина на другой не читается вовсе.
TEST_F(TextWrapTest, AWordLongerThanTheWidthKeepsItsOwnLine)
{
	ASSERT_TRUE(isFontReady);

	sf::String said = FromUtf8(u8"Стой предупредительный");
	auto lines = WrapText(said, font, SIZE, 60.f);

	ASSERT_EQ(lines.size(), 2u);
	EXPECT_EQ(lines[1], FromUtf8(u8"предупредительный"));
}

TEST_F(TextWrapTest, ALineBreakInTheTextIsKept)
{
	ASSERT_TRUE(isFontReady);

	auto lines = WrapText(FromUtf8(u8"Стой\nкто идёт"), font, SIZE, 400.f);

	ASSERT_EQ(lines.size(), 2u);
	EXPECT_EQ(lines[0], FromUtf8(u8"Стой"));
	EXPECT_EQ(lines[1], FromUtf8(u8"кто идёт"));
}

TEST_F(TextWrapTest, NothingToSayGivesNoLines)
{
	ASSERT_TRUE(isFontReady);

	EXPECT_TRUE(WrapText(sf::String(), font, SIZE, 400.f).empty());
	EXPECT_FLOAT_EQ(TextWidth(sf::String(), font, SIZE), 0.f);
}
