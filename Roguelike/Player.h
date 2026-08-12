#pragma once

#include <memory>
#include "CameraComponent.h"
#include "GameWorld.h"
#include "SpriteRendererComponent.h"
#include "RenderSystem.h"
#include "InputComponent.h"
#include "GameObject.h"
#include "Vector.h"
#include "Weapon.h"

namespace RoguelikeGame
{
    class Player
    {
    public:
        Player(const XYZEngine::Vector2Df& position);
        XYZEngine::GameObject* GetGameObject();

    private:
        XYZEngine::GameObject* gameObject;
        std::unique_ptr<Weapon> weapon;
    };
}
