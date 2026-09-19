#include "SpeechDirector.h"
#include "GameSettings.h"
#include <LoggerRegistry.h>
#include <ResourceSystem.h>
#include <randomizer.h>

namespace RoguelikeGame
{
	SpeechDirector& SpeechDirector::Current()
	{
		static SpeechDirector director;

		return director;
	}

	void SpeechDirector::SetCatalog(const SpeechCatalog* newCatalog)
	{
		catalog = newCatalog;
	}

	void SpeechDirector::SetQueue(SubtitleQueue* newQueue)
	{
		queue = newQueue;
	}

	bool SpeechDirector::Say(const std::string& lineId, XYZEngine::GameObject* source, SoundPlace place)
	{
		if (catalog == nullptr)
		{
			LOG_WARN("Nothing to say from: speech catalog is not set");
			return false;
		}

		const SpeechLine* line = catalog->FindLine(lineId);
		if (line == nullptr)
		{
			// Молчать втихую нельзя: пропавшую реплику иначе не найти.
			LOG_WARN("There is no such speech line: " + lineId);
			return false;
		}

		saidCount++;

		if (queue != nullptr)
		{
			queue->Say(line->speaker, line->text, SubtitleTimeFor(line->text));
		}

		if (!line->sound.empty())
		{
			const sf::SoundBuffer* voice = XYZEngine::ResourceSystem::Instance()->GetSound(line->sound);
			PlayOneShot(voice, VOICE_VOLUME, SoundKind::Voice, place, source);
		}

		return true;
	}

	bool SpeechDirector::SayFromSet(const std::string& setId, XYZEngine::GameObject* source, SoundPlace place)
	{
		if (catalog == nullptr)
		{
			LOG_WARN("Nothing to say from: speech catalog is not set");
			return false;
		}

		const std::vector<std::string>* set = catalog->FindSet(setId);
		if (set == nullptr || set->empty())
		{
			LOG_WARN("There is no such speech set: " + setId);
			return false;
		}

		int index = random<int>(0, static_cast<int>(set->size()) - 1);

		return Say((*set)[index], source, place);
	}

	void SpeechDirector::Silence()
	{
		if (queue != nullptr)
		{
			queue->Clear();
		}
	}

	int SpeechDirector::GetSaidCount() const
	{
		return saidCount;
	}
}
