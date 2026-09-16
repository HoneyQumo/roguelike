#pragma once

#include <istream>
#include <string>
#include <SFML/Graphics/Rect.hpp>
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
        float height = 0.f;
        sf::Color color = {150, 110, 60};
        sf::Color brokenColor = {80, 60, 35};
        sf::Color openedColor = {90, 80, 55};
        std::string keyItem;
        std::string texturePath;
        sf::IntRect frame;
        sf::IntRect spentFrame;
        bool openable = false;
        bool isSolid = true;
        bool isCover = false;
        bool isPanel = false;

        // Высота по умолчанию равна ширине: почти все пропы квадратные.
        float Height() const;
        bool IsDestructible() const;
        bool IsOpenable() const;
        bool HasFrame() const;
        bool HasSpentFrame() const;
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
