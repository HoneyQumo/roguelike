#pragma once

#include <string>
#include <GameObject.h>
#include <GameWorld.h>
#include "FogVisibilityComponent.h"

namespace RoguelikeGame
{
    /**
    *	Объект-эффект: кровь, огонь, вспышка, снаряд, метка приёма босса.
    *
    *	У всех них два общих свойства, и оба легко забыть по отдельности.
    *	Эффект принадлежит локации и уезжает вместе с ней - это уже было, и
    *	забытый флаг однажды перевёз огонь на следующую карту. Эффект прячется
    *	туманом, как всё остальное на карте, - а вот это забыли у всех пяти сразу,
    *	и горящая за стеной бочка светила сквозь неразведанную стену.
    *
    *	Новый эффект рождается здесь и получает оба свойства даром.
    *
    *	Прятание гасит только отрисовку: снаряд в тумане продолжает лететь и бить.
    */
    inline XYZEngine::GameObject* CreateEffectObject(const std::string& name, int renderLayer)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(name);
        gameObject->SetTemporary(true);
        gameObject->SetRenderLayer(renderLayer);
        HideInFog(gameObject);

        return gameObject;
    }
}
