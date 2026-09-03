#pragma once

#include "Scene.h"
#include "Level.h"
#include <GameObject.h>
#include <Cooldown.h>

namespace RoguelikeGame
{
    class DeveloperLevel : public XYZEngine::Scene
    {
    public:
        void Start() override;
        void Update(float deltaTime) override;
        void Restart() override;
        void Stop() override;

    private:
        enum class State
        {
            Playing,
            PlayerDied,
            GameOver
        };

        Level level;
        XYZEngine::GameObject* player = nullptr;
        XYZEngine::GameObject* music = nullptr;
        XYZEngine::GameObject* crosshair = nullptr;
        XYZEngine::GameObject* ammoHud = nullptr;
        XYZEngine::GameObject* messageOverlay = nullptr;

        State state = State::Playing;
        XYZEngine::Cooldown gameOverDelay;

        void SetPaused(bool isPaused);
        void ShowGameOver();
        void UpdateOverlay();
    };
}
