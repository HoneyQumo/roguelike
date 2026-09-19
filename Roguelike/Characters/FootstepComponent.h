#pragma once

#include <Component.h>
#include "Faction.h"
#include "Footsteps.h"
#include "WorldSound.h"

namespace XYZEngine
{
	class SpriteMovementAnimationComponent;
}

namespace RoguelikeGame
{
	/**
	*	Озвучивает шаги и решает, оставлять ли от них след.
	*
	*	Шаг привязан к кадру касания, а не к таймеру: таймер разъехался бы
	*	с картинкой на первом же изменении скорости.
	*/
	class FootstepComponent : public XYZEngine::Component
	{
	public:
		FootstepComponent(XYZEngine::GameObject* gameObject);

		void Start() override;
		void Update(float deltaTime) override;
		void Render() override;

		void SetPlace(SoundPlace newPlace);
		void SetSide(Faction newSide);
		void SetSteps(const char* newSteps);

		int GetStepCount() const;

	private:
		XYZEngine::SpriteMovementAnimationComponent* animation = nullptr;

		SoundPlace place = SoundPlace::InWorld;
		Faction side = Faction::Neutral;
		const char* steps = FOE_STEPS;

		StepBeat beat;
		int nextVariant = 1;
		int stepCount = 0;

		void Step(bool isRunning);
	};
}
