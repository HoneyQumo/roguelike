#include "Level.h"
#include <GameWorld.h>

namespace RoguelikeGame
{
    Level::~Level()
    {
        Clear();
    }

    Level::Level(Level&& other) noexcept
        : objects(std::move(other.objects)), playerSpawn(other.playerSpawn), entrance(other.entrance),
          exitObject(other.exitObject), bossObject(other.bossObject), info(std::move(other.info))
    {
        other.objects.clear();
        other.playerSpawn.reset();
        other.entrance.reset();
        other.exitObject = nullptr;
        other.bossObject = nullptr;
    }

    Level& Level::operator=(Level&& other) noexcept
    {
        if (this != &other)
        {
            Clear();
            objects = std::move(other.objects);
            playerSpawn = other.playerSpawn;
            entrance = other.entrance;
            exitObject = other.exitObject;
            bossObject = other.bossObject;
            info = std::move(other.info);
            other.objects.clear();
            other.playerSpawn.reset();
            other.entrance.reset();
            other.exitObject = nullptr;
            other.bossObject = nullptr;
        }

        return *this;
    }

    bool Level::Add(XYZEngine::GameObject* gameObject)
    {
        if (gameObject == nullptr)
        {
            return false;
        }

        objects.push_back(gameObject);
        return true;
    }

    void Level::SetPlayerSpawn(const XYZEngine::Vector2Df& position)
    {
        playerSpawn = position;
    }

    void Level::SetEntrance(const XYZEngine::Vector2Df& position)
    {
        entrance = position;
    }

    void Level::SetInfo(const LevelInfo& newInfo)
    {
        info = newInfo;
    }

    void Level::SetExit(XYZEngine::GameObject* newExitObject)
    {
        exitObject = newExitObject;
    }

    void Level::SetBoss(XYZEngine::GameObject* newBossObject)
    {
        bossObject = newBossObject;
    }

    void Level::SetWaveDirector(XYZEngine::GameObject* newDirectorObject)
    {
        directorObject = newDirectorObject;
    }

    XYZEngine::GameObject* Level::GetWaveDirector() const
    {
        return directorObject;
    }

    XYZEngine::GameObject* Level::GetEscapeCar() const
    {
        return carObject;
    }

    void Level::SetEscapeCar(XYZEngine::GameObject* newCarObject)
    {
        carObject = newCarObject;
    }

    std::optional<XYZEngine::Vector2Df> Level::GetEntrance() const
    {
        return entrance;
    }

    XYZEngine::Vector2Df Level::GetStartPosition() const
    {
        if (entrance.has_value())
        {
            return *entrance;
        }

        return playerSpawn.value_or(XYZEngine::Vector2Df{0.f, 0.f});
    }

    const LevelInfo& Level::GetInfo() const
    {
        return info;
    }

    XYZEngine::GameObject* Level::GetExit() const
    {
        return exitObject;
    }

    XYZEngine::GameObject* Level::GetBoss() const
    {
        return bossObject;
    }

    std::optional<XYZEngine::Vector2Df> Level::GetPlayerSpawn() const
    {
        return playerSpawn;
    }

    std::size_t Level::GetObjectsCount() const
    {
        return objects.size();
    }

    void Level::Clear()
    {
        for (auto gameObject : objects)
        {
            XYZEngine::GameWorld::Instance()->DestroyGameObject(gameObject);
        }

        objects.clear();
        playerSpawn.reset();
        entrance.reset();
        exitObject = nullptr;
        bossObject = nullptr;
        info = LevelInfo();
    }
}
