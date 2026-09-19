#include "FootstepComponent.h"
#include "Footsteps.h"
#include "GameResources.h"
#include "GameSettings.h"
#include "Noise.h"
#include "SpriteAtlas.h"
#include <GameObject.h>
#include <SpriteMovementAnimationComponent.h>

namespace RoguelikeGame
{
	FootstepComponent::FootstepComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

	void FootstepComponent::Start()
	{
		animation = gameObject->GetComponent<XYZEngine::SpriteMovementAnimationComponent>();
	}

	void FootstepComponent::Update(float deltaTime)
	{
		if (animation == nullptr)
		{
			return;
		}

		XYZEngine::MovementAnimation current = animation->GetCurrentAnimation();
		bool isRunning = current == XYZEngine::MovementAnimation::Run;

		if (current != XYZEngine::MovementAnimation::Walk && !isRunning)
		{
			LoseStep(beat);
			return;
		}

		int frames = isRunning ? RUN_ANIMATION.frames : WALK_ANIMATION.frames;

		if (TakeStep(beat, animation->GetCurrentFrame(), frames))
		{
			Step(isRunning);
		}
	}

	void FootstepComponent::Render()
	{
	}

	void FootstepComponent::SetPlace(SoundPlace newPlace)
	{
		place = newPlace;
	}

	void FootstepComponent::SetSide(Faction newSide)
	{
		side = newSide;
	}

	void FootstepComponent::SetSteps(const char* newSteps)
	{
		steps = newSteps;
	}

	int FootstepComponent::GetStepCount() const
	{
		return stepCount;
	}

	void FootstepComponent::Step(bool isRunning)
	{
		stepCount++;

		PlayOneShot(GameResources::GetStep(steps, nextVariant), STEP_VOLUME, SoundKind::Step, place, gameObject);
		nextVariant = nextVariant % STEP_VARIANTS + 1;

		// Шаг слышно ухом, но следа он не оставляет: иначе подойти со спины
		// нельзя - враг услышит раньше, чем игрок дотянется ножом.
		if (!isRunning)
		{
			return;
		}

		Noise noise;
		noise.position = gameObject->GetTransform()->GetWorldPosition();
		noise.radius = RUN_NOISE_RADIUS;
		noise.loudness = RUN_NOISE_LOUDNESS;
		noise.from = side;

		RaiseNoise(noise, gameObject);
	}
}
