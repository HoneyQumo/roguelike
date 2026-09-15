#include "Tileset.h"
#include <LoggerRegistry.h>
#include <ResourceSystem.h>
#include <cctype>

namespace RoguelikeGame
{
    namespace
    {
        bool IsPlain(char symbol)
        {
            return (symbol >= 'a' && symbol <= 'z') || (symbol >= '0' && symbol <= '9') || symbol == '_';
        }
    }

    std::string TilesetName(const std::string& tileset)
    {
        std::string plain;
        for (char symbol : tileset)
        {
            char lowered = static_cast<char>(std::tolower(static_cast<unsigned char>(symbol)));
            if (IsPlain(lowered))
            {
                plain += lowered;
            }
        }

        return plain.empty() ? DEFAULT_TILESET : plain;
    }

    std::string TilesetTextureName(const std::string& tileset)
    {
        return TILESET_TEXTURE_PREFIX + TilesetName(tileset);
    }

    std::string TilesetFilePath(const std::string& tileset)
    {
        return TILESET_FILE_PREFIX + TilesetName(tileset) + TILESET_FILE_SUFFIX;
    }

    const sf::Texture* LoadTileset(const std::string& tileset)
    {
        std::string textureName = TilesetTextureName(tileset);

        if (!XYZEngine::ResourceSystem::Instance()->HasTexture(textureName))
        {
            XYZEngine::ResourceSystem::Instance()->LoadTexture(textureName, TilesetFilePath(tileset), false);
        }

        const sf::Texture* loaded = XYZEngine::ResourceSystem::Instance()->HasTexture(textureName)
            ? XYZEngine::ResourceSystem::Instance()->GetTextureShared(textureName)
            : nullptr;

        if (loaded == nullptr && TilesetName(tileset) != DEFAULT_TILESET)
        {
            LOG_WARN("Tileset " + TilesetName(tileset) + " is not loaded, falling back to " + DEFAULT_TILESET);

            return LoadTileset(DEFAULT_TILESET);
        }

        return loaded;
    }
}
