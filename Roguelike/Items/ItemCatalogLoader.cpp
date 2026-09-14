#include "ItemCatalogLoader.h"
#include <LoggerRegistry.h>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace RoguelikeGame
{
    namespace
    {
        constexpr char ITEM_COMMENT_SYMBOL = ';';
        const std::string ITEM_WHITESPACE = " \t";
        const std::string ITEM_UTF8_BOM = "\xEF\xBB\xBF";
        const std::string ITEM_BLOCK_PREFIX = "[item ";
    }

    ItemCatalog ItemCatalogLoader::Load(const std::string& filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            LOG_ERROR("Can't open item catalog file: " + filePath);
            throw std::runtime_error("Item catalog file is not available: " + filePath);
        }

        return Parse(file, filePath);
    }

    ItemCatalog ItemCatalogLoader::Parse(std::istream& input, const std::string& sourceName)
    {
        ItemCatalog catalog;
        ItemDefinition current;
        bool hasCurrent = false;

        std::string line;
        int lineNumber = 0;
        int blockLine = 0;

        while (std::getline(input, line))
        {
            lineNumber++;

            if (lineNumber == 1 && line.compare(0, ITEM_UTF8_BOM.size(), ITEM_UTF8_BOM) == 0)
            {
                line.erase(0, ITEM_UTF8_BOM.size());
            }

            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }

            line = Trim(line);
            if (line.empty() || line.front() == ITEM_COMMENT_SYMBOL)
            {
                continue;
            }

            std::string id;
            if (TryReadBlockId(line, id))
            {
                if (hasCurrent)
                {
                    Validate(current, blockLine);
                    catalog.Add(current);
                }

                if (catalog.Contains(id))
                {
                    LOG_ERROR("Duplicate item id in " + sourceName + ": " + id);
                    throw std::runtime_error("Duplicate item id: " + id);
                }

                current = ItemDefinition();
                current.id = id;
                hasCurrent = true;
                blockLine = lineNumber;
                continue;
            }

            if (!hasCurrent)
            {
                LOG_ERROR("Item field outside of a block in " + sourceName + ", line " + std::to_string(lineNumber));
                throw std::runtime_error("Item field outside of a block");
            }

            ReadField(line, lineNumber, current);
        }

        if (hasCurrent)
        {
            Validate(current, blockLine);
            catalog.Add(current);
        }

        return catalog;
    }

    bool ItemCatalogLoader::TryReadBlockId(const std::string& line, std::string& id)
    {
        if (line.compare(0, ITEM_BLOCK_PREFIX.size(), ITEM_BLOCK_PREFIX) != 0 || line.back() != ']')
        {
            return false;
        }

        id = Trim(line.substr(ITEM_BLOCK_PREFIX.size(), line.size() - ITEM_BLOCK_PREFIX.size() - 1));
        if (id.empty())
        {
            LOG_ERROR("Item block has no id");
            throw std::runtime_error("Item block has no id");
        }

        return true;
    }

    void ItemCatalogLoader::ReadField(const std::string& line, int lineNumber, ItemDefinition& item)
    {
        std::istringstream stream(line);
        std::string key;
        stream >> key;

        if (key == "name")
        {
            std::string rest;
            std::getline(stream, rest);
            item.name = Trim(rest);
            return;
        }

        if (key == "type")
        {
            std::string name;
            stream >> name;
            if (!TryGetType(name, item.type))
            {
                LOG_ERROR("Unknown item type at line " + std::to_string(lineNumber) + ": " + name);
                throw std::runtime_error("Unknown item type: " + name);
            }

            return;
        }

        if (key == "icon")
        {
            std::string path;
            int x = 0;
            int y = 0;
            int width = 0;
            int height = 0;
            if (!(stream >> path >> x >> y >> width >> height) || width <= 0 || height <= 0)
            {
                LOG_ERROR("Bad item icon at line " + std::to_string(lineNumber));
                throw std::runtime_error("Bad item icon at line " + std::to_string(lineNumber));
            }

            item.icon.texturePath = path;
            item.icon.rect = {x, y, width, height};
            return;
        }

        if (key == "tint")
        {
            int red = 255;
            int green = 255;
            int blue = 255;
            if (!(stream >> red >> green >> blue))
            {
                LOG_ERROR("Bad item tint at line " + std::to_string(lineNumber));
                throw std::runtime_error("Bad item tint at line " + std::to_string(lineNumber));
            }

            item.icon.tint = sf::Color(static_cast<sf::Uint8>(red), static_cast<sf::Uint8>(green), static_cast<sf::Uint8>(blue));
            return;
        }

        if (key == "scale")
        {
            stream >> item.icon.worldScale;
            return;
        }

        if (key == "stackable")
        {
            std::string value;
            stream >> value;
            item.stackable = value == "true" || value == "1";
            return;
        }

        if (key == "maxStack")
        {
            stream >> item.maxStack;
            return;
        }

        if (key == "effect")
        {
            std::string name;
            stream >> name;
            if (!TryGetEffectKind(name, item.effect.kind))
            {
                LOG_ERROR("Unknown item effect at line " + std::to_string(lineNumber) + ": " + name);
                throw std::runtime_error("Unknown item effect: " + name);
            }

            stream >> item.effect.amount;

            std::string target;
            stream >> target;
            item.effect.target = target;
            return;
        }

        LOG_WARN("Unknown item field at line " + std::to_string(lineNumber) + ": " + key);
    }

    void ItemCatalogLoader::Validate(const ItemDefinition& item, int lineNumber)
    {
        if (item.name.empty())
        {
            LOG_ERROR("Item " + item.id + " has no name, line " + std::to_string(lineNumber));
            throw std::runtime_error("Item has no name: " + item.id);
        }

        if (item.icon.texturePath.empty() || item.icon.rect.width <= 0 || item.icon.rect.height <= 0)
        {
            LOG_ERROR("Item " + item.id + " has no icon, line " + std::to_string(lineNumber));
            throw std::runtime_error("Item has no icon: " + item.id);
        }

        if (item.maxStack < 1)
        {
            LOG_ERROR("Item " + item.id + " has max stack below one");
            throw std::runtime_error("Item has max stack below one: " + item.id);
        }

        if (!item.stackable && item.maxStack > 1)
        {
            LOG_WARN("Item " + item.id + " is not stackable, max stack is ignored");
        }
    }

    bool ItemCatalogLoader::TryGetType(const std::string& name, ItemType& type)
    {
        for (const ItemTypeName& typeName : ITEM_TYPE_NAMES)
        {
            if (name == typeName.name)
            {
                type = typeName.type;
                return true;
            }
        }

        return false;
    }

    bool ItemCatalogLoader::TryGetEffectKind(const std::string& name, ItemEffectKind& kind)
    {
        for (const ItemEffectName& effectName : ITEM_EFFECT_NAMES)
        {
            if (name == effectName.name)
            {
                kind = effectName.kind;
                return true;
            }
        }

        return false;
    }

    std::string ItemCatalogLoader::Trim(const std::string& line)
    {
        size_t first = line.find_first_not_of(ITEM_WHITESPACE);
        if (first == std::string::npos)
        {
            return "";
        }

        size_t last = line.find_last_not_of(ITEM_WHITESPACE);
        return line.substr(first, last - first + 1);
    }
}
