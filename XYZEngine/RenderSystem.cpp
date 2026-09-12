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

		LOG_INFO("Main window is set");
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
		GetMainWindow().draw(drawable);
	}
	void RenderSystem::Render(const sf::Drawable& drawable, const sf::RenderStates& states)
	{
		GetMainWindow().draw(drawable, states);
	}
}