#include "SpeechCatalog.h"
#include <algorithm>

namespace RoguelikeGame
{
	void SpeechCatalog::AddLine(const SpeechLine& line)
	{
		lines[line.id] = line;
	}

	void SpeechCatalog::AddSet(const std::string& id, const std::vector<std::string>& setLines)
	{
		sets[id] = setLines;
	}

	const SpeechLine* SpeechCatalog::FindLine(const std::string& id) const
	{
		auto found = lines.find(id);

		return found == lines.end() ? nullptr : &found->second;
	}

	const std::vector<std::string>* SpeechCatalog::FindSet(const std::string& id) const
	{
		auto found = sets.find(id);

		return found == sets.end() ? nullptr : &found->second;
	}

	std::vector<std::string> SpeechCatalog::GetSounds() const
	{
		std::vector<std::string> sounds;

		for (const auto& line : lines)
		{
			if (line.second.sound.empty())
			{
				continue;
			}

			if (std::find(sounds.begin(), sounds.end(), line.second.sound) == sounds.end())
			{
				sounds.push_back(line.second.sound);
			}
		}

		return sounds;
	}

	std::size_t SpeechCatalog::GetLineCount() const
	{
		return lines.size();
	}

	std::size_t SpeechCatalog::GetSetCount() const
	{
		return sets.size();
	}
}
