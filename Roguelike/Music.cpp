#include "Music.h"
#include <GameWorld.h>
#include <MusicComponent.h>
#include <ResourceSystem.h>

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateMusic(const std::string& musicName, float volume)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Music");

        auto musicPlayer = gameObject->AddComponent<XYZEngine::MusicComponent>();
        musicPlayer->SetMusic(XYZEngine::ResourceSystem::Instance()->GetMusic(musicName));
        musicPlayer->SetVolume(volume);
        musicPlayer->SetLoop(true);
        musicPlayer->Play();

        return gameObject;
    }
}
