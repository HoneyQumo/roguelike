#include "Level.h"
#include <GameWorld.h>

namespace RoguelikeGame
{
    Level::~Level()
    {
        Clear();
    }

    Level::Level(Level&& other) noexcept
    {
        TakeFrom(other);
    }

    Level& Level::operator=(Level&& other) noexcept
    {
        if (this != &other)
        {
            Clear();
            TakeFrom(other);
        }

        return *this;
    }

    // Забирает всё и оставляет источник пустым: забытое здесь поле тихо теряется при смене уровня.
    void Level::TakeFrom(Level& other) noexcept
    {
        objects = std::move(other.objects);
        playerSpawn = other.playerSpawn;
        entrance = other.entrance;
        roles = other.roles;
        info = std::move(other.info);

        other.objects.clear();
        other.playerSpawn.reset();
        other.entrance.reset();
        other.roles.fill(nullptr);
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

    void Level::Set(LevelRole role, XYZEngine::GameObject* gameObject)
    {
        if (role == LevelRole::Count)
        {
            return;
        }

        roles[static_cast<std::size_t>(role)] = gameObject;
    }

    XYZEngine::GameObject* Level::Get(LevelRole role) const
    {
        return role == LevelRole::Count ? nullptr : roles[static_cast<std::size_t>(role)];
    }

    void Level::SetExit(XYZEngine::GameObject* newExitObject)
    {
        Set(LevelRole::Exit, newExitObject);
    }

    void Level::SetBoss(XYZEngine::GameObject* newBossObject)
    {
        Set(LevelRole::Boss, newBossObject);
    }

    void Level::SetWaveDirector(XYZEngine::GameObject* newDirectorObject)
    {
        Set(LevelRole::WaveDirector, newDirectorObject);
    }

    void Level::SetPursuit(XYZEngine::GameObject* newPursuitObject)
    {
        Set(LevelRole::Pursuit, newPursuitObject);
    }

    void Level::SetEscapeCar(XYZEngine::GameObject* newCarObject)
    {
        Set(LevelRole::EscapeCar, newCarObject);
    }

    XYZEngine::GameObject* Level::GetExit() const
    {
        return Get(LevelRole::Exit);
    }

    XYZEngine::GameObject* Level::GetBoss() const
    {
        return Get(LevelRole::Boss);
    }

    XYZEngine::GameObject* Level::GetWaveDirector() const
    {
        return Get(LevelRole::WaveDirector);
    }

    XYZEngine::GameObject* Level::GetPursuit() const
    {
        return Get(LevelRole::Pursuit);
    }

    XYZEngine::GameObject* Level::GetEscapeCar() const
    {
        return Get(LevelRole::EscapeCar);
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

    std::optional<XYZEngine::Vector2Df> Level::GetPlayerSpawn() const
    {
        return playerSpawn;
    }

    const std::vector<XYZEngine::GameObject*>& Level::GetObjects() const
    {
        return objects;
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
        roles.fill(nullptr);
        info = LevelInfo();
    }
}
