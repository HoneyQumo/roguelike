#pragma once

#include <map>
#include <set>
#include <utility>
#include <vector>
#include "Separation.h"
#include "SpatialHashGrid.h"
#include "ColliderComponent.h"
#include "RigidbodyComponent.h"
#include "Vector.h"

namespace XYZEngine
{
	class ColliderComponent;

	class PhysicsSystem
	{
	public:
		static PhysicsSystem* Instance();

		void Update();

		void Subscribe(ColliderComponent* collider);
		void Unsubscribe(ColliderComponent* collider);
		void OnBoundsChanged(ColliderComponent* collider);

		void SetCellSize(float newCellSize);
		float GetCellSize() const;

		std::vector<ColliderComponent*> Overlap(const sf::FloatRect& area) const;
		const std::vector<ColliderComponent*>& GetColliders() const;
	private:
		PhysicsSystem() {}
		~PhysicsSystem() {}

		PhysicsSystem(PhysicsSystem const&) = delete;
		PhysicsSystem& operator= (PhysicsSystem const&) = delete;

		using TriggerPair = std::pair<ColliderComponent*, ColliderComponent*>;

		static TriggerPair MakeTriggerPair(ColliderComponent* first, ColliderComponent* second);
		static bool IsBefore(ColliderComponent* first, ColliderComponent* second);

		void Collect(const sf::FloatRect& area, std::vector<ColliderComponent*>& found) const;
		Vector2Df StepOf(ColliderComponent* collider) const;
		void MoveOut(ColliderComponent* collider, const Vector2Df& offset);
		void RememberPlaces();

		std::vector<ColliderComponent*> colliders;
		std::set<TriggerPair> triggersEnteredPair;
		std::set<TriggerPair> separatedPairs;
		std::map<ColliderComponent*, Vector2Df> lastPlaces;

		SpatialHashGrid<ColliderComponent*> grid;
		std::vector<ColliderComponent*> candidates;
		unsigned int nextOrder = 0u;
	};
}
