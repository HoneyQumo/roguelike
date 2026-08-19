#include "Music.h"
#include <ResourceSystem.h>

namespace RoguelikeGame
{
    Music::Music(const std::string& musicName, float volume)
    {
        gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Music");

        auto musicPlayer = gameObject->AddComponent<XYZEngine::MusicComponent>();
        musicPlayer->SetMusic(XYZEngine::ResourceSystem::Instance()->GetMusic(musicName));
        musicPlayer->SetVolume(volume);
        musicPlayer->SetLoop(true);
        musicPlayer->Play();
    }

    XYZEngine::GameObject* Music::GetGameObject()
    {
        return gameObject;
    }
}
