#pragma once

#include <optional>
#include <vector>
#include <GameObject.h>
#include <Vector.h>
#include "LevelData.h"

namespace RoguelikeGame
{
    class Level
    {
    public:
        Level() = default;
        ~Level();

        Level(const Level&) = delete;
        Level& operator=(const Level&) = delete;
        Level(Level&& other) noexcept;
        Level& operator=(Level&& other) noexcept;

        bool Add(XYZEngine::GameObject* gameObject);
        void SetPlayerSpawn(const XYZEngine::Vector2Df& position);
        void SetEntrance(const XYZEngine::Vector2Df& position);
        void SetInfo(const LevelInfo& newInfo);
        void SetExit(XYZEngine::GameObject* exitObject);

        std::optional<XYZEngine::Vector2Df> GetPlayerSpawn() const;
        std::optional<XYZEngine::Vector2Df> GetEntrance() const;
        XYZEngine::Vector2Df GetStartPosition() const;
        const LevelInfo& GetInfo() const;
        XYZEngine::GameObject* GetExit() const;
        std::size_t GetObjectsCount() const;

        void Clear();

    private:
        std::vector<XYZEngine::GameObject*> objects;
        std::optional<XYZEngine::Vector2Df> playerSpawn;
        std::optional<XYZEngine::Vector2Df> entrance;
        XYZEngine::GameObject* exitObject = nullptr;
        LevelInfo info;
    };
}
