#include "pch.h"
#include "ResourceSystem.h"
#include "LoggerRegistry.h"
#include <cassert>

namespace XYZEngine
{
    ResourceSystem* ResourceSystem::Instance()
    {
        static ResourceSystem resourceSystem;
        return &resourceSystem;
    }

    void ResourceSystem::LoadTexture(const std::string& name, std::string sourcePath, bool isSmooth)
    {
        textures.Load(name, sourcePath, [&sourcePath, isSmooth](sf::Texture& texture)
        {
            if (!texture.loadFromFile(sourcePath))
            {
                return false;
            }

            texture.setSmooth(isSmooth);
            return true;
        });
    }

    void ResourceSystem::LoadTexturePart(const std::string& name, std::string sourcePath, sf::IntRect area, bool isSmooth)
    {
        assert(area.width > 0 && area.height > 0);

        textures.Load(name, sourcePath, [&sourcePath, area, isSmooth](sf::Texture& texture)
        {
            if (!texture.loadFromFile(sourcePath, area))
            {
                return false;
            }

            texture.setSmooth(isSmooth);
            return true;
        });
    }

    const sf::Texture* ResourceSystem::GetTextureShared(const std::string& name) const
    {
        return textures.Get(name);
    }

    bool ResourceSystem::HasTexture(const std::string& name) const
    {
        return textures.Contains(name);
    }

    void ResourceSystem::DeleteSharedTexture(const std::string& name)
    {
        textures.Erase(name);
    }

    void ResourceSystem::LoadTextureMap(const std::string& name, std::string sourcePath, sf::Vector2u elementPixelSize, int totalElements, bool isSmooth)
    {
        assert(elementPixelSize.x > 0 && elementPixelSize.y > 0);
        assert(totalElements > 0);

        if (textureMaps.find(name) != textureMaps.end())
        {
            LOG_WARN("Texture map is already loaded: " + name);
            return;
        }

        sf::Image source;
        if (!source.loadFromFile(sourcePath))
        {
            LOG_ERROR("Can't load texture map: " + sourcePath);
            return;
        }

        auto sourceSize = source.getSize();
        std::vector<sf::IntRect> areas;

        for (unsigned int y = 0; y + elementPixelSize.y <= sourceSize.y && static_cast<int>(areas.size()) < totalElements; y += elementPixelSize.y)
        {
            for (unsigned int x = 0; x + elementPixelSize.x <= sourceSize.x && static_cast<int>(areas.size()) < totalElements; x += elementPixelSize.x)
            {
                areas.push_back(sf::IntRect(x, y, elementPixelSize.x, elementPixelSize.y));
            }
        }

        if (static_cast<int>(areas.size()) < totalElements)
        {
            LOG_WARN("Texture map is smaller than requested: " + name + ", elements: " + std::to_string(areas.size()));
        }

        CutTextureMap(name, source, areas, isSmooth);
    }

    // Эффекты лежат полосами, кадры режутся по прямоугольнику.
    void ResourceSystem::LoadTextureStrip(const std::string& name, std::string sourcePath, sf::IntRect firstElementArea, int totalElements, bool isSmooth)
    {
        assert(firstElementArea.width > 0 && firstElementArea.height > 0);
        assert(totalElements > 0);

        if (textureMaps.find(name) != textureMaps.end())
        {
            LOG_WARN("Texture map is already loaded: " + name);
            return;
        }

        sf::Image source;
        if (!source.loadFromFile(sourcePath))
        {
            LOG_ERROR("Can't load texture map: " + sourcePath);
            return;
        }

        std::vector<sf::IntRect> areas;
        for (int i = 0; i < totalElements; i++)
        {
            areas.push_back(sf::IntRect(firstElementArea.left + firstElementArea.width * i, firstElementArea.top,
                                        firstElementArea.width, firstElementArea.height));
        }

        CutTextureMap(name, source, areas, isSmooth);
    }

    const sf::Texture* ResourceSystem::GetTextureMapElementShared(const std::string& name, int elementIndex) const
    {
        auto textureMap = textureMaps.find(name);
        if (textureMap == textureMaps.end())
        {
            LOG_ERROR("Texture map not found: " + name);
            return nullptr;
        }

        const TextureMap& elements = textureMap->second;
        if (elementIndex < 0 || elementIndex >= static_cast<int>(elements.size()))
        {
            LOG_ERROR("Texture map element out of range: " + name + ", index " + std::to_string(elementIndex));
            return nullptr;
        }

        return elements[elementIndex].get();
    }

    int ResourceSystem::GetTextureMapElementsCount(const std::string& name) const
    {
        auto textureMap = textureMaps.find(name);
        if (textureMap == textureMaps.end())
        {
            LOG_WARN("Texture map not found: " + name);
            return 0;
        }

        return static_cast<int>(textureMap->second.size());
    }

    void ResourceSystem::DeleteSharedTextureMap(const std::string& name)
    {
        textureMaps.erase(name);
    }

    void ResourceSystem::CutTextureMap(const std::string& name, const sf::Image& source, const std::vector<sf::IntRect>& areas, bool isSmooth)
    {
        TextureMap elements;
        elements.reserve(areas.size());

        for (const auto& area : areas)
        {
            auto element = std::make_unique<sf::Texture>();
            if (!element->loadFromImage(source, area))
            {
                LOG_ERROR("Can't cut texture map element: " + name + ", index " + std::to_string(elements.size()));
                continue;
            }

            element->setSmooth(isSmooth);
            elements.push_back(std::move(element));
        }

        LOG_INFO("Texture map loaded: " + name + ", elements: " + std::to_string(elements.size()));
        textureMaps.emplace(name, std::move(elements));
    }

    void ResourceSystem::LoadSound(const std::string& name, std::string sourcePath)
    {
        sounds.Load(name, sourcePath, [&sourcePath](sf::SoundBuffer& sound) { return sound.loadFromFile(sourcePath); });
    }

    const sf::SoundBuffer* ResourceSystem::GetSound(const std::string& name) const
    {
        return sounds.Get(name);
    }

    bool ResourceSystem::HasSound(const std::string& name) const
    {
        return sounds.Contains(name);
    }

    void ResourceSystem::DeleteSound(const std::string& name)
    {
        sounds.Erase(name);
    }

    void ResourceSystem::LoadMusic(const std::string& name, std::string sourcePath)
    {
        musicTracks.Load(name, sourcePath, [&sourcePath](sf::Music& music) { return music.openFromFile(sourcePath); });
    }

    sf::Music* ResourceSystem::GetMusic(const std::string& name) const
    {
        return musicTracks.Get(name);
    }

    void ResourceSystem::DeleteMusic(const std::string& name)
    {
        musicTracks.Erase(name);
    }

    void ResourceSystem::LoadFont(const std::string& name, std::string sourcePath)
    {
        fonts.Load(name, sourcePath, [&sourcePath](sf::Font& font) { return font.loadFromFile(sourcePath); });
    }

    const sf::Font* ResourceSystem::GetFont(const std::string& name) const
    {
        return fonts.Get(name);
    }

    void ResourceSystem::DeleteFont(const std::string& name)
    {
        fonts.Erase(name);
    }

    void ResourceSystem::LoadShader(const std::string& name, std::string sourcePath, sf::Shader::Type type)
    {
        if (!sf::Shader::isAvailable())
        {
            LOG_WARN("Shaders are not supported by this graphics card: " + name);
            return;
        }

        shaders.Load(name, sourcePath, [&sourcePath, type](sf::Shader& shader) { return shader.loadFromFile(sourcePath, type); });
    }

    // Шейдер необязателен: без него компонент просто рисует без эффекта, поэтому молча.
    sf::Shader* ResourceSystem::GetShader(const std::string& name) const
    {
        return shaders.Find(name);
    }

    void ResourceSystem::DeleteShader(const std::string& name)
    {
        shaders.Erase(name);
    }

    void ResourceSystem::Clear()
    {
        textures.Clear();
        textureMaps.clear();
        sounds.Clear();
        musicTracks.Clear();
        fonts.Clear();
        shaders.Clear();
    }
}
