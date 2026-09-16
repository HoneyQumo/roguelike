#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include "Component.h"
#include "Collision.h"
#include "EventList.h"
#include "Trigger.h"
#include "PhysicsSystem.h"
#include "Vector.h"

namespace XYZEngine
{
	class RigidbodyComponent;

	constexpr unsigned int DEFAULT_COLLISION_LAYER = 1u;

	class ColliderComponent : public Component
	{
	public:
		ColliderComponent(GameObject* gameObject);

		void Start() override;
		virtual void Update(float deltaTime) = 0;
		virtual void Render() = 0;

		void SetTrigger(bool newIsTrigger);
		bool IsTrigger() const;

		const sf::FloatRect& GetBounds() const;
		RigidbodyComponent* GetBody();

		void SetCollisionLayer(unsigned int newCollisionLayer);
		unsigned int GetCollisionLayer() const;

		void SetIgnoredLayers(unsigned int newIgnoredLayers);
		unsigned int GetIgnoredLayers() const;

		SubscriptionId SubscribeCollision(std::function<void(const Collision&)> onCollisionAction);
		void UnsubscribeCollision(SubscriptionId subscription);

		SubscriptionId SubscribeTriggerEnter(std::function<void(const Trigger&)> onTriggerEnterAction);
		void UnsubscribeTriggerEnter(SubscriptionId subscription);

		SubscriptionId SubscribeTriggerExit(std::function<void(const Trigger&)> onTriggerExitAction);
		void UnsubscribeTriggerExit(SubscriptionId subscription);

		friend class PhysicsSystem;

	protected:
		void SetBounds(const sf::FloatRect& newBounds);

		sf::FloatRect bounds;
		sf::FloatRect gridBounds;
		Vector2Df lastPlace = {0.f, 0.f};
		bool isInGrid = false;
		unsigned int order = 0u;
		RigidbodyComponent* body = nullptr;
		bool isBodyFound = false;
		bool isTrigger = false;
		unsigned int collisionLayer = DEFAULT_COLLISION_LAYER;
		unsigned int ignoredLayers = 0u;

		void OnCollision(const Collision& collision);
		void OnTriggerEnter(const Trigger& trigger);
		void OnTriggerExit(const Trigger& trigger);

		EventList<const Collision&> collisionEvent;
		EventList<const Trigger&> triggerEnterEvent;
		EventList<const Trigger&> triggerExitEvent;
	};
}