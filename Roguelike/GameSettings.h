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

	// The character stands on the bottom edge of its frame, so the collider is shifted down to his feet.
	constexpr float CHARACTER_COLLIDER_WIDTH = 40.f;
	constexpr float CHARACTER_COLLIDER_HEIGHT = 48.f;
	constexpr float CHARACTER_COLLIDER_OFFSET_Y = -8.f;

	constexpr float WALK_FRAMERATE = 8.f;
	constexpr float IDLE_FRAMERATE = 4.f;
	constexpr float HURT_FRAMERATE = 12.f;
	constexpr float DEATH_FRAMERATE = 4.f;

	constexpr int PLAYER_WALK_FIRST_FRAME = 5;
	constexpr int PLAYER_WALK_FRAMES = 4;
	constexpr int PLAYER_IDLE_FIRST_FRAME = 0;
	constexpr int PLAYER_IDLE_FRAMES = 4;
	constexpr int PLAYER_HURT_FIRST_FRAME = 10;
	constexpr int PLAYER_HURT_FRAMES = 3;
	constexpr int PLAYER_DEATH_FIRST_FRAME = 15;
	constexpr int PLAYER_DEATH_FRAMES = 2;

	constexpr int ENEMY_WALK_FIRST_FRAME = 0;
	constexpr int ENEMY_WALK_FRAMES = 5;
	constexpr int ENEMY_IDLE_FIRST_FRAME = 5;
	constexpr int ENEMY_IDLE_FRAMES = 3;
	constexpr int ENEMY_DEATH_FIRST_FRAME = 10;
	constexpr int ENEMY_DEATH_FRAMES = 1;

	constexpr float PLAYER_SPEED = 250.f;
	constexpr float PLAYER_MAX_HEALTH = 100.f;
	constexpr float PLAYER_ARMOR = 5.f;

	constexpr float ENEMY_SPEED = 150.f;
	constexpr float ENEMY_DETECTION_RADIUS = 300.f;
	constexpr float ENEMY_MAX_HEALTH = 50.f;
	constexpr float ENEMY_ARMOR = 0.f;

	constexpr float HEALTH_BAR_WIDTH = 48.f;
	constexpr float HEALTH_BAR_HEIGHT = 6.f;
	constexpr float HEALTH_BAR_OFFSET_Y = 36.f;

	constexpr float MUSIC_VOLUME = 15.f;

	constexpr const char* TEST_LEVEL_PATH = "Resources/Levels/test_level.config";
	constexpr const char* LOG_FILE_PATH = "log.txt";

	const sf::Color WALL_COLOR = { 92, 86, 80 };
	const sf::Color FLOOR_COLOR = { 46, 42, 38 };
}
