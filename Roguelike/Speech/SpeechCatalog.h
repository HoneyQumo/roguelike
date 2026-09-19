#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace RoguelikeGame
{
	struct SpeechLine
	{
		std::string id;
		std::string speaker;
		std::string text;

		// Ключ звука, а не путь: путь собирает код, как у голосов врагов.
		// Пусто - реплика без озвучки, только субтитром.
		std::string sound;
	};

	class SpeechCatalog
	{
	public:
		void AddLine(const SpeechLine& line);
		void AddSet(const std::string& id, const std::vector<std::string>& lines);

		const SpeechLine* FindLine(const std::string& id) const;

		// nullptr, если набора нет: у звонящего может не быть реплик вовсе.
		const std::vector<std::string>* FindSet(const std::string& id) const;

		std::size_t GetLineCount() const;
		std::size_t GetSetCount() const;

	private:
		std::unordered_map<std::string, SpeechLine> lines;
		std::unordered_map<std::string, std::vector<std::string>> sets;
	};
}
