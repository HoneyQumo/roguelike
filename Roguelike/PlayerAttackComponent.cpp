#include "PlayerAttackComponent.h"
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <cmath>

namespace RoguelikeGame
{
	const float RADIANS_TO_DEGREES = 57.29578f;

	PlayerAttackComponent::PlayerAttackComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
	{
		transform = gameObject->GetComponent<XYZEngine::TransformComponent>();
	}

	void PlayerAttackComponent::Update(float deltaTime)
	{
		if (input == nullptr)
		{
			input = gameObject->GetComponent<XYZEngine::InputComponent>();
		}
		if (weapon == nullptr)
		{
			weapon = gameObject->GetComponent<XYZEngine::WeaponComponent>();
		}
		if (health == nullptr)
		{
			health = gameObject->GetComponent<XYZEngine::HealthComponent>();
		}

		if (input == nullptr || weapon == nullptr)
		{
			LOG_ERROR("Player attack needs input and weapon components");
			gameObject->RemoveComponent(this);
			return;
		}

		if (health != nullptr && !health->IsAlive())
		{
			return;
		}

		XYZEngine::Vector2Df aimDirection = input->GetMouseWorldPosition() - transform->GetWorldPosition();
		AimWeapon(aimDirection);

		if (input->IsAttackPressed())
		{
			weapon->TryShoot(aimDirection);
		}
	}
	void PlayerAttackComponent::Render()
	{

	}

	void PlayerAttackComponent::SetWeapon(XYZEngine::TransformComponent* newWeaponTransform, XYZEngine::SpriteRendererComponent* newWeaponRenderer)
	{
		weaponTransform = newWeaponTransform;
		weaponRenderer = newWeaponRenderer;
	}

	void PlayerAttackComponent::AimWeapon(const XYZEngine::Vector2Df& direction)
	{
		if (weaponTransform == nullptr || direction.GetLength() <= 0.f)
		{
			return;
		}

		float angle = std::atan2(direction.y, direction.x) * RADIANS_TO_DEGREES;
		weaponTransform->SetWorldRotation(angle);

		if (weaponRenderer != nullptr)
		{
			weaponRenderer->FlipY(direction.x < 0.f);
		}
	}
}
