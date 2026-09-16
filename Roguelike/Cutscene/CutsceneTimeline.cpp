#include "CutsceneTimeline.h"

namespace RoguelikeGame
{
    const std::string CutsceneTimeline::NOTHING;

    void CutsceneTimeline::SetBeats(std::vector<CutsceneBeat> newBeats)
    {
        beats = std::move(newBeats);
        current = -1;
        inBeat = 0.f;
        isOver = beats.empty();
    }

    // Возвращает true, когда сцена перешла на новый бит: вызывающему пора менять картинку.
    bool CutsceneTimeline::Advance(float deltaTime)
    {
        if (isOver)
        {
            return false;
        }

        if (current < 0)
        {
            current = 0;
            inBeat = 0.f;

            return true;
        }

        inBeat += deltaTime;

        bool hasChanged = false;
        while (!isOver && inBeat >= beats[current].seconds)
        {
            inBeat -= beats[current].seconds;

            if (current + 1 >= static_cast<int>(beats.size()))
            {
                isOver = true;
                inBeat = 0.f;
                break;
            }

            current++;
            hasChanged = true;
        }

        return hasChanged;
    }

    int CutsceneTimeline::GetCurrent() const
    {
        return current;
    }

    const std::string& CutsceneTimeline::GetCurrentAction() const
    {
        if (current < 0 || current >= static_cast<int>(beats.size()))
        {
            return NOTHING;
        }

        return beats[current].action;
    }

    float CutsceneTimeline::GetBeatProgress() const
    {
        if (current < 0 || current >= static_cast<int>(beats.size()) || beats[current].seconds <= 0.f)
        {
            return 1.f;
        }

        float part = inBeat / beats[current].seconds;

        return part > 1.f ? 1.f : part;
    }

    bool CutsceneTimeline::IsOver() const
    {
        return isOver;
    }

    bool CutsceneTimeline::IsEmpty() const
    {
        return beats.empty();
    }
}
