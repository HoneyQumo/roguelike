#include "pch.h"
#include "RenderSystem.h"
#include "LoggerRegistry.h"
#include <cassert>

namespace XYZEngine
{
	RenderSystem* RenderSystem::Instance()
	{
		static RenderSystem render;
		return &render;
	}

	void RenderSystem::SetMainWindow(sf::RenderWindow* newWindow)
	{
		assert(newWindow != nullptr);

		if (newWindow == nullptr)
		{
			LOG_ERROR("Main window can't be null");
			return;
		}

		window = newWindow;
		HandleResize(window->getSize().x, window->getSize().y);
		ApplyMouseHold();

		LOG_INFO("Main window is set");
	}

	void RenderSystem::HandleFocus(bool newIsFocused)
	{
		isFocused = newIsFocused;
		ApplyMouseHold();
	}

	void RenderSystem::HoldMouse(bool isWanted)
	{
		isMouseWanted = isWanted;
		ApplyMouseHold();
	}

	bool RenderSystem::IsMouseHeld() const
	{
		return isMouseWanted && isFocused;
	}

	void RenderSystem::ApplyMouseHold()
	{
		if (window == nullptr)
		{
			return;
		}

		window->setMouseCursorGrabbed(IsMouseHeld());
	}

	void RenderSystem::HandleResize(unsigned int width, unsigned int height)
	{
		windowSize = {width, height};
		uiView = sf::View(sf::FloatRect(0.f, 0.f, static_cast<float>(width), static_cast<float>(height)));

		LOG_INFO("Window resized to " + std::to_string(width) + "x" + std::to_string(height));

		resizeEvent.Invoke(width, height);
	}
	sf::Vector2u RenderSystem::GetWindowSize() const
	{
		return windowSize;
	}
	const sf::View& RenderSystem::GetUiView() const
	{
		return uiView;
	}

	SubscriptionId RenderSystem::SubscribeResize(std::function<void(unsigned int, unsigned int)> handler)
	{
		return resizeEvent.Subscribe(std::move(handler));
	}
	void RenderSystem::UnsubscribeResize(SubscriptionId id)
	{
		resizeEvent.Unsubscribe(id);
	}
	sf::RenderWindow& RenderSystem::GetMainWindow() const
	{
		static sf::RenderWindow fallbackWindow;
		static bool isFallbackReported = false;

		if (window == nullptr)
		{
			if (!isFallbackReported)
			{
				isFallbackReported = true;
				LOG_ERROR("Main window is not set");
			}

			return fallbackWindow;
		}

		return *window;
	}

	void RenderSystem::BeginUiPass()
	{
		if (isUiPass)
		{
			return;
		}

		savedView = GetMainWindow().getView();
		GetMainWindow().setView(uiView);
		isUiPass = true;
	}

	void RenderSystem::EndUiPass()
	{
		if (!isUiPass)
		{
			return;
		}

		GetMainWindow().setView(savedView);
		isUiPass = false;
	}

	void RenderSystem::Render(const sf::Drawable& drawable)
	{
		drawnCount++;
		GetMainWindow().draw(drawable);
	}
	void RenderSystem::Render(const sf::Drawable& drawable, const sf::RenderStates& states)
	{
		drawnCount++;
		GetMainWindow().draw(drawable, states);
	}

	sf::FloatRect RenderSystem::GetViewArea() const
	{
		const sf::View& view = GetMainWindow().getView();
		sf::Vector2f center = view.getCenter();
		sf::Vector2f size = view.getSize();

		return NormalizedArea(sf::FloatRect(center.x - 0.5f * size.x, center.y - 0.5f * size.y, size.x, size.y));
	}

	bool RenderSystem::IsVisible(const sf::FloatRect& bounds) const
	{
		if (IsInView(bounds, GetViewArea(), VIEW_CULLING_MARGIN))
		{
			return true;
		}

		culledCount++;

		return false;
	}

	void RenderSystem::ResetFrameStats()
	{
		drawnCount = 0;
		culledCount = 0;
	}
	int RenderSystem::GetDrawnCount() const
	{
		return drawnCount;
	}
	int RenderSystem::GetCulledCount() const
	{
		return culledCount;
	}
}