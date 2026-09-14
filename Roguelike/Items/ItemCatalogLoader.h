#pragma once

#include <istream>
#include <string>
#include "ItemCatalog.h"

namespace RoguelikeGame
{
    class ItemCatalogLoader
    {
    public:
        static ItemCatalog Load(const std::string& filePath);
        static ItemCatalog Parse(std::istream& input, const std::string& sourceName);

    private:
        static bool TryReadBlockId(const std::string& line, std::string& id);
        static void ReadField(const std::string& line, int lineNumber, ItemDefinition& item);
        static void Validate(const ItemDefinition& item, int lineNumber);
        static bool TryGetType(const std::string& name, ItemType& type);
        static bool TryGetEffectKind(const std::string& name, ItemEffectKind& kind);
        static std::string Trim(const std::string& line);
    };
}
