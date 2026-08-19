#include "pch.h"
#include "PhysicsSystem.h"
#include <cassert>

namespace XYZEngine
{
	PhysicsSystem* PhysicsSystem::Instance()
	{
		static PhysicsSystem physicsSystem;
		return &physicsSystem;
	}

	float PhysicsSystem::GetFixedDeltaTime() const
	{
		return fixedDeltaTime;
	}

	void PhysicsSystem::Update()
	{
		for (int i = 0; i < colliders.size(); i++)
		{
			auto body = colliders[i]->GetGameObject()->GetComponent<RigidbodyComponent>();
			if (body == nullptr || body->GetKinematic())
			{
				continue;
			}

			for (int j = 0; j < colliders.size(); j++)
			{
				if (j == i)
				{
					continue;
				}

				sf::FloatRect intersection;
				if (colliders[i]->bounds.intersects(colliders[j]->bounds, intersection))
				{
					if (colliders[i]->isTrigger != colliders[j]->isTrigger)
					{
						if (triggersEnteredPair.find(colliders[i]) == triggersEnteredPair.end() && triggersEnteredPair.find(colliders[j]) == triggersEnteredPair.end())
						{
							Trigger trigger(colliders[i], colliders[j]);
							colliders[i]->OnTriggerEnter(trigger);
							colliders[j]->OnTriggerEnter(trigger);

							triggersEnteredPair.emplace(colliders[i], colliders[j]);
						}
					}
					else if (!colliders[i]->isTrigger)
					{
						float intersectionWidth = intersection.width;
						float intersectionHeight = intersection.height;
						Vector2Df intersectionPosition = { intersection.left - 0.5f * intersectionWidth, intersection.top - 0.5f * intersectionHeight };

						Vector2Df aPosition = { colliders[i]->bounds.left,  colliders[i]->bounds.top };
						auto aTransform = colliders[i]->GetGameObject()->GetComponent<TransformComponent>();

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
						colliders[i]->bounds.left += pushOffset.x;
						colliders[i]->bounds.top += pushOffset.y;

						Collision collision(colliders[i], colliders[j], intersection);
						colliders[i]->OnCollision(collision);
						colliders[j]->OnCollision(collision);
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

	void PhysicsSystem::Subscribe(ColliderComponent* collider)
	{
		assert(collider != nullptr);
		colliders.push_back(collider);
	}
	void PhysicsSystem::Unsubscribe(ColliderComponent* collider)
	{
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
}