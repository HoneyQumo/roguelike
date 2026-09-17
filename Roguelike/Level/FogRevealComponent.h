#pragma once

#include <string>
#include <Component.h>

namespace RoguelikeGame
{
    /**
    *	Открывает туман вокруг игрока.
    *
    *	Живёт отдельным объектом уровня, а не на самом игроке: в катсценах игрока
    *	замораживают, выключая ему все компоненты, и туман бы застыл вместе с ним.
    */
    class FogRevealComponent : public XYZEngine::Component
    {
    public:
        FogRevealComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override {}

        void SetTargetName(const std::string& newTargetName);

        int GetRevealCount() const;

    private:
        std::string targetName;
        XYZEngine::GameObject* target = nullptr;

        bool hasCell = false;
        int column = 0;
        int row = 0;
        unsigned int gridVersion = 0u;
        int revealCount = 0;
    };
}
