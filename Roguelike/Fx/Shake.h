#pragma once

#include <CameraComponent.h>
#include <GameWorld.h>

namespace RoguelikeGame
{
    /**
    *	Трясёт камеру, где бы она ни висела.
    *
    *	Искать её по имени владельца нельзя: камера уже переезжала с игрока на
    *	свой объект, и поиск по имени тогда молча перестал находить хоть что-то.
    *	По типу она находится всегда.
    */
    inline void ShakeEveryCamera(const XYZEngine::CameraShake& shake)
    {
        for (XYZEngine::CameraComponent* camera : XYZEngine::GameWorld::Instance()->FindComponents<XYZEngine::CameraComponent>())
        {
            if (camera != nullptr)
            {
                camera->Shake(shake);
            }
        }
    }
}
