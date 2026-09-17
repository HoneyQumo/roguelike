#pragma once

#include <SFML/Graphics.hpp>
#include "EventList.h"
#include "ViewCulling.h"

namespace XYZEngine
{
	class RenderSystem
	{
	public:
		static RenderSystem* Instance();

		void SetMainWindow(sf::RenderWindow* newWindow);
		sf::RenderWindow& GetMainWindow() const;

		void HandleResize(unsigned int width, unsigned int height);
		void HandleFocus(bool isFocused);
		void HoldMouse(bool isWanted);
		bool IsMouseHeld() const;
		sf::Vector2u GetWindowSize() const;
		const sf::View& GetUiView() const;

		SubscriptionId SubscribeResize(std::function<void(unsigned int, unsigned int)> handler);
		void UnsubscribeResize(SubscriptionId id);

		void BeginUiPass();
		void EndUiPass();

		void Render(const sf::Drawable& drawable);
		void Render(const sf::Drawable& drawable, const sf::RenderStates& states);

		sf::FloatRect GetViewArea() const;
		bool IsVisible(const sf::FloatRect& bounds) const;

		void ResetFrameStats();
		int GetDrawnCount() const;
		// Вызовов отрисовки десятки, а вершин десятки тысяч: узкое место видно только по ним.
		int GetVertexCount() const;
		void CountVertices(int count);
		int GetCulledCount() const;

	private:
		sf::RenderWindow* window = nullptr;
		bool isMouseWanted = true;
		bool isFocused = true;
		sf::Vector2u windowSize = {0, 0};
		sf::View uiView = sf::View(sf::FloatRect(0.f, 0.f, 0.f, 0.f));
		sf::View savedView = sf::View(sf::FloatRect(0.f, 0.f, 0.f, 0.f));
		bool isUiPass = false;
		int drawnCount = 0;
		int vertexCount = 0;
		mutable int culledCount = 0;
		EventList<unsigned int, unsigned int> resizeEvent;

		void ApplyMouseHold();

		RenderSystem() {}
		~RenderSystem() {}

		RenderSystem(RenderSystem const&) = delete;
		RenderSystem& operator= (RenderSystem const&) = delete;
	};
}