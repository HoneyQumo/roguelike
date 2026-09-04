#pragma once

#include <functional>
#include <memory>
#include <string>
#include <SFML/Graphics/Color.hpp>
#include "Weapon.h"
#include "FactionComponent.h"
#include "HitFlashComponent.h"
#include <GameObject.h>
#include <TransformComponent.h>
#include <SpriteRendererComponent.h>
#include <MovementComponent.h>
#include <BoxColliderComponent.h>
#include <AimRotationComponent.h>
#include <SpriteMovementAnimationComponent.h>
#include "HealthComponent.h"
#include <AudioComponent.h>

namespace RoguelikeGame
{
    struct CharacterSpec
    {
        std::string objectName;
        std::string textureMapName;
        XYZEngine::Vector2Df position;
        unsigned int collisionLayer = 0u;
        float speed = 0.f;
        float maxHealth = 0.f;
        float armor = 0.f;
        Faction faction = Faction::Neutral;
        WeaponId weapon = WeaponId::Knife;
        sf::Color healthBarColor;
    };

    struct CharacterParts
    {
        XYZEngine::GameObject* gameObject = nullptr;
        XYZEngine::TransformComponent* transform = nullptr;
        XYZEngine::SpriteRendererComponent* renderer = nullptr;
        XYZEngine::MovementComponent* movement = nullptr;
        XYZEngine::BoxColliderComponent* collider = nullptr;
        XYZEngine::AimRotationComponent* aim = nullptr;
        XYZEngine::SpriteMovementAnimationComponent* animation = nullptr;
        HealthComponent* health = nullptr;
        XYZEngine::AudioComponent* hurtAudio = nullptr;
        HitFlashComponent* hitFlash = nullptr;
        WeaponLayerComponent* weapon = nullptr;
    };

    using AddControls = std::function<void(XYZEngine::GameObject*)>;

    CharacterParts CreateCharacter(const CharacterSpec& spec, const AddControls& addControls);
}
