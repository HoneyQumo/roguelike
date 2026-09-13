#pragma once

#include "SpriteAtlas.h"

namespace RoguelikeGame
{
    /**	Раскладка листа босса «Кукловод» из Docs/Sprites/boss_puppeteer.json.
    *	Кадр 96x96 с пивотом (48, 48), взгляд +X, свободный поворот.
    *	Петля замаха крутится, пока код не даст выброс; выход разрешён
    *	с любого её кадра. first < 0 — петли в строке нет.
    */
    constexpr int BOSS_FRAME_SIZE = 96;
    constexpr int BOSS_ATLAS_COLUMNS = 12;
    constexpr int BOSS_ATLAS_ROWS = 7;
    constexpr int BOSS_ATLAS_FRAMES = BOSS_ATLAS_COLUMNS * BOSS_ATLAS_ROWS;
    constexpr float BOSS_PIVOT_X = 48.f;
    constexpr float BOSS_PIVOT_Y = 48.f;

    constexpr FrameOffset BOSS_MASK_ANCHOR = {60.f, 48.f};
    constexpr FrameOffset BOSS_CAST_ANCHOR = {72.f, 48.f};

    constexpr int BOSS_SLOT_IDLE = 0;
    constexpr int BOSS_SLOT_GLIDE = 1;
    constexpr int BOSS_SLOT_SUMMON = 2;
    constexpr int BOSS_SLOT_CURSE = 3;
    constexpr int BOSS_SLOT_HURT = 4;
    constexpr int BOSS_SLOT_DEATH = 5;
    constexpr int BOSS_SLOT_ENRAGE = 6;

    constexpr int BossAtlasFrameIndex(int row, int column)
    {
        return row * BOSS_ATLAS_COLUMNS + column;
    }

    struct BossAnimation
    {
        int row;
        int frames;
        const float* frameSeconds;
        bool looped;
        int windupLoopFirst;
        int windupLoopLast;
    };

    constexpr float PUPPETEER_IDLE_SECONDS[6] = {0.175f, 0.175f, 0.175f, 0.175f, 0.175f, 0.175f};
    constexpr float PUPPETEER_GLIDE_SECONDS[8] = {0.110f, 0.110f, 0.110f, 0.110f, 0.110f, 0.110f, 0.110f, 0.110f};
    constexpr float PUPPETEER_SUMMON_SECONDS[10] = {0.110f, 0.110f, 0.110f, 0.110f, 0.110f, 0.090f, 0.090f, 0.120f, 0.120f, 0.120f};
    constexpr float PUPPETEER_CURSE_SECONDS[8] = {0.110f, 0.110f, 0.110f, 0.110f, 0.080f, 0.080f, 0.130f, 0.130f};
    constexpr float PUPPETEER_HURT_SECONDS[3] = {0.080f, 0.080f, 0.080f};
    constexpr float PUPPETEER_DEATH_SECONDS[10] = {0.110f, 0.110f, 0.110f, 0.110f, 0.110f, 0.110f, 0.110f, 0.110f, 0.110f, 0.110f};
    constexpr float PUPPETEER_ENRAGE_SECONDS[8] = {0.120f, 0.120f, 0.120f, 0.120f, 0.120f, 0.120f, 0.120f, 0.120f};

    constexpr BossAnimation PUPPETEER_IDLE_ANIMATION = {0, 6, PUPPETEER_IDLE_SECONDS, true, -1, -1};
    constexpr BossAnimation PUPPETEER_GLIDE_ANIMATION = {1, 8, PUPPETEER_GLIDE_SECONDS, true, -1, -1};
    constexpr BossAnimation PUPPETEER_SUMMON_ANIMATION = {2, 10, PUPPETEER_SUMMON_SECONDS, false, 3, 4};
    constexpr BossAnimation PUPPETEER_CURSE_ANIMATION = {3, 8, PUPPETEER_CURSE_SECONDS, false, 2, 3};
    constexpr BossAnimation PUPPETEER_HURT_ANIMATION = {4, 3, PUPPETEER_HURT_SECONDS, false, -1, -1};
    constexpr BossAnimation PUPPETEER_DEATH_ANIMATION = {5, 10, PUPPETEER_DEATH_SECONDS, false, -1, -1};
    constexpr BossAnimation PUPPETEER_ENRAGE_ANIMATION = {6, 8, PUPPETEER_ENRAGE_SECONDS, false, -1, -1};

    constexpr FrameOffset PUPPETEER_SUMMON_THREAD_ANCHORS[10][4] = {
        {{28.65f, 25.44f}, {73.40f, 24.41f}, {73.40f, 71.59f}, {28.65f, 70.56f}},
        {{53.64f, 30.60f}, {69.46f, 37.68f}, {69.46f, 58.32f}, {53.64f, 65.40f}},
        {{61.98f, 32.33f}, {68.15f, 42.10f}, {68.15f, 53.90f}, {61.98f, 63.67f}},
        {{61.98f, 32.33f}, {68.15f, 42.10f}, {68.15f, 53.90f}, {61.98f, 63.67f}},
        {{59.16f, 31.61f}, {68.74f, 40.60f}, {68.74f, 55.40f}, {59.16f, 64.39f}},
        {{15.06f, 20.36f}, {77.87f, 17.07f}, {77.87f, 78.93f}, {15.06f, 75.64f}},
        {{16.64f, 25.11f}, {73.02f, 18.31f}, {73.02f, 77.69f}, {16.64f, 70.89f}},
        {{21.37f, 39.35f}, {58.49f, 22.04f}, {58.49f, 73.96f}, {21.37f, 56.65f}},
        {{18.25f, 30.22f}, {67.81f, 19.56f}, {67.81f, 76.44f}, {18.25f, 65.78f}},
        {{15.69f, 22.76f}, {75.43f, 17.53f}, {75.43f, 78.47f}, {15.69f, 73.24f}},
    };
    constexpr FrameOffset PUPPETEER_CURSE_THREAD_ANCHORS[8][4] = {
        {{20.82f, 42.68f}, {85.71f, 36.42f}, {55.20f, 74.74f}, {20.82f, 53.32f}},
        {{23.97f, 54.89f}, {92.00f, 48.00f}, {42.80f, 72.45f}, {23.97f, 41.11f}},
        {{23.97f, 54.89f}, {92.00f, 48.00f}, {42.80f, 72.45f}, {23.97f, 41.11f}},
        {{23.97f, 54.89f}, {92.00f, 48.00f}, {42.80f, 72.45f}, {23.97f, 41.11f}},
        {{16.55f, 38.68f}, {71.64f, 27.95f}, {59.49f, 78.72f}, {16.55f, 57.32f}},
        {{17.33f, 27.31f}, {59.27f, 26.81f}, {70.78f, 77.16f}, {17.33f, 68.69f}},
        {{21.37f, 39.35f}, {58.49f, 22.04f}, {58.49f, 73.96f}, {21.37f, 56.65f}},
        {{15.69f, 22.76f}, {75.43f, 17.53f}, {75.43f, 78.47f}, {15.69f, 73.24f}},
    };

    constexpr FxStrip FX_PUPPETEER_RIFT = {0, 0, 64, 64, 6, 32.f, 32.f, 0.090f};
    constexpr FxStrip FX_PUPPETEER_CLOUD = {0, 64, 96, 96, 6, 48.f, 48.f, 0.130f};
    constexpr FxStrip FX_PUPPETEER_SNAP = {0, 160, 48, 48, 4, 24.f, 24.f, 0.060f};
    constexpr FxStrip FX_PUPPETEER_MARK = {0, 208, 64, 64, 4, 32.f, 32.f, 0.110f};

    constexpr int FX_PUPPETEER_MARK_LOOP_FIRST = 2;
    constexpr int FX_PUPPETEER_MARK_LOOP_LAST = 3;

}
