#pragma once

#include <istream>
#include <string>
#include <vector>
#include <SFML/Graphics/Color.hpp>

namespace RoguelikeGame
{
    struct PropDefinition
    {
        std::string id;
        std::string name;
        std::string lootTable;
        std::string hitEffect = "impact";
        float health = 0.f;
        float size = 48.f;
        sf::Color color = {150, 110, 60};
        sf::Color brokenColor = {80, 60, 35};

        bool IsDestructible() const;
    };

    class PropCatalog
    {
    public:
        static PropCatalog Load(const std::string& filePath);
        static PropCatalog Parse(std::istream& input, const std::string& sourceName);
        static const PropCatalog& Empty();

        const PropDefinition* Find(const std::string& id) const;

        std::size_t Size() const;
        bool IsEmpty() const;

        std::vector<PropDefinition>::const_iterator begin() const;
        std::vector<PropDefinition>::const_iterator end() const;

    private:
        std::vector<PropDefinition> props;

        static std::string Trim(const std::string& line);
    };
}
