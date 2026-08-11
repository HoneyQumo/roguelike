#pragma once

#include <SFML/Graphics/Color.hpp>

namespace RoguelikeGame
{
	constexpr int SCREEN_WIDTH = 1280;
	constexpr int SCREEN_HEIGHT = 720;

	constexpr float TILE_SIZE = 64.f;

	constexpr int CHARACTER_FRAME_SIZE = 32;
	constexpr int CHARACTER_FRAMES_IN_MAP = 20;
	constexpr int CHARACTER_SPRITE_SIZE = 64;
	constexpr float CHARACTER_COLLIDER_SIZE = 48.f;

	constexpr int WALK_FIRST_FRAME = 0;
	constexpr int WALK_FRAMES_COUNT = 4;
	constexpr float WALK_FRAMERATE = 8.f;

	constexpr float PLAYER_SPEED = 250.f;

	constexpr float ENEMY_SPEED = 150.f;
	constexpr float ENEMY_DETECTION_RADIUS = 300.f;

	constexpr float MUSIC_VOLUME = 15.f;

	constexpr const char* TEST_LEVEL_PATH = "Resources/Levels/test_level.config";

	const sf::Color WALL_COLOR = { 92, 86, 80 };
	const sf::Color FLOOR_COLOR = { 46, 42, 38 };
}
