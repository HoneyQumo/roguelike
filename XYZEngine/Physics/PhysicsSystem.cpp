#include "pch.h"
#include "PhysicsSystem.h"
#include "GameObject.h"
#include <cassert>

namespace XYZEngine
{
	PhysicsSystem* PhysicsSystem::Instance()
	{
		static PhysicsSystem physicsSystem;
		return &physicsSystem;
	}

	void PhysicsSystem::Update()
	{
		for (int i = 0; i < colliders.size(); i++)
		{
			auto body = colliders[i]->GetBody();
			if (body == nullptr || body->GetKinematic())
			{
				continue;
			}

			ColliderComponent* moving = colliders[i];
			Collect(moving->bounds, candidates);

			for (ColliderComponent* other : candidates)
			{
				if (i >= static_cast<int>(colliders.size()) || colliders[i] != moving)
				{
					break;
				}

				if (other == moving)
				{
					continue;
				}

				if ((colliders[i]->collisionLayer & other->ignoredLayers) != 0u
					|| (other->collisionLayer & colliders[i]->ignoredLayers) != 0u)
				{
					continue;
				}

				sf::FloatRect intersection;
				if (colliders[i]->bounds.intersects(other->bounds, intersection))
				{
					if (colliders[i]->isTrigger != other->isTrigger)
					{
						TriggerPair enteredPair = MakeTriggerPair(colliders[i], other);
						if (triggersEnteredPair.find(enteredPair) == triggersEnteredPair.end())
						{
							Trigger trigger(colliders[i], other);
							colliders[i]->OnTriggerEnter(trigger);
							other->OnTriggerEnter(trigger);

							triggersEnteredPair.insert(enteredPair);
						}
					}
					else if (!colliders[i]->isTrigger)
					{
						float intersectionWidth = intersection.width;
						float intersectionHeight = intersection.height;
						Vector2Df intersectionPosition = { intersection.left + 0.5f * intersectionWidth, intersection.top + 0.5f * intersectionHeight };

						Vector2Df aPosition = { colliders[i]->bounds.left + 0.5f * colliders[i]->bounds.width,
												colliders[i]->bounds.top + 0.5f * colliders[i]->bounds.height };
						auto aTransform = colliders[i]->GetGameObject()->GetTransform();

						Vector2Df pushOffset = { 0.f, 0.f };
						if (intersectionWidth > intersectionHeight)
						{
							if (intersectionPosition.y > aPosition.y)
							{
								pushOffset = { 0.f, -intersectionHeight };
							}
							else
							{
								pushOffset = { 0.f, intersectionHeight };
							}
						}
						else
						{
							if (intersectionPosition.x > aPosition.x)
							{
								pushOffset = { -intersectionWidth, 0.f };
							}
							else
							{
								pushOffset = { intersectionWidth, 0.f };
							}
						}

						aTransform->MoveBy(pushOffset);

						// Bounds must follow the push, otherwise the next collider in the loop pushes the object out twice.
						sf::FloatRect pushedBounds = colliders[i]->bounds;
						pushedBounds.left += pushOffset.x;
						pushedBounds.top += pushOffset.y;
						colliders[i]->SetBounds(pushedBounds);

						Collision collision(colliders[i], other, intersection);
						colliders[i]->OnCollision(collision);
						other->OnCollision(collision);
					}
				}
			}
		}

		for (auto triggeredPair = triggersEnteredPair.cbegin(), nextTriggeredPair = triggeredPair; triggeredPair != triggersEnteredPair.cend(); triggeredPair = nextTriggeredPair)
		{
			++nextTriggeredPair;
			if (!triggeredPair->first->bounds.intersects(triggeredPair->second->bounds))
			{
				Trigger trigger(triggeredPair->first, triggeredPair->second);
				triggeredPair->first->OnTriggerExit(trigger);
				triggeredPair->second->OnTriggerExit(trigger);

				triggersEnteredPair.erase(triggeredPair);
			}
		}
	}

	PhysicsSystem::TriggerPair PhysicsSystem::MakeTriggerPair(ColliderComponent* first, ColliderComponent* second)
	{
		return first < second ? TriggerPair(first, second) : TriggerPair(second, first);
	}

	bool PhysicsSystem::IsBefore(ColliderComponent* first, ColliderComponent* second)
	{
		return first->order < second->order;
	}

	void PhysicsSystem::Collect(const sf::FloatRect& area, std::vector<ColliderComponent*>& found) const
	{
		grid.Query(area, found);
		std::sort(found.begin(), found.end(), IsBefore);
	}

	std::vector<ColliderComponent*> PhysicsSystem::Overlap(const sf::FloatRect& area) const
	{
		std::vector<ColliderComponent*> found;
		Collect(area, found);

		found.erase(std::remove_if(found.begin(), found.end(),
			[&area](ColliderComponent* collider) { return collider == nullptr || !collider->bounds.intersects(area); }),
			found.end());

		return found;
	}

	const std::vector<ColliderComponent*>& PhysicsSystem::GetColliders() const
	{
		return colliders;
	}

	void PhysicsSystem::SetCellSize(float newCellSize)
	{
		if (newCellSize <= 0.f || newCellSize == grid.GetCellSize())
		{
			return;
		}

		grid.SetCellSize(newCellSize);

		for (ColliderComponent* collider : colliders)
		{
			collider->gridBounds = collider->bounds;
			grid.Insert(collider, collider->gridBounds);
			collider->isInGrid = true;
		}
	}
	float PhysicsSystem::GetCellSize() const
	{
		return grid.GetCellSize();
	}

	void PhysicsSystem::Subscribe(ColliderComponent* collider)
	{
		assert(collider != nullptr);
		colliders.push_back(collider);

		collider->order = nextOrder++;
		collider->gridBounds = collider->bounds;
		grid.Insert(collider, collider->gridBounds);
		collider->isInGrid = true;
	}
	void PhysicsSystem::Unsubscribe(ColliderComponent* collider)
	{
		if (collider != nullptr && collider->isInGrid)
		{
			grid.Remove(collider, collider->gridBounds);
			collider->isInGrid = false;
		}

		colliders.erase(std::remove_if(colliders.begin(), colliders.end(), [collider](ColliderComponent* obj) { return obj == collider; }), colliders.end());

		// A destroyed collider must not stay in trigger pairs, they are checked after the object is gone.
		for (auto pair = triggersEnteredPair.cbegin(); pair != triggersEnteredPair.cend(); )
		{
			if (pair->first == collider || pair->second == collider)
			{
				pair = triggersEnteredPair.erase(pair);
			}
			else
			{
				++pair;
			}
		}
	}

	void PhysicsSystem::OnBoundsChanged(ColliderComponent* collider)
	{
		if (collider == nullptr || !collider->isInGrid)
		{
			return;
		}

		grid.Move(collider, collider->gridBounds, collider->bounds);
		collider->gridBounds = collider->bounds;
	}
}
