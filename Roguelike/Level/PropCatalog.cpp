#include "PropCatalog.h"
#include <LoggerRegistry.h>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace RoguelikeGame
{
    namespace
    {
        constexpr char PROP_COMMENT_SYMBOL = ';';
        const std::string PROP_WHITESPACE = " \t";
        const std::string PROP_UTF8_BOM = "\xEF\xBB\xBF";
        const std::string PROP_BLOCK_PREFIX = "[prop ";

        bool ReadColor(std::istringstream& stream, sf::Color& color)
        {
            int red = 0;
            int green = 0;
            int blue = 0;

            if (!(stream >> red >> green >> blue))
            {
                return false;
            }

            color = sf::Color(static_cast<sf::Uint8>(red), static_cast<sf::Uint8>(green), static_cast<sf::Uint8>(blue));

            return true;
        }
    }

    float PropDefinition::Height() const
    {
        return height > 0.f ? height : size;
    }

    // Взрыв имеет смысл только у того, что можно сломать: детонирует остов, а не целая бочка.
    bool PropDefinition::IsExplosive() const
    {
        return blastRadius > 0.f && blastDamage > 0.f && IsDestructible();
    }

    bool PropDefinition::ShowsWreckOnBreak() const
    {
        return !IsExplosive();
    }

    bool PropDefinition::IsDestructible() const
    {
        return health > 0.f;
    }

    bool PropDefinition::IsTrap() const
    {
        return trapKind != TrapKind::None;
    }

    bool PropDefinition::IsOpenable() const
    {
        return openable;
    }

    bool PropDefinition::HasFrame() const
    {
        return !texturePath.empty() && frame.width > 0 && frame.height > 0;
    }

    bool PropDefinition::HasSpentFrame() const
    {
        return HasFrame() && spentFrame.width > 0 && spentFrame.height > 0;
    }

    PropCatalog PropCatalog::Load(const std::string& filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            LOG_ERROR("Can't open prop catalog: " + filePath);
            throw std::runtime_error("Prop catalog is not available: " + filePath);
        }

        return Parse(file, filePath);
    }

    PropCatalog PropCatalog::Parse(std::istream& input, const std::string& sourceName)
    {
        PropCatalog catalog;
        PropDefinition current;
        bool hasCurrent = false;

        std::string line;
        int lineNumber = 0;

        while (std::getline(input, line))
        {
            lineNumber++;

            if (lineNumber == 1 && line.compare(0, PROP_UTF8_BOM.size(), PROP_UTF8_BOM) == 0)
            {
                line.erase(0, PROP_UTF8_BOM.size());
            }

            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }

            line = Trim(line);
            if (line.empty() || line.front() == PROP_COMMENT_SYMBOL)
            {
                continue;
            }

            if (line.compare(0, PROP_BLOCK_PREFIX.size(), PROP_BLOCK_PREFIX) == 0 && line.back() == ']')
            {
                if (hasCurrent)
                {
                    catalog.props.push_back(current);
                }

                current = PropDefinition();
                current.id = Trim(line.substr(PROP_BLOCK_PREFIX.size(), line.size() - PROP_BLOCK_PREFIX.size() - 1));
                hasCurrent = true;

                if (current.id.empty())
                {
                    LOG_ERROR("Prop has no id, line " + std::to_string(lineNumber) + " in " + sourceName);
                    throw std::runtime_error("Prop has no id in " + sourceName);
                }

                continue;
            }

            if (!hasCurrent)
            {
                LOG_ERROR("Prop line outside of a block, line " + std::to_string(lineNumber) + " in " + sourceName);
                throw std::runtime_error("Prop line outside of a block in " + sourceName);
            }

            std::istringstream stream(line);
            std::string key;
            stream >> key;

            if (key == "name")
            {
                std::string rest;
                std::getline(stream, rest);
                current.name = Trim(rest);
                continue;
            }

            if (key == "hit")
            {
                stream >> current.hitEffect;
                continue;
            }

            if (key == "loot")
            {
                stream >> current.lootTable;
                continue;
            }

            if (key == "health")
            {
                if (!(stream >> current.health) || current.health < 0.f)
                {
                    LOG_ERROR("Prop health must not be negative, line " + std::to_string(lineNumber) + " in " + sourceName);
                    throw std::runtime_error("Prop health must not be negative in " + sourceName);
                }

                continue;
            }

            if (key == "size")
            {
                if (!(stream >> current.size) || current.size <= 0.f)
                {
                    LOG_ERROR("Prop size must be positive, line " + std::to_string(lineNumber) + " in " + sourceName);
                    throw std::runtime_error("Prop size must be positive in " + sourceName);
                }

                continue;
            }

            if (key == "color" || key == "brokenColor" || key == "openedColor")
            {
                sf::Color color;
                if (!ReadColor(stream, color))
                {
                    LOG_ERROR("Prop color needs three numbers, line " + std::to_string(lineNumber) + " in " + sourceName);
                    throw std::runtime_error("Prop color needs three numbers in " + sourceName);
                }

                if (key == "color")
                {
                    current.color = color;
                }
                else if (key == "brokenColor")
                {
                    current.brokenColor = color;
                }
                else
                {
                    current.openedColor = color;
                }

                continue;
            }

            if (key == "frame" || key == "spentFrame")
            {
                std::string path;
                int x = 0;
                int y = 0;
                int width = 0;
                int height = 0;

                if (!(stream >> path >> x >> y >> width >> height) || width <= 0 || height <= 0)
                {
                    LOG_ERROR("Prop frame needs a path and four numbers, line " + std::to_string(lineNumber) + " in " + sourceName);
                    throw std::runtime_error("Prop frame needs a path and four numbers in " + sourceName);
                }

                current.texturePath = path;
                (key == "frame" ? current.frame : current.spentFrame) = {x, y, width, height};
                continue;
            }

            if (key == "blast")
            {
                stream >> current.blastRadius >> current.blastDamage >> current.blastFuse;
                continue;
            }

            if (key == "height")
            {
                stream >> current.height;
                continue;
            }

            if (key == "openable")
            {
                std::string value;
                stream >> value;
                current.openable = value == "true";
                continue;
            }

            if (key == "solid" || key == "cover")
            {
                std::string value;
                stream >> value;
                (key == "solid" ? current.isSolid : current.isCover) = value == "true";
                continue;
            }

            if (key == "trap")
            {
                std::string kind;
                stream >> kind >> current.trapAmount;
                current.trapKind = TrapKindFrom(kind);
                if (current.trapKind == TrapKind::None)
                {
                    LOG_WARN("Unknown trap kind at line " + std::to_string(lineNumber) + ": " + kind);
                }

                continue;
            }

            if (key == "burn")
            {
                stream >> current.burnTime >> current.burnSpread;
                continue;
            }

            if (key == "jitter")
            {
                stream >> current.jitterDegrees;
                continue;
            }

            if (key == "wreck")
            {
                std::string value;
                stream >> value;
                current.leavesWreck = value == "true";
                continue;
            }

            if (key == "panel")
            {
                std::string value;
                stream >> value;
                current.isPanel = value == "true";
                continue;
            }

            if (key == "key")
            {
                stream >> current.keyItem;
                current.openable = true;
                continue;
            }

            LOG_WARN("Unknown prop field at line " + std::to_string(lineNumber) + ": " + key);
        }

        if (hasCurrent)
        {
            catalog.props.push_back(current);
        }

        LOG_INFO("Props loaded: " + std::to_string(catalog.props.size()));
        return catalog;
    }

    const PropCatalog& PropCatalog::Empty()
    {
        static const PropCatalog empty;
        return empty;
    }

    const PropDefinition* PropCatalog::Find(const std::string& id) const
    {
        if (id.empty())
        {
            return nullptr;
        }

        for (const PropDefinition& prop : props)
        {
            if (prop.id == id)
            {
                return &prop;
            }
        }

        return nullptr;
    }

    std::size_t PropCatalog::Size() const
    {
        return props.size();
    }

    bool PropCatalog::IsEmpty() const
    {
        return props.empty();
    }

    std::vector<PropDefinition>::const_iterator PropCatalog::begin() const
    {
        return props.begin();
    }

    std::vector<PropDefinition>::const_iterator PropCatalog::end() const
    {
        return props.end();
    }

    std::string PropCatalog::Trim(const std::string& line)
    {
        size_t first = line.find_first_not_of(PROP_WHITESPACE);
        if (first == std::string::npos)
        {
            return "";
        }

        size_t last = line.find_last_not_of(PROP_WHITESPACE);

        return line.substr(first, last - first + 1);
    }
}
