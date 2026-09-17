#pragma once

#include <vector>
#include <Component.h>
#include <GameObject.h>
#include <GameWorld.h>
#include <RectangleRendererComponent.h>
#include <SpriteRendererComponent.h>
#include <TransformComponent.h>

namespace RoguelikeGame
{
    /**
    *	Выключенный компонент вместе с тем, кому он принадлежит.
    *
    *	Хозяин нужен, чтобы узнать, дожил ли компонент до разморозки: физика
    *	раздаёт столкновения не глядя на выключенность, и замороженная пуля,
    *	которую сбили за время сцены, успевает себя уничтожить.
    */
    struct FrozenPart
    {
        XYZEngine::GameObject* owner = nullptr;
        XYZEngine::Component* part = nullptr;
    };

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
    inline void Freeze(XYZEngine::GameObject* object, std::vector<FrozenPart>& frozen)
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
            frozen.push_back({object, part});
        }
    }

    inline void Thaw(std::vector<FrozenPart>& frozen)
    {
        for (const FrozenPart& frozenPart : frozen)
        {
            // Объекта нет - нет и компонента: трогать его значит писать в чужую память.
            if (frozenPart.part == nullptr || !XYZEngine::GameWorld::Instance()->Contains(frozenPart.owner))
            {
                continue;
            }

            frozenPart.part->SetEnabled(true);
        }

        frozen.clear();
    }
}
