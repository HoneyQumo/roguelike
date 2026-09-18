#pragma once

#include <Component.h>
#include <GameObject.h>

namespace RoguelikeGame
{
    /**
    *	Сумкой можно пользоваться, только пока её хозяин в игре.
    *
    *	Экран зовёт InventoryComponent напрямую, минуя GameWorld, поэтому ни
    *	выключенный компонент, ни выключенный объект сами по себе его не останавливают:
    *	в катсцене побега игрока замораживают, а лечиться из сумки всё равно можно.
    */
    inline bool CanUseInventory(const XYZEngine::Component* inventory)
    {
        return inventory != nullptr && inventory->IsEnabled() && inventory->GetGameObject() != nullptr
            && inventory->GetGameObject()->IsActive();
    }
}
