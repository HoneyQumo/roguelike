#include "SpeechCatalog.h"

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

	std::size_t SpeechCatalog::GetLineCount() const
	{
		return lines.size();
	}

	std::size_t SpeechCatalog::GetSetCount() const
	{
		return sets.size();
	}
}
