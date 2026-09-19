#include "SpeechCatalogLoader.h"
#include <LoggerRegistry.h>
#include <fstream>
#include <stdexcept>

namespace RoguelikeGame
{
	namespace
	{
		constexpr char COMMENT_SYMBOL = ';';
		const std::string WHITESPACE = " \t";
		const std::string UTF8_BOM = "\xEF\xBB\xBF";
		const std::string LINE_PREFIX = "[line ";
		const std::string SET_PREFIX = "[set ";

		std::string Trim(const std::string& text)
		{
			std::size_t first = text.find_first_not_of(WHITESPACE);
			if (first == std::string::npos)
			{
				return std::string();
			}

			return text.substr(first, text.find_last_not_of(WHITESPACE) - first + 1);
		}

		bool TryReadBlockId(const std::string& text, const std::string& prefix, std::string& id)
		{
			if (text.compare(0, prefix.size(), prefix) != 0 || text.back() != ']')
			{
				return false;
			}

			id = Trim(text.substr(prefix.size(), text.size() - prefix.size() - 1));

			return !id.empty();
		}

		// Текст реплики - это остаток строки целиком: в нём есть пробелы,
		// запятые и всё прочее, поэтому делим ровно один раз.
		void ReadField(const std::string& text, int lineNumber, SpeechLine& line)
		{
			std::size_t split = text.find_first_of(WHITESPACE);
			if (split == std::string::npos)
			{
				throw std::runtime_error("Speech line " + std::to_string(lineNumber) + " has a field without a value");
			}

			std::string key = text.substr(0, split);
			std::string value = Trim(text.substr(split + 1));

			if (key == "speaker")
			{
				line.speaker = value;
			}
			else if (key == "text")
			{
				line.text = value;
			}
			else if (key == "sound")
			{
				line.sound = value;
			}
			else
			{
				throw std::runtime_error("Speech line " + std::to_string(lineNumber) + " has unknown field: " + key);
			}
		}
	}

	SpeechCatalog SpeechCatalogLoader::Load(const std::string& filePath)
	{
		std::ifstream file(filePath);
		if (!file.is_open())
		{
			LOG_ERROR("Can't open speech catalog file: " + filePath);
			throw std::runtime_error("Speech catalog file is not available: " + filePath);
		}

		return Parse(file, filePath);
	}

	SpeechCatalog SpeechCatalogLoader::Parse(std::istream& input, const std::string& sourceName)
	{
		SpeechCatalog catalog;

		SpeechLine current;
		bool hasLine = false;

		std::string setId;
		std::vector<std::string> setLines;
		bool hasSet = false;

		std::string text;
		int lineNumber = 0;

		// Набор ссылается на реплики, которые могут идти ниже по файлу,
		// поэтому проверяем ссылки не на месте, а когда прочитано всё.
		std::vector<std::pair<std::string, int>> mentioned;

		auto takeLine = [&]()
		{
			if (!hasLine)
			{
				return;
			}

			if (current.text.empty())
			{
				throw std::runtime_error("Speech line " + current.id + " has no text");
			}

			catalog.AddLine(current);
			current = SpeechLine();
			hasLine = false;
		};

		auto takeSet = [&]()
		{
			if (!hasSet)
			{
				return;
			}

			if (setLines.empty())
			{
				throw std::runtime_error("Speech set " + setId + " is empty");
			}

			catalog.AddSet(setId, setLines);
			setLines.clear();
			hasSet = false;
		};

		while (std::getline(input, text))
		{
			lineNumber++;

			if (lineNumber == 1 && text.compare(0, UTF8_BOM.size(), UTF8_BOM) == 0)
			{
				text.erase(0, UTF8_BOM.size());
			}

			if (!text.empty() && text.back() == '\r')
			{
				text.pop_back();
			}

			text = Trim(text);
			if (text.empty() || text.front() == COMMENT_SYMBOL)
			{
				continue;
			}

			std::string id;
			if (TryReadBlockId(text, LINE_PREFIX, id))
			{
				takeLine();
				takeSet();

				if (catalog.FindLine(id) != nullptr)
				{
					throw std::runtime_error("Speech line is declared twice: " + id);
				}

				current.id = id;
				hasLine = true;
				continue;
			}

			if (TryReadBlockId(text, SET_PREFIX, id))
			{
				takeLine();
				takeSet();

				if (catalog.FindSet(id) != nullptr)
				{
					throw std::runtime_error("Speech set is declared twice: " + id);
				}

				setId = id;
				hasSet = true;
				continue;
			}

			if (hasSet)
			{
				std::size_t split = text.find_first_of(WHITESPACE);
				if (split == std::string::npos || text.substr(0, split) != "line")
				{
					throw std::runtime_error("Speech set " + setId + " has a line that is not a reference");
				}

				std::string reference = Trim(text.substr(split + 1));
				setLines.push_back(reference);
				mentioned.push_back({reference, lineNumber});
				continue;
			}

			if (!hasLine)
			{
				throw std::runtime_error("Speech field outside of a block at line " + std::to_string(lineNumber));
			}

			ReadField(text, lineNumber, current);
		}

		takeLine();
		takeSet();

		for (const auto& reference : mentioned)
		{
			if (catalog.FindLine(reference.first) == nullptr)
			{
				throw std::runtime_error("Speech set points at a line that does not exist: " + reference.first
					+ " at line " + std::to_string(reference.second));
			}
		}

		LOG_INFO("Speech catalog loaded from " + sourceName + ": lines " + std::to_string(catalog.GetLineCount())
			+ ", sets " + std::to_string(catalog.GetSetCount()));

		return catalog;
	}
}
