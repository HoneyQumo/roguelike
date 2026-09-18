#pragma once

#include <Component.h>
#include <Engine.h>
#include <GameObject.h>

namespace RoguelikeGame
{
    /**
    *	Сумкой можно пользоваться, только пока её хозяин в игре, а игра идёт.
    *
    *	Экран зовёт InventoryComponent напрямую, минуя GameWorld, поэтому ни заморозка
    *	в катсцене, ни пауза сами по себе его не останавливают. Проверка здесь,
    *	а не в экране: тем же путём ходят пояс и сброс предмета.
    */
    inline bool CanUseInventory(const XYZEngine::Component* inventory)
    {
        return inventory != nullptr && !XYZEngine::Engine::Instance()->IsPaused()
            && inventory->IsEnabled() && inventory->GetGameObject() != nullptr
            && inventory->GetGameObject()->IsActive();
    }
}
