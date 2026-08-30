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
        if (textures.find(name) != textures.end())
        {
            LOG_WARN("Texture is already loaded: " + name);
            return;
        }

        auto newTexture = new sf::Texture();
        if (newTexture->loadFromFile(sourcePath))
        {
            newTexture->setSmooth(isSmooth);
            textures.emplace(name, newTexture);
            LOG_INFO("Texture loaded: " + name + " from " + sourcePath);
            return;
        }

        LOG_ERROR("Can't load texture: " + sourcePath);
        delete newTexture;
    }

    void ResourceSystem::LoadTexturePart(const std::string& name, std::string sourcePath, sf::IntRect area, bool isSmooth)
    {
        assert(area.width > 0 && area.height > 0);

        if (textures.find(name) != textures.end())
        {
            LOG_WARN("Texture is already loaded: " + name);
            return;
        }

        auto newTexture = new sf::Texture();
        if (newTexture->loadFromFile(sourcePath, area))
        {
            newTexture->setSmooth(isSmooth);
            textures.emplace(name, newTexture);
            LOG_INFO("Texture part loaded: " + name + " from " + sourcePath);
            return;
        }

        LOG_ERROR("Can't load texture part: " + sourcePath);
        delete newTexture;
    }

    const sf::Texture* ResourceSystem::GetTextureShared(const std::string& name) const
    {
        auto texturePair = textures.find(name);
        if (texturePair == textures.end())
        {
            LOG_ERROR("Texture not found: " + name);
            return nullptr;
        }

        return texturePair->second;
    }

    sf::Texture* ResourceSystem::GetTextureCopy(const std::string& name) const
    {
        const sf::Texture* texture = GetTextureShared(name);
        if (texture == nullptr)
        {
            return nullptr;
        }

        return new sf::Texture(*texture);
    }

    void ResourceSystem::DeleteSharedTexture(const std::string& name)
    {
        auto texturePair = textures.find(name);

        sf::Texture* deletingTexure = texturePair->second;
        textures.erase(texturePair);
        delete deletingTexure;
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

        const auto& elements = textureMap->second;
        if (elementIndex < 0 || elementIndex >= static_cast<int>(elements.size()))
        {
            LOG_ERROR("Texture map element out of range: " + name + ", index " + std::to_string(elementIndex));
            return nullptr;
        }

        return elements[elementIndex];
    }

    sf::Texture* ResourceSystem::GetTextureMapElementCopy(const std::string& name, int elementIndex) const
    {
        const sf::Texture* element = GetTextureMapElementShared(name, elementIndex);
        if (element == nullptr)
        {
            return nullptr;
        }

        return new sf::Texture(*element);
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
        auto textureMap = textureMaps.find(name);
        auto deletingTextures = textureMap->second;

        for (int i = 0; i < deletingTextures.size(); i++)
        {
            delete deletingTextures[i];
        }

        textureMaps.erase(textureMap);
    }

    void ResourceSystem::CutTextureMap(const std::string& name, const sf::Image& source, const std::vector<sf::IntRect>& areas, bool isSmooth)
    {
        std::vector<sf::Texture*> elements;
        elements.reserve(areas.size());

        for (const auto& area : areas)
        {
            auto newElement = new sf::Texture();
            if (!newElement->loadFromImage(source, area))
            {
                LOG_ERROR("Can't cut texture map element: " + name + ", index " + std::to_string(elements.size()));
                delete newElement;
                continue;
            }

            newElement->setSmooth(isSmooth);
            elements.push_back(newElement);
        }

        textureMaps.emplace(name, elements);
        LOG_INFO("Texture map loaded: " + name + ", elements: " + std::to_string(elements.size()));
    }

    void ResourceSystem::LoadSound(const std::string& name, std::string sourcePath)
    {
        if (sounds.find(name) != sounds.end())
        {
            return;
        }

        auto newSound = new sf::SoundBuffer();
        if (newSound->loadFromFile(sourcePath))
        {
            sounds.emplace(name, newSound);
            LOG_INFO("Sound loaded: " + name + " from " + sourcePath);
            return;
        }

        LOG_ERROR("Can't load sound: " + sourcePath);
        delete newSound;
    }

    const sf::SoundBuffer* ResourceSystem::GetSound(const std::string& name) const
    {
        auto soundPair = sounds.find(name);
        if (soundPair == sounds.end())
        {
            LOG_ERROR("Sound not found: " + name);
            return nullptr;
        }
        return soundPair->second;
    }

    void ResourceSystem::DeleteSound(const std::string& name)
    {
        auto soundPair = sounds.find(name);
        if (soundPair == sounds.end())
        {
            return;
        }

        sf::SoundBuffer* deletingSound = soundPair->second;
        sounds.erase(soundPair);
        delete deletingSound;
    }

    void ResourceSystem::LoadMusic(const std::string& name, std::string sourcePath)
    {
        if (musicTracks.find(name) != musicTracks.end())
        {
            return;
        }

        auto newMusic = new sf::Music();
        if (newMusic->openFromFile(sourcePath))
        {
            musicTracks.emplace(name, newMusic);
            LOG_INFO("Music opened: " + name + " from " + sourcePath);
            return;
        }

        LOG_ERROR("Can't open music: " + sourcePath);
        delete newMusic;
    }

    sf::Music* ResourceSystem::GetMusic(const std::string& name) const
    {
        auto musicPair = musicTracks.find(name);
        if (musicPair == musicTracks.end())
        {
            LOG_ERROR("Music not found: " + name);
            return nullptr;
        }
        return musicPair->second;
    }

    void ResourceSystem::DeleteMusic(const std::string& name)
    {
        auto musicPair = musicTracks.find(name);
        if (musicPair == musicTracks.end())
        {
            return;
        }

        sf::Music* deletingMusic = musicPair->second;
        musicTracks.erase(musicPair);

        deletingMusic->stop();
        delete deletingMusic;
    }

    void ResourceSystem::LoadShader(const std::string& name, std::string sourcePath, sf::Shader::Type type)
    {
        if (shaders.find(name) != shaders.end())
        {
            LOG_WARN("Shader is already loaded: " + name);
            return;
        }

        if (!sf::Shader::isAvailable())
        {
            LOG_WARN("Shaders are not supported by this graphics card: " + name);
            return;
        }

        auto newShader = new sf::Shader();
        if (newShader->loadFromFile(sourcePath, type))
        {
            shaders.emplace(name, newShader);
            LOG_INFO("Shader loaded: " + name + " from " + sourcePath);
            return;
        }

        LOG_ERROR("Can't load shader: " + sourcePath);
        delete newShader;
    }

    sf::Shader* ResourceSystem::GetShader(const std::string& name) const
    {
        auto shaderPair = shaders.find(name);
        if (shaderPair == shaders.end())
        {
            return nullptr;
        }
        return shaderPair->second;
    }

    void ResourceSystem::DeleteShader(const std::string& name)
    {
        auto shaderPair = shaders.find(name);
        if (shaderPair == shaders.end())
        {
            return;
        }

        sf::Shader* deletingShader = shaderPair->second;
        shaders.erase(shaderPair);
        delete deletingShader;
    }

    void ResourceSystem::Clear()
    {
        DeleteAllTextures();
        DeleteAllTextureMaps();
        DeleteAllSounds();
        DeleteAllMusic();
        DeleteAllShaders();
    }

    void ResourceSystem::DeleteAllTextures()
    {
        std::vector<std::string> keysToDelete;

        for (const auto& texturePair : textures)
        {
            keysToDelete.push_back(texturePair.first);
        }

        for (const auto& key : keysToDelete)
        {
            DeleteSharedTexture(key);
        }
    }

    void ResourceSystem::DeleteAllTextureMaps()
    {
        std::vector<std::string> keysToDelete;

        for (const auto& textureMapPair : textureMaps)
        {
            keysToDelete.push_back(textureMapPair.first);
        }

        for (const auto& key : keysToDelete)
        {
            DeleteSharedTextureMap(key);
        }
    }

    void ResourceSystem::DeleteAllSounds()
    {
        std::vector<std::string> keysToDelete;

        for (const auto& soundPair : sounds)
        {
            keysToDelete.push_back(soundPair.first);
        }

        for (const auto& key : keysToDelete)
        {
            DeleteSound(key);
        }
    }

    void ResourceSystem::DeleteAllMusic()
    {
        std::vector<std::string> keysToDelete;

        for (const auto& musicPair : musicTracks)
        {
            keysToDelete.push_back(musicPair.first);
        }

        for (const auto& key : keysToDelete)
        {
            DeleteMusic(key);
        }
    }

    void ResourceSystem::DeleteAllShaders()
    {
        std::vector<std::string> keysToDelete;

        for (const auto& shaderPair : shaders)
        {
            keysToDelete.push_back(shaderPair.first);
        }

        for (const auto& key : keysToDelete)
        {
            DeleteShader(key);
        }
    }
}
