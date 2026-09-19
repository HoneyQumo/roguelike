#include "pch.h"
#include "AudioComponent.h"
#include "GameObject.h"
#include "LoggerRegistry.h"
#include "TransformComponent.h"

namespace XYZEngine
{
	// Пока источник не поставили на карту, он звучит на слушателе - как и звучал,
	// когда пространства не было вовсе.
	AudioComponent::AudioComponent(GameObject* gameObject) : SoundSourceComponent(gameObject)
	{
		SetRelativeToListener(true);
	}

	void AudioComponent::SetSound(const sf::SoundBuffer* newSound)
	{
		if (newSound == nullptr)
		{
			LOG_WARN("Can't set empty sound.");
			return;
		}

		sound.setBuffer(*newSound);
	}

	// Звук на карте едет за своим объектом: машина побега приезжает издалека и
	// меняет место каждый кадр, а разовой установки в момент запуска ей мало.
	void AudioComponent::Update(float deltaTime)
	{
		if (IsRelativeToListener())
		{
			return;
		}

		TransformComponent* transform = GetGameObject()->GetTransform();
		if (transform == nullptr || transform->GetWorldPosition() == GetWorldPosition())
		{
			return;
		}

		SetWorldPosition(transform->GetWorldPosition());
	}

	void AudioComponent::Play()
	{
		if (sound.getBuffer() == nullptr)
		{
			LOG_WARN("Can't play sound without buffer.");
			return;
		}

		SoundSourceComponent::Play();
	}

	sf::Sound* AudioComponent::GetSource()
	{
		return &sound;
	}
	const sf::Sound* AudioComponent::GetSource() const
	{
		return &sound;
	}
}
