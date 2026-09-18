#pragma once

#include <Component.h>
#include <Engine.h>
#include <GameObject.h>

namespace RoguelikeGame
{
    /**
    *	Сумкой можно пользоваться, только пока её хозяин в игре, а игра идёт.
    *
    *	Экран зовёт InventoryComponent напрямую, минуя GameWorld, поэтому ни
    *	выключенный компонент, ни выключенный объект сами по себе его не останавливают:
    *	в катсцене побега игрока замораживают, а лечиться из сумки всё равно можно.
    *
    *	Пауза - тот же вопрос, только снаружи. Слой интерфейса обновляется до проверки
    *	паузы, иначе замерли бы оверлей и затемнение, - и вместе с ними крутится экран
    *	сумки. Лечиться, экипироваться и выбрасывать вещи в остановленный мир нельзя.
    *
    *	Проверка здесь, а не в экране: тем же путём ходят пояс и сброс предмета.
    */
    inline bool CanUseInventory(const XYZEngine::Component* inventory)
    {
        return inventory != nullptr && !XYZEngine::Engine::Instance()->IsPaused()
            && inventory->IsEnabled() && inventory->GetGameObject() != nullptr
            && inventory->GetGameObject()->IsActive();
    }
}
