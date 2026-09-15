#pragma once

namespace RoguelikeGame
{
    constexpr int HATCH_FRAMES = 5;
    constexpr float HATCH_OPEN_TIME = 0.9f;
    constexpr int LEVER_FRAMES = 2;

    enum class HatchState
    {
        Shut,
        Opening,
        Open
    };

    constexpr int HatchFrame(HatchState state, float since, float openTime)
    {
        if (state == HatchState::Shut)
        {
            return 0;
        }

        if (state == HatchState::Open || openTime <= 0.f)
        {
            return HATCH_FRAMES - 1;
        }

        float part = since / openTime;
        if (part < 0.f)
        {
            part = 0.f;
        }

        int frame = static_cast<int>(part * HATCH_FRAMES);

        return frame >= HATCH_FRAMES ? HATCH_FRAMES - 1 : frame;
    }

    constexpr HatchState NextHatchState(HatchState state, float since, float openTime)
    {
        if (state != HatchState::Opening)
        {
            return state;
        }

        return since >= openTime ? HatchState::Open : HatchState::Opening;
    }

    constexpr int LeverFrame(bool isPulled)
    {
        return isPulled ? 1 : 0;
    }
}
