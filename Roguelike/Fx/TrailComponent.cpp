#include "TrailComponent.h"
#include <algorithm>
#include <GameObject.h>
#include <VertexArrayRendererComponent.h>

using namespace XYZEngine;

namespace RoguelikeGame
{
    TrailComponent::TrailComponent(GameObject* gameObject) : Component(gameObject) {}

    void TrailComponent::Start()
    {
        ribbon = gameObject->GetComponent<VertexArrayRendererComponent>();
        if (ribbon == nullptr)
        {
            ribbon = gameObject->AddComponent<VertexArrayRendererComponent>();
        }
    }

    void TrailComponent::Update(float deltaTime)
    {
        if (isDirty)
        {
            Rebuild();
        }
    }

    void TrailComponent::Render()
    {
    }

    void TrailComponent::SetColour(const sf::Color& newColour)
    {
        colour = newColour;
        isDirty = true;
    }

    void TrailComponent::SetWidth(float newWidth)
    {
        width = newWidth;
        isDirty = true;
    }

    void TrailComponent::SetStep(float newStep)
    {
        step = newStep;
    }

    bool TrailComponent::Add(const Vector2Df& place, float alpha)
    {
        if (!points.empty() && (place - points.back().place).GetLength() < step)
        {
            return false;
        }

        points.push_back({place, width, alpha});
        isDirty = true;

        return true;
    }

    void TrailComponent::Cut()
    {
        points.clear();
        isDirty = true;
    }

    std::size_t TrailComponent::GetPointsCount() const
    {
        return points.size();
    }

    const std::vector<TrailPoint>& TrailComponent::GetPoints() const
    {
        return points;
    }

    void TrailComponent::Rebuild()
    {
        isDirty = false;

        if (ribbon == nullptr)
        {
            return;
        }

        ribbon->Clear();

        for (const TrailQuad& quad : BuildTrail(points))
        {
            sf::Color tint = colour;
            tint.a = static_cast<sf::Uint8>(colour.a * std::max(0.f, std::min(1.f, quad.alpha)));

            ribbon->AddQuad(quad.corners[0], quad.corners[1], quad.corners[2], quad.corners[3], tint);
        }
    }
}
