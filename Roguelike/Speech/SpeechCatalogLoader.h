#pragma once

#include <istream>
#include <string>
#include "SpeechCatalog.h"

namespace RoguelikeGame
{
	class SpeechCatalogLoader
	{
	public:
		static SpeechCatalog Load(const std::string& filePath);
		static SpeechCatalog Parse(std::istream& input, const std::string& sourceName);
	};
}
