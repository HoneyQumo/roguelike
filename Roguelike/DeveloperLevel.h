#pragma once

#include "Scene.h"
#include "Level.h"
#include "HudScreen.h"
#include "InventoryScreen.h"
#include "MessageScreen.h"
#include <memory>
#include <string>
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
            GameOver,
            Victory
        };

        Level level;
        XYZEngine::GameObject* player = nullptr;
        XYZEngine::GameObject* particles = nullptr;
        XYZEngine::GameObject* uiRoot = nullptr;
        std::unique_ptr<HudScreen> hudScreen;
        std::unique_ptr<InventoryScreen> inventoryScreen;
        std::unique_ptr<MessageScreen> messageScreen;
        XYZEngine::GameObject* music = nullptr;
        XYZEngine::GameObject* crosshair = nullptr;

        State state = State::Playing;
        XYZEngine::Cooldown gameOverDelay;
        int currentLevelIndex = 0;
        std::string pendingLevelId;

        void SetPaused(bool isPaused);
        void ShowGameOver();
        void UpdateOverlay();

        bool LoadLevel(int levelIndex);
        void SubscribeExit();
        void RequestNextLevel();
        void GoToPendingLevel();
    };
}
