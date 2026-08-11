#pragma once

#include <SFML/Graphics/Color.hpp>
#include "EnemyConfig.h"

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
	constexpr int PLAYER_WEAPON_FRAME = 17;

	constexpr float PLAYER_SPEED = 250.f;
	constexpr float PLAYER_MAX_HEALTH = 100.f;
	constexpr float PLAYER_ARMOR = 5.f;
	constexpr float PLAYER_ATTACK_DAMAGE = 25.f;
	constexpr float PLAYER_ATTACK_COOLDOWN = 0.3f;
	constexpr float PLAYER_PROJECTILE_SPEED = 800.f;

	constexpr int WEAPON_SPRITE_SIZE = 48;
	constexpr float WEAPON_OFFSET_X = 8.f;
	constexpr float WEAPON_OFFSET_Y = -6.f;
	constexpr float SHOT_OFFSET = 34.f;

	constexpr float PROJECTILE_SIZE = 10.f;
	constexpr float PROJECTILE_LIFETIME = 2.f;

	constexpr float HEALTH_BAR_WIDTH = 48.f;
	constexpr float HEALTH_BAR_HEIGHT = 6.f;
	constexpr float HEALTH_BAR_OFFSET_Y = 36.f;

	constexpr float MUSIC_VOLUME = 15.f;
	constexpr float SHOT_VOLUME = 20.f;
	constexpr float HURT_VOLUME = 35.f;

	constexpr const char* TEST_LEVEL_PATH = "Resources/Levels/test_level.config";
	constexpr const char* LOG_FILE_PATH = "log.txt";

	const sf::Color WALL_COLOR = { 92, 86, 80 };
	const sf::Color FLOOR_COLOR = { 46, 42, 38 };
	const sf::Color PROJECTILE_COLOR = { 255, 214, 120 };

	const EnemyConfig PRISONER_CONFIG = {
		"Enemy", "enemy",
		0, 5,
		5, 3,
		0, 0,
		10, 1,
		-1,
		150.f, 300.f, 40.f, 50.f, 0.f,
		120.f, 8.f, 1.f, 450.f
	};

	const EnemyConfig RIFLEMAN_CONFIG = {
		"Rifleman", "rifleman",
		5, 4,
		0, 4,
		10, 2,
		15, 2,
		12,
		110.f, 420.f, 220.f, 70.f, 5.f,
		360.f, 12.f, 1.4f, 700.f
	};
}
