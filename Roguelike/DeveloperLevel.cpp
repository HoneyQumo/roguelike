#include "DeveloperLevel.h"
#include "GameSettings.h"
#include "LevelBuilder.h"
#include "LevelLoader.h"
#include "Player.h"
#include "Music.h"
#include "Crosshair.h"
#include "AmmoHud.h"
#include "MessageOverlay.h"
#include "MessageOverlayComponent.h"
#include <Engine.h>
#include <GameWorld.h>
#include <InputSystem.h>
#include <RenderSystem.h>
#include <MusicComponent.h>
#include <LoggerRegistry.h>

using namespace XYZEngine;

namespace RoguelikeGame
{
    void DeveloperLevel::Start()
    {
        LOG_INFO("Developer level is starting");

        try
        {
            level = LevelBuilder::Build(LevelLoader::Load(TEST_LEVEL_FILE));
        }
        catch (const std::exception& exception)
        {
            LOG_ERROR(std::string("Level is not loaded: ") + exception.what());
            LOG_WARN("Game continues with an empty level");
        }

        auto playerSpawn = level.GetPlayerSpawn();
        if (playerSpawn.has_value())
        {
            try
            {
                player = CreatePlayer(*playerSpawn);
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

        ammoHud = CreateAmmoHud();
        messageOverlay = CreateMessageOverlay();
    }

    void DeveloperLevel::Update(float deltaTime)
    {
        auto input = InputSystem::Instance();
        bool isPaused = Engine::Instance()->IsPaused();

        if (input->WasKeyPressed(sf::Keyboard::Escape))
        {
            SetPaused(!isPaused);
        }
        else if (!input->HasFocus() && !isPaused)
        {
            SetPaused(true);
        }
    }

    void DeveloperLevel::Restart()
    {
        Stop();
        Start();
    }

    void DeveloperLevel::Stop()
    {
        LOG_INFO("Developer level is stopping");

        for (auto sceneObject : {messageOverlay, ammoHud, crosshair, music, player})
        {
            if (sceneObject != nullptr)
            {
                GameWorld::Instance()->DestroyGameObject(sceneObject);
            }
        }
        messageOverlay = nullptr;
        ammoHud = nullptr;
        crosshair = nullptr;
        music = nullptr;
        player = nullptr;

        level.Clear();

        GameWorld::Instance()->Clear();
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

        if (messageOverlay != nullptr)
        {
            auto overlay = messageOverlay->GetComponent<MessageOverlayComponent>();
            isPaused ? overlay->Show(PAUSE_TITLE, PAUSE_HINT) : overlay->Hide();
        }

        LOG_INFO(isPaused ? "Game paused" : "Game resumed");
    }
}
