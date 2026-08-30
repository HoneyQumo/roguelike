#pragma once

#include <map>
#include <string>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

namespace XYZEngine
{
	class ResourceSystem
	{
	public:
		static ResourceSystem* Instance();

		void LoadTexture(const std::string& name, std::string sourcePath, bool isSmooth = true);
		void LoadTexturePart(const std::string& name, std::string sourcePath, sf::IntRect area, bool isSmooth = true);
		const sf::Texture* GetTextureShared(const std::string& name) const;
		sf::Texture* GetTextureCopy(const std::string& name) const;
		void DeleteSharedTexture(const std::string& name);

		void LoadTextureMap(const std::string& name, std::string sourcePath, sf::Vector2u elementPixelSize, int totalElements, bool isSmooth = true);
		void LoadTextureStrip(const std::string& name, std::string sourcePath, sf::IntRect firstElementArea, int totalElements, bool isSmooth = true);
		const sf::Texture* GetTextureMapElementShared(const std::string& name, int elementIndex) const;
		sf::Texture* GetTextureMapElementCopy(const std::string& name, int elementIndex) const;
		int GetTextureMapElementsCount(const std::string& name) const;
		void DeleteSharedTextureMap(const std::string& name);

		void LoadSound(const std::string& name, std::string sourcePath);
		const sf::SoundBuffer* GetSound(const std::string& name) const;
		void DeleteSound(const std::string& name);

		void LoadMusic(const std::string& name, std::string sourcePath);
		sf::Music* GetMusic(const std::string& name) const;
		void DeleteMusic(const std::string& name);

		void LoadShader(const std::string& name, std::string sourcePath, sf::Shader::Type type);
		sf::Shader* GetShader(const std::string& name) const;
		void DeleteShader(const std::string& name);

		void Clear();

	private:
		std::map<std::string, sf::Texture*> textures;
		std::map<std::string, std::vector<sf::Texture*>> textureMaps;
		std::map<std::string, sf::SoundBuffer*> sounds;
		std::map<std::string, sf::Music*> musicTracks;
		std::map<std::string, sf::Shader*> shaders;

		ResourceSystem() {}
		~ResourceSystem() {}

		ResourceSystem(ResourceSystem const&) = delete;
		ResourceSystem& operator= (ResourceSystem const&) = delete;

		void CutTextureMap(const std::string& name, const sf::Image& source, const std::vector<sf::IntRect>& areas, bool isSmooth);

		void DeleteAllTextures();
		void DeleteAllTextureMaps();
		void DeleteAllSounds();
		void DeleteAllMusic();
		void DeleteAllShaders();
	};
}