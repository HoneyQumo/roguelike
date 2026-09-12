#pragma once

#include <SFML/Graphics.hpp>
#include "EventList.h"

namespace XYZEngine
{
	class RenderSystem
	{
	public:
		static RenderSystem* Instance();

		void SetMainWindow(sf::RenderWindow* newWindow);
		sf::RenderWindow& GetMainWindow() const;

		void HandleResize(unsigned int width, unsigned int height);
		sf::Vector2u GetWindowSize() const;
		const sf::View& GetUiView() const;

		SubscriptionId SubscribeResize(std::function<void(unsigned int, unsigned int)> handler);
		void UnsubscribeResize(SubscriptionId id);

		void Render(const sf::Drawable& drawable);
		void Render(const sf::Drawable& drawable, const sf::RenderStates& states);

	private:
		sf::RenderWindow* window = nullptr;
		sf::Vector2u windowSize = {0, 0};
		sf::View uiView = sf::View(sf::FloatRect(0.f, 0.f, 0.f, 0.f));
		EventList<unsigned int, unsigned int> resizeEvent;

		RenderSystem() {}
		~RenderSystem() {}

		RenderSystem(RenderSystem const&) = delete;
		RenderSystem& operator= (RenderSystem const&) = delete;
	};
}