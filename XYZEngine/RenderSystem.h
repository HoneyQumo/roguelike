#pragma once

#include <SFML/Graphics.hpp>

namespace XYZEngine
{
	class RenderSystem
	{
	public:
		static RenderSystem* Instance();

		void SetMainWindow(sf::RenderWindow* newWindow);
		sf::RenderWindow& GetMainWindow() const;

		void Render(const sf::Drawable& drawable);
		void Render(const sf::Drawable& drawable, const sf::RenderStates& states);

	private:
		sf::RenderWindow* window;

		RenderSystem() {}
		~RenderSystem() {}

		RenderSystem(RenderSystem const&) = delete;
		RenderSystem& operator= (RenderSystem const&) = delete;
	};
}