#pragma once

#include <vector>
#include <Component.h>
#include <GameObject.h>
#include <RectangleRendererComponent.h>
#include <SpriteRendererComponent.h>
#include <TransformComponent.h>

namespace RoguelikeGame
{
    // Что нельзя выключать: без этого объект пропадёт с экрана, а не замрёт.
    inline bool KeepsDrawing(XYZEngine::Component* part)
    {
        return dynamic_cast<XYZEngine::SpriteRendererComponent*>(part) != nullptr
            || dynamic_cast<XYZEngine::RectangleRendererComponent*>(part) != nullptr
            || dynamic_cast<XYZEngine::TransformComponent*>(part) != nullptr;
    }

    /**
    *	Замораживает объект: выключает всё, кроме того, что рисует.
    *
    *	Выключенное запоминается, и вернуть надо будет ровно его. Слепо включить
    *	всё обратно нельзя: у босса, например, есть свой хозяин над компонентом
    *	преследования, и он сам решает, когда тот работает.
    */
    inline void Freeze(XYZEngine::GameObject* object, std::vector<XYZEngine::Component*>& frozen)
    {
        if (object == nullptr)
        {
            return;
        }

        for (XYZEngine::Component* part : object->GetComponents<XYZEngine::Component>())
        {
            if (part == nullptr || KeepsDrawing(part) || !part->IsEnabled())
            {
                continue;
            }

            part->SetEnabled(false);
            frozen.push_back(part);
        }
    }

    inline void Thaw(std::vector<XYZEngine::Component*>& frozen)
    {
        for (XYZEngine::Component* part : frozen)
        {
            if (part != nullptr)
            {
                part->SetEnabled(true);
            }
        }

        frozen.clear();
    }
}
