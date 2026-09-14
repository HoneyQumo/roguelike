#pragma once

#include <set>
#include <utility>
#include <iostream>
#include "ColliderComponent.h"
#include "RigidbodyComponent.h"
#include "Vector.h"

namespace XYZEngine
{
	class PhysicsSystem
	{
	public:
		static PhysicsSystem* Instance();

		void Update();

		void Subscribe(ColliderComponent* collider);
		void Unsubscribe(ColliderComponent* collider);

		std::vector<ColliderComponent*> Overlap(const sf::FloatRect& area) const;
		const std::vector<ColliderComponent*>& GetColliders() const;
	private:
		PhysicsSystem() {}
		~PhysicsSystem() {}

		PhysicsSystem(PhysicsSystem const&) = delete;
		PhysicsSystem& operator= (PhysicsSystem const&) = delete;

		using TriggerPair = std::pair<ColliderComponent*, ColliderComponent*>;

		static TriggerPair MakeTriggerPair(ColliderComponent* first, ColliderComponent* second);

		std::vector<ColliderComponent*> colliders;
		std::set<TriggerPair> triggersEnteredPair;

	};
}