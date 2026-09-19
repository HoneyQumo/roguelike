#include "pch.h"
#include "SoundPool.h"
#include "FrameClock.h"
#include "SoundSpace.h"

namespace XYZEngine
{
	int ChooseSoundSlot(const std::vector<SoundSlot>& slots, float now, int category)
	{
		int oldest = -1;

		for (std::size_t index = 0; index < slots.size(); index++)
		{
			const SoundSlot& slot = slots[index];
			if (slot.endsAt <= now)
			{
				return static_cast<int>(index);
			}

			if (slot.category != category)
			{
				continue;
			}

			if (oldest < 0 || slot.startedAt < slots[oldest].startedAt)
			{
				oldest = static_cast<int>(index);
			}
		}

		return oldest;
	}

	SoundPool* SoundPool::Instance()
	{
		static SoundPool instance;

		return &instance;
	}

	void SoundPool::SetSlots(std::size_t count)
	{
		sounds.clear();
		sounds.resize(count);
		slots.assign(count, SoundSlot());
	}

	std::size_t SoundPool::GetSlots() const
	{
		return slots.size();
	}

	void SoundPool::PlayAt(const sf::SoundBuffer* buffer, const Vector2Df& at, float volume, int category,
		float minDistance, float attenuation)
	{
		int index = Take(buffer, volume, category);
		if (index < 0)
		{
			return;
		}

		SoundPoint point = ToSoundPoint(at);

		sounds[index].setRelativeToListener(false);
		sounds[index].setPosition(point.x, point.y, point.z);
		sounds[index].setMinDistance(minDistance);
		sounds[index].setAttenuation(attenuation);
		sounds[index].play();
	}

	void SoundPool::PlayAtListener(const sf::SoundBuffer* buffer, float volume, int category)
	{
		int index = Take(buffer, volume, category);
		if (index < 0)
		{
			return;
		}

		sounds[index].setRelativeToListener(true);
		sounds[index].setPosition(0.f, 0.f, 0.f);
		sounds[index].play();
	}

	void SoundPool::Clear()
	{
		for (sf::Sound& sound : sounds)
		{
			sound.stop();
		}

		slots.assign(slots.size(), SoundSlot());
	}

	const std::vector<SoundSlot>& SoundPool::GetBusySlots() const
	{
		return slots;
	}

	int SoundPool::Take(const sf::SoundBuffer* buffer, float volume, int category)
	{
		if (buffer == nullptr || slots.empty())
		{
			return -1;
		}

		float now = FrameClock::Instance()->GetUnscaledElapsedSeconds();

		int index = ChooseSoundSlot(slots, now, category);
		if (index < 0)
		{
			return -1;
		}

		slots[index].category = category;
		slots[index].startedAt = now;
		slots[index].endsAt = now + buffer->getDuration().asSeconds();

		sounds[index].stop();
		sounds[index].setBuffer(*buffer);
		sounds[index].setVolume(volume);

		return index;
	}
}
