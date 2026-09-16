#pragma once

#include <string>
#include <vector>

namespace RoguelikeGame
{
    struct CutsceneBeat
    {
        std::string action;
        float seconds = 0.f;
    };

    /**
    *	Порядок и время сцены, без единого знания о мире: что делать на каждом бите,
    *	решает тот, кто её проигрывает.
    */
    class CutsceneTimeline
    {
    public:
        void SetBeats(std::vector<CutsceneBeat> newBeats);

        bool Advance(float deltaTime);

        int GetCurrent() const;
        const std::string& GetCurrentAction() const;
        float GetBeatProgress() const;
        bool IsOver() const;
        bool IsEmpty() const;

    private:
        std::vector<CutsceneBeat> beats;
        int current = -1;
        float inBeat = 0.f;
        bool isOver = false;

        static const std::string NOTHING;
    };
}
