#include "pch.h"
#include "ItemIconLayout.h"
#include "GameSettings.h"
#include "PixelBounds.h"
#include <vector>

using RoguelikeGame::IconWorldSize;
using RoguelikeGame::ITEM_WORLD_SIZE;

namespace
{
	RoguelikeGame::ItemIcon MakeIcon(const sf::IntRect& rect, float scale)
	{
		RoguelikeGame::ItemIcon icon;
		icon.rect = rect;
		icon.worldScale = scale;

		return icon;
	}

	float Longest(const sf::Vector2f& size)
	{
		return std::max(size.x, size.y);
	}
}

TEST(ItemIconLayoutTest, LongestSideIsTheSameForEveryIcon)
{
	sf::Vector2f pistol = IconWorldSize(MakeIcon({40, 163, 32, 25}, 1.f));
	sf::Vector2f rifle = IconWorldSize(MakeIcon({17, 4, 78, 24}, 1.f));

	EXPECT_FLOAT_EQ(Longest(pistol), ITEM_WORLD_SIZE);
	EXPECT_FLOAT_EQ(Longest(rifle), ITEM_WORLD_SIZE);
}

TEST(ItemIconLayoutTest, AspectRatioIsKept)
{
	sf::Vector2f size = IconWorldSize(MakeIcon({0, 0, 80, 20}, 1.f));

	EXPECT_FLOAT_EQ(size.x / size.y, 4.f);
}

TEST(ItemIconLayoutTest, ScaleTunesTheNormalizedSize)
{
	sf::Vector2f half = IconWorldSize(MakeIcon({0, 0, 32, 25}, 0.5f));

	EXPECT_FLOAT_EQ(Longest(half), ITEM_WORLD_SIZE * 0.5f);
}

TEST(ItemIconLayoutTest, EmptyRectHasNoSize)
{
	sf::Vector2f size = IconWorldSize(MakeIcon({0, 0, 0, 0}, 1.f));

	EXPECT_FLOAT_EQ(size.x, 0.f);
	EXPECT_FLOAT_EQ(size.y, 0.f);
}

TEST(ItemIconLayoutTest, PaddingInTheAtlasCellDoesNotShrinkTheItem)
{
	const int cellWidth = 112;
	const int cellHeight = 32;
	const sf::IntRect art = {40, 3, 32, 25};

	std::vector<sf::Uint8> pixels(static_cast<std::size_t>(cellWidth) * cellHeight * 4, 0);

	for (int y = art.top; y < art.top + art.height; ++y)
	{
		for (int x = art.left; x < art.left + art.width; ++x)
		{
			pixels[(static_cast<std::size_t>(y) * cellWidth + x) * 4 + 3] = 255;
		}
	}

	sf::IntRect cell = {0, 0, cellWidth, cellHeight};
	sf::IntRect trimmed = XYZEngine::OpaqueBounds(pixels.data(), cellWidth, cellHeight, cell);

	EXPECT_EQ(trimmed.width, art.width);
	EXPECT_EQ(trimmed.height, art.height);

	float cellScale = IconWorldSize(MakeIcon(cell, 1.f)).x / cellWidth;
	float artScale = IconWorldSize(MakeIcon(trimmed, 1.f)).x / trimmed.width;

	EXPECT_FLOAT_EQ(Longest(IconWorldSize(MakeIcon(trimmed, 1.f))), ITEM_WORLD_SIZE);
	EXPECT_GT(artScale, cellScale * 3.f);
}
