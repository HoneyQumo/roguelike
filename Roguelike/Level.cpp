#include "Level.h"
#include <GameWorld.h>

namespace RoguelikeGame
{
    Level::~Level()
    {
        Clear();
    }

    Level::Level(Level&& other) noexcept
        : objects(std::move(other.objects)), playerSpawn(other.playerSpawn)
    {
        other.objects.clear();
        other.playerSpawn.reset();
    }

    Level& Level::operator=(Level&& other) noexcept
    {
        if (this != &other)
        {
            Clear();
            objects = std::move(other.objects);
            playerSpawn = other.playerSpawn;
            other.objects.clear();
            other.playerSpawn.reset();
        }

        return *this;
    }

    void Level::Add(XYZEngine::GameObject* gameObject)
    {
        if (gameObject != nullptr)
        {
            objects.push_back(gameObject);
        }
    }

    void Level::SetPlayerSpawn(const XYZEngine::Vector2Df& position)
    {
        playerSpawn = position;
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
    }
}
