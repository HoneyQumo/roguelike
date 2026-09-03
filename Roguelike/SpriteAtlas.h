#pragma once

#include <Vector.h>

namespace RoguelikeGame
{
    /**	Раскладка спрайтов из Docs/Sprites/character.json.
    *	Кадр 64x64 с пивотом (32, 32)
    *	Строка — анимация, столбец — кадр
    */
    constexpr int CHARACTER_FRAME_SIZE = 64;
    constexpr int CHARACTER_ATLAS_COLUMNS = 12;
    constexpr int ENEMY_ATLAS_ROWS = 8;
    constexpr int PLAYER_ATLAS_ROWS = 18;
    constexpr int ENEMY_ATLAS_FRAMES = CHARACTER_ATLAS_COLUMNS * ENEMY_ATLAS_ROWS;
    constexpr int PLAYER_ATLAS_FRAMES = CHARACTER_ATLAS_COLUMNS * PLAYER_ATLAS_ROWS;

    struct CharacterAnimation
    {
        int row;
        int frames;
        float framesPerSecond;
    };

    constexpr float FramesPerSecond(float millisecondsPerFrame)
    {
        return 1000.f / millisecondsPerFrame;
    }

    constexpr int AtlasFrameIndex(int row, int column)
    {
        return row * CHARACTER_ATLAS_COLUMNS + column;
    }

    constexpr CharacterAnimation IDLE_ANIMATION = {0, 6, FramesPerSecond(175.f)};
    constexpr CharacterAnimation WALK_ANIMATION = {1, 8, FramesPerSecond(110.f)};
    constexpr CharacterAnimation RUN_ANIMATION = {2, 8, FramesPerSecond(75.f)};
    constexpr CharacterAnimation SHOOT_ANIMATION = {3, 4, FramesPerSecond(60.f)};
    constexpr CharacterAnimation RELOAD_ANIMATION = {4, 12, FramesPerSecond(90.f)};
    constexpr CharacterAnimation MELEE_ANIMATION = {5, 6, FramesPerSecond(60.f)};
    constexpr CharacterAnimation HURT_ANIMATION = {6, 3, FramesPerSecond(80.f)};
    constexpr CharacterAnimation DEATH_ANIMATION = {7, 8, FramesPerSecond(110.f)};

    constexpr int HEAVY_ANIMATION_ROW = 8;
    constexpr int HEAVY_ANIMATION_FRAMES = 12;
    constexpr float HEAVY_FRAME_SECONDS[HEAVY_ANIMATION_FRAMES] = {
        0.070f, 0.085f, 0.095f, 0.110f, 0.110f, 0.090f, 0.090f, 0.034f, 0.034f, 0.046f, 0.095f, 0.115f
    };

    constexpr int HEAVY_CHARGE_LOOP_FIRST = 3;
    constexpr int HEAVY_CHARGE_LOOP_LAST = 4;
    constexpr int HEAVY_CHARGED_LOOP_FIRST = 5;
    constexpr int HEAVY_CHARGED_LOOP_LAST = 6;
    constexpr int HEAVY_RELEASE_FRAME = 7;
    constexpr int HEAVY_HIT_FIRST_FRAME = 8;
    constexpr int HEAVY_HIT_LAST_FRAME = 9;
    constexpr float HEAVY_CHARGE_TIME = 0.44f;

    constexpr int SWAP_ANIMATION_ROW = 9;
    constexpr int SWAP_ANIMATION_FRAMES = 10;
    constexpr float SWAP_FRAME_SECONDS[SWAP_ANIMATION_FRAMES] = {
        0.055f, 0.060f, 0.065f, 0.070f, 0.070f, 0.060f, 0.055f, 0.045f, 0.035f, 0.030f
    };

    constexpr int SWAP_CHANGE_FRAME = 5;

    constexpr int ROLL_ANIMATION_ROW0 = 10;
    constexpr int ROLL_DIRECTIONS = 8;
    constexpr int ROLL_ANIMATION_FRAMES = 10;
    constexpr float ROLL_FRAMES_PER_SECOND = FramesPerSecond(50.f);

    constexpr int RollAtlasRow(int direction)
    {
        return ROLL_ANIMATION_ROW0 + (ROLL_DIRECTIONS - direction) % ROLL_DIRECTIONS;
    }

    constexpr int RollFirstFrame(int direction)
    {
        return AtlasFrameIndex(RollAtlasRow(direction), 0);
    }

    constexpr float ROLL_MOVE_SPEED[ROLL_ANIMATION_FRAMES] = {
        0.10f, 0.55f, 1.00f, 1.00f, 0.92f, 0.66f, 0.46f, 0.30f, 0.10f, 0.00f
    };

    constexpr int ROLL_INVULNERABLE_FIRST_FRAME = 2;
    constexpr int ROLL_INVULNERABLE_LAST_FRAME = 7;

    /**	
    *	Покадровое смещение слоя оружия, в пикселях кадра.
    *	В Idle оружие намеренно неподвижно.
    */
    struct FrameOffset
    {
        float x;
        float y;
    };

    constexpr FrameOffset WALK_WEAPON_OFFSET[8] = {{0, 0}, {0, 1}, {0, 0}, {0, -1}, {0, 0}, {0, 1}, {0, 0}, {0, -1}};
    constexpr FrameOffset RUN_WEAPON_OFFSET[8] = {{0, 3}, {0, 4}, {0, 3}, {0, 2}, {0, 3}, {0, 4}, {0, 3}, {0, 2}};
    constexpr FrameOffset SHOOT_WEAPON_OFFSET[4] = {{-3, 0}, {-2, 0}, {-1, 0}, {0, 0}};
    constexpr FrameOffset RELOAD_WEAPON_OFFSET[12] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
    constexpr FrameOffset MELEE_WEAPON_OFFSET[6] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
    constexpr FrameOffset HEAVY_WEAPON_OFFSET[HEAVY_ANIMATION_FRAMES] = {
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {1, -1}, {-1, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
    };
    constexpr FrameOffset SWAP_WEAPON_OFFSET[SWAP_ANIMATION_FRAMES] = {
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
    };
    constexpr FrameOffset ROLL_WEAPON_OFFSET[ROLL_ANIMATION_FRAMES] = {
        {0.f, 1.5f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f},
        {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {-1.f, 1.f}, {0.f, 0.f}
    };
    constexpr bool ROLL_WEAPON_HIDDEN[ROLL_ANIMATION_FRAMES] = {
        false, true, true, true, true, true, true, true, false, false
    };
    constexpr FrameOffset HURT_WEAPON_OFFSET[3] = {{-2, 1}, {-1, 0}, {0, 0}};
    constexpr FrameOffset DEATH_WEAPON_OFFSET[8] = {{0, 1}, {-2, 2}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};

    constexpr float MELEE_WEAPON_ROTATION[6] = {34.f, 18.f, -22.f, -52.f, -28.f, 0.f};
    constexpr float HEAVY_WEAPON_ROTATION[HEAVY_ANIMATION_FRAMES] = {
        0.f, 44.f, 88.f, 116.f, 110.f, 122.f, 115.f, 48.f, -20.f, -78.f, -40.f, 0.f
    };
    constexpr float SWAP_WEAPON_ROTATION[SWAP_ANIMATION_FRAMES] = {
        22.f, 56.f, 0.f, 0.f, 0.f, 0.f, 48.f, 22.f, 6.f, 0.f
    };

    constexpr int MELEE_HIT_FRAME = 3;

    constexpr float HEAVY_MOVE_SPEED[HEAVY_ANIMATION_FRAMES] = {
        0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.55f, 1.00f, 0.72f, 0.18f, 0.f
    };

    constexpr bool SWAP_WEAPON_HIDDEN[SWAP_ANIMATION_FRAMES] = {
        false, false, true, true, true, true, false, false, false, false
    };
    constexpr int SWAP_WEAPON_VARIANT[SWAP_ANIMATION_FRAMES] = {0, 1, 0, 0, 0, 0, 1, 0, 0, 0};
    constexpr FrameOffset SWAP_STOW_OFFSET[SWAP_ANIMATION_FRAMES] = {
        {0, 0}, {0, 0}, {-13, 12}, {-17, 7}, {-18, -1}, {-13, -11}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
    };
    constexpr float SWAP_STOW_ROTATION[SWAP_ANIMATION_FRAMES] = {
        0.f, 0.f, -96.f, -124.f, -150.f, -170.f, 0.f, 0.f, 0.f, 0.f
    };

    /**
    *	Столбец weapons.png на каждом кадре перезарядки: 
    *	0-обычный хват, 
    *	1-рука к магазину, 
    *	2-магазин вынут.
    */
    constexpr int RELOAD_WEAPON_VARIANT[12] = {0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 1, 0};

    // Кадр смерти с которого оружие пропадает
    constexpr int DEATH_WEAPON_HIDDEN_FROM_FRAME = 2;

    struct FxStrip
    {
        int x;
        int y;
        int width;
        int height;
        int frames;
        float pivotX;
        float pivotY;
        float millisecondsPerFrame;
    };

    constexpr FxStrip FX_MUZZLE_FLASH = {0, 0, 32, 24, 3, 4.f, 12.f, 60.f};
    constexpr FxStrip FX_BLOOD_POOL = {0, 24, 64, 64, 10, 32.f, 32.f, 150.f};
    constexpr FxStrip FX_EXPLOSION = {0, 88, 64, 64, 7, 32.f, 32.f, 55.f};
    constexpr FxStrip FX_BLOOD_HIT = {0, 152, 32, 24, 4, 6.f, 12.f, 45.f};
    constexpr FxStrip FX_BLOOD_SPECK = {0, 176, 32, 24, 1, 6.f, 12.f, 45.f};
    constexpr FxStrip FX_IMPACT = {0, 200, 24, 24, 4, 4.f, 12.f, 40.f};
    constexpr FxStrip FX_PICKUP = {0, 224, 24, 24, 6, 12.f, 12.f, 150.f};
    constexpr FxStrip FX_HAND_ITEM = {0, 248, 16, 16, 6, 8.f, 8.f, 90.f};
    constexpr FxStrip FX_SHELL_FLY = {0, 264, 12, 12, 4, 6.f, 6.f, 60.f};
    constexpr FxStrip FX_ROCKET = {0, 276, 28, 12, 2, 4.f, 6.f, 70.f};
    constexpr FxStrip FX_BULLET = {0, 288, 24, 8, 3, 2.f, 4.f, 60.f};

    inline XYZEngine::Vector2Df ToWorldOffset(float frameX, float frameY)
    {
        return {frameX, -frameY};
    }

    constexpr float ToWorldAngle(float frameAngle)
    {
        return -frameAngle;
    }
}
