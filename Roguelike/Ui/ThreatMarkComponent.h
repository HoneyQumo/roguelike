#pragma once

#include <vector>
#include <SFML/Graphics/Text.hpp>
#include <Component.h>
#include "ThreatPlacement.h"

namespace RoguelikeGame
{
    /**
    *	Рисует знаки угрозы в мировых координатах: «?» над встревоженным,
    *	«!» над тем, кто уже ведёт игрока.
    *
    *	Знаки нужны ровно для тех, кого не видно: над видимым противником и так
    *	висит шкала осведомлённости, и второй указатель был бы лишним.
    */
    class ThreatMarkComponent : public XYZEngine::Component
    {
    public:
        ThreatMarkComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override {}
        void Render() override;

        void SetMarks(const std::vector<ThreatMark>& newMarks);

        const std::vector<ThreatMark>& GetMarks() const;
        static const char* GlyphOf(ThreatKind kind);
        static sf::Color ColorOf(ThreatKind kind, float strength);

    private:
        std::vector<ThreatMark> marks;
        sf::Text label;
        bool hasFont = false;
    };
}
