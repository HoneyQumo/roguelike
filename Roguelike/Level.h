#pragma once

#include <optional>
#include <vector>
#include <GameObject.h>
#include <Vector.h>

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

        void Add(XYZEngine::GameObject* gameObject);
        void SetPlayerSpawn(const XYZEngine::Vector2Df& position);

        std::optional<XYZEngine::Vector2Df> GetPlayerSpawn() const;
        std::size_t GetObjectsCount() const;

        void Clear();

    private:
        std::vector<XYZEngine::GameObject*> objects;
        std::optional<XYZEngine::Vector2Df> playerSpawn;
    };
}
