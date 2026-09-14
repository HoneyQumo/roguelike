#include "pch.h"
#include "ColliderComponent.h"
#include "GameObject.h"
#include "RigidbodyComponent.h"

namespace XYZEngine
{
	ColliderComponent::ColliderComponent(GameObject* gameObject) : Component(gameObject) 
	{ 
		
	}

	void ColliderComponent::Start()
	{
		GetBody();
	}

	// Тело ищется один раз: состав компонентов объекта после создания не меняется.
	RigidbodyComponent* ColliderComponent::GetBody()
	{
		if (!isBodyFound)
		{
			body = gameObject->GetComponent<RigidbodyComponent>();
			isBodyFound = true;
		}

		return body;
	}

	void ColliderComponent::SetTrigger(bool newIsTrigger)
	{
		isTrigger = newIsTrigger;
	}
	bool ColliderComponent::IsTrigger() const
	{
		return isTrigger;
	}

	const sf::FloatRect& ColliderComponent::GetBounds() const
	{
		return bounds;
	}

	void ColliderComponent::SetBounds(const sf::FloatRect& newBounds)
	{
		bounds = newBounds;
		PhysicsSystem::Instance()->OnBoundsChanged(this);
	}

	void ColliderComponent::SetCollisionLayer(unsigned int newCollisionLayer)
	{
		collisionLayer = newCollisionLayer;
	}
	unsigned int ColliderComponent::GetCollisionLayer() const
	{
		return collisionLayer;
	}

	void ColliderComponent::SetIgnoredLayers(unsigned int newIgnoredLayers)
	{
		ignoredLayers = newIgnoredLayers;
	}
	unsigned int ColliderComponent::GetIgnoredLayers() const
	{
		return ignoredLayers;
	}

	SubscriptionId ColliderComponent::SubscribeCollision(std::function<void(const Collision&)> onCollisionAction)
	{
		return collisionEvent.Subscribe(std::move(onCollisionAction));
	}
	void ColliderComponent::UnsubscribeCollision(SubscriptionId subscription)
	{
		collisionEvent.Unsubscribe(subscription);
	}

	SubscriptionId ColliderComponent::SubscribeTriggerEnter(std::function<void(const Trigger&)> onTriggerEnterAction)
	{
		return triggerEnterEvent.Subscribe(std::move(onTriggerEnterAction));
	}
	void ColliderComponent::UnsubscribeTriggerEnter(SubscriptionId subscription)
	{
		triggerEnterEvent.Unsubscribe(subscription);
	}

	SubscriptionId ColliderComponent::SubscribeTriggerExit(std::function<void(const Trigger&)> onTriggerExitAction)
	{
		return triggerExitEvent.Subscribe(std::move(onTriggerExitAction));
	}
	void ColliderComponent::UnsubscribeTriggerExit(SubscriptionId subscription)
	{
		triggerExitEvent.Unsubscribe(subscription);
	}

	void ColliderComponent::OnCollision(const Collision& collision)
	{
		collisionEvent.Invoke(collision);
	}
	void ColliderComponent::OnTriggerEnter(const Trigger& trigger)
	{
		triggerEnterEvent.Invoke(trigger);
	}
	void ColliderComponent::OnTriggerExit(const Trigger& trigger)
	{
		triggerExitEvent.Invoke(trigger);
	}
}