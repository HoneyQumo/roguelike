#pragma once

#include <Component.h>
#include <GameObject.h>
#include "FogOfWar.h"

namespace RoguelikeGame
{
    /**
    *	Прячет своего хозяина, пока его клетка не видна игроку.
    *
    *	Память уровня хранит только геометрию: враг или ящик в разведанной комнате
    *	мог с тех пор уйти или сгореть, и рисовать его по памяти - врать игроку.
    *
    *	Прячется весь объект, а не его рисовалка: выключенную рисовалку хозяин
    *	объекта может включить по своим делам, и подобранный предмет всплыл бы обратно.
    */
    class FogVisibilityComponent : public XYZEngine::Component
    {
    public:
        FogVisibilityComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override {}

        bool IsHidden() const;

    private:
        bool isHidden = false;
    };

    // На локации без тумана компонента нет вовсе, а не просто нечего делать каждый кадр.
    inline void HideInFog(XYZEngine::GameObject* object)
    {
        if (object != nullptr && FogOfWar::Current().IsEnabled())
        {
            object->AddComponent<FogVisibilityComponent>();
        }
    }
}
