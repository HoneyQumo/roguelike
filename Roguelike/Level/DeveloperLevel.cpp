#include "DeveloperLevel.h"
#include "GameSettings.h"
#include "GameResources.h"
#include "LevelBuilder.h"
#include "ActAssembler.h"
#include "LevelLoader.h"
#include "LevelProgression.h"
#include "Player.h"
#include "Music.h"
#include "Crosshair.h"
#include "Particles.h"
#include "BossBrainComponent.h"
#include "CutscenePlayerComponent.h"
#include "PlayerHudBinderComponent.h"
#include "EscapeCarComponent.h"
#include "WaveDirectorComponent.h"
#include "Fx.h"
#include "LevelExitComponent.h"
#include "UiRoot.h"
#include <Engine.h>
#include <GameWorld.h>
#include <ParticleSystem.h>
#include <UiManager.h>
#include <FrameClock.h>
#include <InputSystem.h>
#include <RectangleRendererComponent.h>
#include <RenderSystem.h>
#include <MusicComponent.h>
#include "HealthComponent.h"
#include <LoggerRegistry.h>
#include <string>

using namespace XYZEngine;

namespace RoguelikeGame
{
    void DeveloperLevel::Start()
    {
        LOG_INFO("Developer level is starting");

        state = State::Playing;
        gameOverDelay.Stop();

        currentLevelIndex = 0;
        pendingLevelIndex = -1;

        LoadLevel(currentLevelIndex);

        particles = CreateParticles();

        {
            try
            {
                player = CreatePlayer(level.GetStartPosition());

                auto health = player->GetComponent<HealthComponent>();
                if (health != nullptr)
                {
                    health->SubscribeDeath([this](const DeathInfo& death)
                    {
                        state = State::PlayerDied;
                        gameOverDelay.Start(GAME_OVER_DELAY);
                    });
                }
            }
            catch (const std::exception& exception)
            {
                LOG_ERROR(std::string("Player is not created: ") + exception.what());
            }
        }

        SubscribeExit();
        SubscribeBoss();
        SubscribeWaves();
        SubscribeEscape();

        music = CreateMusic(MAIN_THEME_MUSIC, MUSIC_VOLUME);

        try
        {
            crosshair = CreateCrosshair();
        }
        catch (const std::exception& exception)
        {
            LOG_ERROR(std::string("Crosshair is not created: ") + exception.what());
        }

        hudScreen = std::make_unique<HudScreen>();
        inventoryScreen = std::make_unique<InventoryScreen>();
        messageScreen = std::make_unique<MessageScreen>();
        fadeScreen = std::make_unique<FadeScreen>();
        uiRoot = CreateUiRoot(*hudScreen, *inventoryScreen, *messageScreen, *fadeScreen);

        fadeScreen->Blackout();
        fadeScreen->FadeIn(LEVEL_FADE_IN_TIME);

        ShowLevelTitle();
    }

    bool DeveloperLevel::LoadLevel(int levelIndex)
    {
        const LevelEntry* entry = GameResources::GetLevels().GetAt(levelIndex);
        std::string levelFile = entry != nullptr ? entry->filePath : TEST_LEVEL_FILE;

        try
        {
            bool isAct = entry != nullptr && entry->isAct;
            LevelData levelData = isAct ? LoadAct(levelFile) : LevelLoader::Load(levelFile);
            level = LevelBuilder::Build(levelData, GameResources::GetItems(), GameResources::GetProps());
        }
        catch (const std::exception& exception)
        {
            LOG_ERROR(std::string("Level is not loaded: ") + exception.what());
            LOG_WARN("Game continues with an empty level");
            return false;
        }

        currentLevelIndex = levelIndex;
        return true;
    }

    void DeveloperLevel::SubscribeExit()
    {
        XYZEngine::GameObject* exitObject = level.GetExit();
        if (exitObject == nullptr)
        {
            return;
        }

        auto exitComponent = exitObject->GetComponent<LevelExitComponent>();
        if (exitComponent == nullptr)
        {
            return;
        }

        exitComponent->SubscribeEntered([this]() { RequestNextLevel(); });
        exitComponent->SubscribeBlocked([this]()
        {
            if (hudScreen != nullptr)
            {
                hudScreen->ShowNotice(level.GetWaveDirector() != nullptr ? WAVE_GATE_NOTICE : BOSS_GATE_NOTICE);
            }
        });
    }

    void DeveloperLevel::SubscribeBoss()
    {
        XYZEngine::GameObject* bossObject = level.GetBoss();
        if (bossObject == nullptr)
        {
            return;
        }

        auto bossHealth = bossObject->GetComponent<HealthComponent>();
        if (bossHealth != nullptr)
        {
            bossHealth->SubscribeDeath([this](const DeathInfo& death) { OnBossDefeated(); });
        }

        auto brain = bossObject->GetComponent<BossBrainComponent>();
        if (brain != nullptr)
        {
            brain->SubscribeMinionSpawned([this](XYZEngine::GameObject* minion) { level.Add(minion); });
        }
    }

    void DeveloperLevel::SubscribeWaves()
    {
        XYZEngine::GameObject* directorObject = level.GetWaveDirector();
        if (directorObject == nullptr)
        {
            return;
        }

        auto director = directorObject->GetComponent<WaveDirectorComponent>();
        if (director == nullptr)
        {
            return;
        }

        director->SetHero(player);

        // Врагов волны кладём в уровень, иначе они переживут смену локации.
        director->SubscribeEnemySpawned([this](XYZEngine::GameObject* enemy) { level.Add(enemy); });

        director->SubscribeWaveStarted([this](int current, int total)
        {
            if (hudScreen != nullptr)
            {
                hudScreen->ShowNotice(std::string(WAVE_STARTED_NOTICE) + std::to_string(current)
                    + WAVE_OF_NOTICE + std::to_string(total));
            }
        });

        director->SubscribeWaveCleared([this](int current, int total)
        {
            if (hudScreen != nullptr && current < total)
            {
                hudScreen->ShowNotice(std::string(WAVE_STARTED_NOTICE) + std::to_string(current) + WAVE_CLEARED_NOTICE);
            }
        });

        director->SubscribeCleared([this]() { OnWavesCleared(); });
    }


    void DeveloperLevel::SubscribeEscape()
    {
        XYZEngine::GameObject* carObject = level.GetEscapeCar();
        if (carObject == nullptr)
        {
            return;
        }

        auto car = carObject->GetComponent<EscapeCarComponent>();
        if (car == nullptr)
        {
            return;
        }

        car->SubscribeBoarded([this]() { PlayEscape(); });
    }

    void DeveloperLevel::PlayEscape()
    {
        XYZEngine::GameObject* carObject = level.GetEscapeCar();
        if (carObject == nullptr || player == nullptr || cutscene != nullptr)
        {
            return;
        }

        cutscene = XYZEngine::GameWorld::Instance()->CreateGameObject(CUTSCENE_OBJECT_NAME);
        level.Add(cutscene);

        auto scene = cutscene->AddComponent<CutscenePlayerComponent>();
        scene->SetBeats({
            {ESCAPE_BEAT_BOARD, ESCAPE_BOARD_TIME},
            {ESCAPE_BEAT_DRIVE, ESCAPE_DRIVE_TIME},
            {ESCAPE_BEAT_LEAVE, ESCAPE_LEAVE_TIME},
        });

        // Игрок становится пассажиром: его выключают, а камера едет вместе с машиной.
        scene->SubscribeBeatStarted([this, carObject](const std::string& beat)
        {
            if (beat == ESCAPE_BEAT_BOARD)
            {
                TakeControl();

                if (hudScreen != nullptr)
                {
                    hudScreen->ShowNotice(ESCAPE_NOTICE);
                }
            }

            if (beat == ESCAPE_BEAT_LEAVE && fadeScreen != nullptr)
            {
                fadeScreen->FadeOut(ESCAPE_LEAVE_TIME);
            }
        });

        scene->SetHandler(ESCAPE_BEAT_DRIVE, [this, carObject](float deltaTime)
        {
            DriveEscape(carObject, deltaTime);
        });

        scene->SetHandler(ESCAPE_BEAT_LEAVE, [this, carObject](float deltaTime)
        {
            DriveEscape(carObject, deltaTime);
        });

        scene->SubscribeFinished([this]() { RequestNextLevel(); });
        scene->Play();
    }

    void DeveloperLevel::TakeControl()
    {
        if (player == nullptr)
        {
            return;
        }

        for (XYZEngine::Component* part : player->GetComponents<XYZEngine::Component>())
        {
            bool isDriving = dynamic_cast<XYZEngine::CameraComponent*>(part) != nullptr
                || dynamic_cast<XYZEngine::TransformComponent*>(part) != nullptr
                || dynamic_cast<PlayerHudBinderComponent*>(part) != nullptr;

            if (!isDriving)
            {
                part->SetEnabled(false);
            }
        }

        if (crosshair != nullptr)
        {
            crosshair->SetActive(false);
        }
    }

    void DeveloperLevel::DriveEscape(XYZEngine::GameObject* carObject, float deltaTime)
    {
        if (carObject == nullptr)
        {
            return;
        }

        escapeSpeed = std::min(ESCAPE_CAR_SPEED, escapeSpeed + ESCAPE_CAR_PICKUP * deltaTime);
        carObject->GetTransform()->MoveBy({escapeSpeed * deltaTime, 0.f});

        if (player != nullptr)
        {
            player->GetTransform()->SetWorldPosition(carObject->GetTransform()->GetWorldPosition());
        }
    }

    void DeveloperLevel::OnWavesCleared()
    {
        XYZEngine::GameObject* exitObject = level.GetExit();
        if (exitObject != nullptr)
        {
            auto exitComponent = exitObject->GetComponent<LevelExitComponent>();
            if (exitComponent != nullptr)
            {
                exitComponent->SetLocked(false);
            }

            auto renderer = exitObject->GetComponent<XYZEngine::RectangleRendererComponent>();
            if (renderer != nullptr)
            {
                renderer->SetColor(LEVEL_EXIT_COLOR);
            }
        }

        XYZEngine::GameObject* carObject = level.GetEscapeCar();
        if (carObject != nullptr)
        {
            auto car = carObject->GetComponent<EscapeCarComponent>();
            if (car != nullptr)
            {
                car->SetReady(true);
            }
        }

        if (hudScreen != nullptr)
        {
            hudScreen->ShowNotice(WAVES_DONE_NOTICE);
        }
    }

    void DeveloperLevel::OnBossDefeated()
    {
        XYZEngine::GameObject* exitObject = level.GetExit();
        if (exitObject != nullptr)
        {
            auto exitComponent = exitObject->GetComponent<LevelExitComponent>();
            if (exitComponent != nullptr)
            {
                exitComponent->SetLocked(false);
            }

            auto renderer = exitObject->GetComponent<XYZEngine::RectangleRendererComponent>();
            if (renderer != nullptr)
            {
                renderer->SetColor(LEVEL_EXIT_COLOR);
            }
        }

        XYZEngine::FrameClock::Instance()->SlowMotion(DEATH_TIME_SCALE, DEATH_SLOW_MOTION_TIME, DEATH_SLOW_MOTION_BLEND);
        Fx::ShakeCamera(CAMERA_SHAKE_HEAVY);

        if (hudScreen != nullptr)
        {
            hudScreen->ShowNotice(BOSS_DEFEATED_NOTICE);
        }

        LOG_INFO("Boss is defeated, the level exit is open");
    }

    void DeveloperLevel::RequestNextLevel()
    {
        const LevelCatalog& levels = GameResources::GetLevels();
        LevelStep step = ResolveNextLevel(levels, currentLevelIndex, level.GetInfo().nextLevelId);

        if (step.kind == LevelStepKind::Unknown)
        {
            LOG_ERROR("Unknown next level: " + level.GetInfo().nextLevelId);
            return;
        }

        if (step.kind == LevelStepKind::Finished)
        {
            LOG_INFO("No more levels, the run is complete");
            state = State::Victory;
            UpdateOverlay();
            return;
        }

        pendingLevelIndex = step.index;

        if (fadeScreen != nullptr)
        {
            fadeScreen->FadeOut(LEVEL_FADE_OUT_TIME);
        }
    }

    void DeveloperLevel::GoToPendingLevel()
    {
        int nextIndex = pendingLevelIndex;
        pendingLevelIndex = -1;

        const LevelEntry* entry = GameResources::GetLevels().GetAt(nextIndex);
        if (entry == nullptr)
        {
            return;
        }

        level.Clear();

        for (const char* temporaryName : {BLOOD_POOL_OBJECT_NAME, FX_OBJECT_NAME, PROJECTILE_OBJECT_NAME,
                                          ROCKET_OBJECT_NAME, CAST_MARK_OBJECT_NAME})
        {
            GameWorld::Instance()->DestroyGameObjects(temporaryName);
        }

        XYZEngine::ParticleSystem::Instance()->Clear();
        GameWorld::Instance()->LateUpdate();

        if (!LoadLevel(nextIndex))
        {
            return;
        }

        if (player != nullptr)
        {
            player->GetTransform()->SetWorldPosition(level.GetStartPosition());
        }

        SubscribeExit();
        SubscribeBoss();
        SubscribeWaves();
        SubscribeEscape();
        ShowLevelTitle();

        LOG_INFO("Level changed to " + entry->id + ", objects in world "
            + std::to_string(GameWorld::Instance()->GetObjectsCount()));
    }

    void DeveloperLevel::ShowLevelTitle()
    {
        if (hudScreen == nullptr)
        {
            return;
        }

        const LevelEntry* entry = GameResources::GetLevels().GetAt(currentLevelIndex);
        const std::string& title = level.GetInfo().title.empty() && entry != nullptr
            ? entry->title
            : level.GetInfo().title;

        if (!title.empty())
        {
            hudScreen->ShowNotice(title.c_str());
        }
    }

    void DeveloperLevel::Update(float deltaTime)
    {
        if (pendingLevelIndex >= 0 && (fadeScreen == nullptr || fadeScreen->IsCovered()))
        {
            GoToPendingLevel();

            if (fadeScreen != nullptr)
            {
                fadeScreen->FadeIn(LEVEL_FADE_IN_TIME);
            }
        }

        auto input = InputSystem::Instance();
        bool isPaused = Engine::Instance()->IsPaused();

        if (input->WasActionPressed(XYZEngine::InputAction::Pause))
        {
            SetPaused(!isPaused);
            return;
        }
        if (!input->HasFocus() && !isPaused)
        {
            SetPaused(true);
            return;
        }
        if (isPaused)
        {
            return;
        }

        if (state == State::PlayerDied)
        {
            gameOverDelay.Tick(deltaTime);
            if (gameOverDelay.IsReady())
            {
                ShowGameOver();
            }
        }
        else if ((state == State::GameOver || state == State::Victory) && input->WasKeyPressed(RESTART_KEY))
        {
            Restart();
        }
        else if (input->WasKeyPressed(DEBUG_HEAL_KEY))
        {
            auto health = GameWorld::Instance()->FindComponent<HealthComponent>(PLAYER_OBJECT_NAME);
            if (health != nullptr)
            {
                health->Heal(DEBUG_HEAL_AMOUNT);
            }
        }
    }

    void DeveloperLevel::Restart()
    {
        LOG_INFO("Level restarts");

        Stop();
        Start();
    }

    void DeveloperLevel::Stop()
    {
        LOG_INFO("Developer level is stopping");

        XYZEngine::FrameClock::Instance()->StopTimeEffects();

        for (auto sceneObject : {uiRoot, crosshair, music, player, particles})
        {
            if (sceneObject != nullptr)
            {
                GameWorld::Instance()->DestroyGameObject(sceneObject);
            }
        }
        uiRoot = nullptr;
        crosshair = nullptr;
        music = nullptr;
        player = nullptr;
        particles = nullptr;

        level.Clear();

        XYZEngine::UiManager::Instance()->Clear();
        GameWorld::Instance()->Clear();

        hudScreen.reset();
        inventoryScreen.reset();
        messageScreen.reset();
        fadeScreen.reset();
    }

    void DeveloperLevel::SetPaused(bool isPaused)
    {
        Engine::Instance()->SetPaused(isPaused);
        RenderSystem::Instance()->GetMainWindow().setMouseCursorVisible(isPaused);
        RenderSystem::Instance()->HoldMouse(!isPaused);

        if (music != nullptr)
        {
            auto musicPlayer = music->GetComponent<MusicComponent>();
            isPaused ? musicPlayer->Pause() : musicPlayer->Resume();
        }

        UpdateOverlay();

        LOG_INFO(isPaused ? "Game paused" : "Game resumed");
    }

    void DeveloperLevel::ShowGameOver()
    {
        state = State::GameOver;
        UpdateOverlay();

        LOG_INFO("Game over");
    }

    void DeveloperLevel::UpdateOverlay()
    {
        if (messageScreen == nullptr)
        {
            return;
        }

        if (Engine::Instance()->IsPaused())
        {
            messageScreen->Show(PAUSE_TITLE, PAUSE_HINT);
        }
        else if (state == State::GameOver)
        {
            messageScreen->Show(GAME_OVER_TITLE, GAME_OVER_HINT);
        }
        else if (state == State::Victory)
        {
            messageScreen->Show(VICTORY_TITLE, VICTORY_HINT);
        }
        else
        {
            messageScreen->Hide();
        }
    }
}
