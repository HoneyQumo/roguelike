#pragma once

#include <string>
#include <vector>
#include <Vector.h>

namespace RoguelikeGame
{
    /**
    *	Что сцена делает в начале шага. Сам шаг при этом остаётся отрезком времени:
    *	команда срабатывает один раз, а дальше идёт отсчёт.
    */
    enum class CutsceneCommand
    {
        None,
        TakeControl,
        GiveControl,
        LookAtTarget,
        LookAtPoint,
        LookAtHero,
        Say
    };

    struct CutsceneBeat
    {
        std::string action;
        float seconds = 0.f;

        CutsceneCommand command = CutsceneCommand::None;
        std::string target;
        XYZEngine::Vector2Df point = {0.f, 0.f};
        float travel = 0.f;

        // Бит несёт ровно одну команду, поэтому говорящий ставится рядом
        // с камерным, а не вместо него: сцена собирается парами битов.
        std::string line;
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
        const CutsceneBeat* GetCurrentBeat() const;
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
