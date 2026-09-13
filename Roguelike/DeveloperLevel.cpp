#include "DeveloperLevel.h"
#include "GameSettings.h"
#include "GameResources.h"
#include "LevelBuilder.h"
#include "LevelLoader.h"
#include "Player.h"
#include "Music.h"
#include "Crosshair.h"
#include "Particles.h"
#include "UiRoot.h"
#include <Engine.h>
#include <GameWorld.h>
#include <UiManager.h>
#include <FrameClock.h>
#include <InputSystem.h>
#include <RenderSystem.h>
#include <MusicComponent.h>
#include "HealthComponent.h"
#include <LoggerRegistry.h>

using namespace XYZEngine;

namespace RoguelikeGame
{
    void DeveloperLevel::Start()
    {
        LOG_INFO("Developer level is starting");

        state = State::Playing;
        gameOverDelay.Stop();

        try
        {
            const LevelEntry* entry = GameResources::GetLevels().GetFirst();
            std::string levelFile = entry != nullptr ? entry->filePath : TEST_LEVEL_FILE;

            level = LevelBuilder::Build(LevelLoader::Load(levelFile), GameResources::GetItems());
        }
        catch (const std::exception& exception)
        {
            LOG_ERROR(std::string("Level is not loaded: ") + exception.what());
            LOG_WARN("Game continues with an empty level");
        }

        particles = CreateParticles();

        auto playerSpawn = level.GetPlayerSpawn();
        if (playerSpawn.has_value())
        {
            try
            {
                player = CreatePlayer(*playerSpawn);

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
        else
        {
            LOG_WARN("Level has no player spawn point");
        }

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
        uiRoot = CreateUiRoot(*hudScreen, *inventoryScreen, *messageScreen);
    }

    void DeveloperLevel::Update(float deltaTime)
    {
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
        else if (state == State::GameOver && input->WasKeyPressed(RESTART_KEY))
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
    }

    void DeveloperLevel::SetPaused(bool isPaused)
    {
        Engine::Instance()->SetPaused(isPaused);
        RenderSystem::Instance()->GetMainWindow().setMouseCursorVisible(isPaused);

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
        else
        {
            messageScreen->Hide();
        }
    }
}
