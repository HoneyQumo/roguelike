#pragma once

#include "Scene.h"
#include "Level.h"
#include "CarArrival.h"
#include "CutsceneTimeline.h"
#include "HudScreen.h"
#include "InventoryScreen.h"
#include "FadeScreen.h"
#include "MessageScreen.h"
#include <memory>
#include <GameObject.h>
#include <Cooldown.h>

namespace RoguelikeGame
{
    class CutscenePlayerComponent;
    class EscapeCarComponent;

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
        XYZEngine::GameObject* cutscene = nullptr;
        XYZEngine::GameObject* camera = nullptr;

        // Что именно выключила сцена: вернуть надо ровно это.
        std::vector<XYZEngine::Component*> takenParts;
        float escapeSpeed = 0.f;
        float arrivalTime = 0.f;
        float boardTime = 0.f;
        float smokeTime = 0.f;
        XYZEngine::Vector2Df markDrift = {0.f, 0.f};
        bool wasSkidding = false;
        XYZEngine::GameObject* particles = nullptr;
        XYZEngine::GameObject* uiRoot = nullptr;
        std::unique_ptr<HudScreen> hudScreen;
        std::unique_ptr<InventoryScreen> inventoryScreen;
        std::unique_ptr<MessageScreen> messageScreen;
        std::unique_ptr<FadeScreen> fadeScreen;
        XYZEngine::GameObject* music = nullptr;
        XYZEngine::GameObject* crosshair = nullptr;

        State state = State::Playing;
        XYZEngine::Cooldown gameOverDelay;
        int currentLevelIndex = 0;
        int pendingLevelIndex = -1;

        void SetPaused(bool isPaused);
        void ShowGameOver();
        void UpdateOverlay();
        void UpdateWavePanel();
        void UpdateChasePanel();
        void ShowLevelTitle();

        bool LoadLevel(int levelIndex);
        void SubscribeExit();
        void SubscribeBoss();
        void SubscribeWaves();
        void SubscribePursuit();
        void SubscribeEscape();
        void SubscribeLevers();
        void PlayHatchScene(const std::string& hatchName);
        CutscenePlayerComponent* StartCutscene(std::vector<CutsceneBeat> beats);
        void PlayEscape();
        void TakeControl();
        void SetControlTaken(bool isTaken);
        void DriveEscape(XYZEngine::GameObject* carObject, float deltaTime);
        void SendCarAway(XYZEngine::GameObject* carObject, EscapeCarComponent* car);
        void ShowCar(XYZEngine::GameObject* carObject, bool isShown);
        void OpenCarDoor(XYZEngine::GameObject* carObject, bool isOpen);
        void PlayArrival();
        void DriveArrival(XYZEngine::GameObject* carObject, float deltaTime);
        void TrailSkid(EscapeCarComponent* car, const CarPose& pose, const XYZEngine::Vector2Df& step, float deltaTime);
        void SilenceCar();
        void OnWavesCleared();
        void OnBossDefeated();
        void RequestNextLevel();
        void GoToPendingLevel();
    };
}
