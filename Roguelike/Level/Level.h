#pragma once

#include <array>
#include <optional>
#include <vector>
#include <GameObject.h>
#include <Vector.h>
#include "LevelData.h"

namespace RoguelikeGame
{
    /**
    *	Объекты, которые уровень помнит поимённо: их ищут снаружи, чтобы связать
    *	между собой - выход запирается боссом, машина ждёт конца волн.
    *	Новая роль добавляется сюда, и переносится при перемещении уровня сама.
    */
    enum class LevelRole
    {
        Exit,
        Boss,
        WaveDirector,
        Pursuit,
        EscapeCar,
        Count
    };

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

        void Set(LevelRole role, XYZEngine::GameObject* gameObject);
        XYZEngine::GameObject* Get(LevelRole role) const;

        void SetExit(XYZEngine::GameObject* exitObject);
        void SetBoss(XYZEngine::GameObject* bossObject);
        void SetWaveDirector(XYZEngine::GameObject* directorObject);
        void SetPursuit(XYZEngine::GameObject* pursuitObject);
        void SetEscapeCar(XYZEngine::GameObject* carObject);

        std::optional<XYZEngine::Vector2Df> GetPlayerSpawn() const;
        std::optional<XYZEngine::Vector2Df> GetEntrance() const;
        XYZEngine::Vector2Df GetStartPosition() const;
        const LevelInfo& GetInfo() const;
        XYZEngine::GameObject* GetExit() const;
        XYZEngine::GameObject* GetBoss() const;
        XYZEngine::GameObject* GetWaveDirector() const;
        XYZEngine::GameObject* GetPursuit() const;
        XYZEngine::GameObject* GetEscapeCar() const;
        std::size_t GetObjectsCount() const;

        void Clear();

    private:
        static constexpr std::size_t ROLES_COUNT = static_cast<std::size_t>(LevelRole::Count);

        using Roles = std::array<XYZEngine::GameObject*, ROLES_COUNT>;

        std::vector<XYZEngine::GameObject*> objects;
        std::optional<XYZEngine::Vector2Df> playerSpawn;
        std::optional<XYZEngine::Vector2Df> entrance;
        Roles roles{};
        LevelInfo info;

        void TakeFrom(Level& other) noexcept;
    };
}
