#pragma once

#include <string>

namespace sf
{
    class Texture;
}

namespace RoguelikeGame
{
    constexpr auto DEFAULT_TILESET = "ruins";
    constexpr auto TILESET_TEXTURE_PREFIX = "tiles_";
    constexpr auto TILESET_FILE_PREFIX = "Resources/Textures/tiles_";
    constexpr auto TILESET_FILE_SUFFIX = ".png";

    std::string TilesetName(const std::string& tileset);
    std::string TilesetTextureName(const std::string& tileset);
    std::string TilesetFilePath(const std::string& tileset);

    const sf::Texture* LoadTileset(const std::string& tileset);
}
