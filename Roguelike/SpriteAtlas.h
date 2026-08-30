#pragma once

#include <Vector.h>

namespace RoguelikeGame
{
    /**	Раскладка спрайтов из Docs/Sprites/character.json.
    *	Кадр 64x64 с пивотом (32, 32)
    *	Строка — анимация, столбец — кадр, а движок индексирует атлас одним плоским списком по строкам.
    */
    constexpr int CHARACTER_FRAME_SIZE = 64;
    constexpr int CHARACTER_ATLAS_COLUMNS = 12;
    constexpr int CHARACTER_ATLAS_ROWS = 8;
    constexpr int CHARACTER_ATLAS_FRAMES = CHARACTER_ATLAS_COLUMNS * CHARACTER_ATLAS_ROWS;

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

    // Строки 2 (бег), 4 (перезарядка) и 5 (удар), а также восемь строк кувырка в player.png не реализованы:
    constexpr CharacterAnimation IDLE_ANIMATION = {0, 6, FramesPerSecond(175.f)};
    constexpr CharacterAnimation WALK_ANIMATION = {1, 8, FramesPerSecond(110.f)};
    constexpr CharacterAnimation SHOOT_ANIMATION = {3, 4, FramesPerSecond(60.f)};
    constexpr CharacterAnimation HURT_ANIMATION = {6, 3, FramesPerSecond(80.f)};
    constexpr CharacterAnimation DEATH_ANIMATION = {7, 8, FramesPerSecond(110.f)};

    /**	
    *	Покадровое смещение слоя оружия, в пикселях кадра.
    *	В Idle оружие намеренно неподвижно.
    */
    struct FrameOffset
    {
        int x;
        int y;
    };

    constexpr FrameOffset WALK_WEAPON_OFFSET[8] = {{0, 0}, {0, 1}, {0, 0}, {0, -1}, {0, 0}, {0, 1}, {0, 0}, {0, -1}};
    constexpr FrameOffset SHOOT_WEAPON_OFFSET[4] = {{-3, 0}, {-2, 0}, {-1, 0}, {0, 0}};
    constexpr FrameOffset HURT_WEAPON_OFFSET[3] = {{-2, 1}, {-1, 0}, {0, 0}};
    constexpr FrameOffset DEATH_WEAPON_OFFSET[8] = {{0, 1}, {-2, 2}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};

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
}
