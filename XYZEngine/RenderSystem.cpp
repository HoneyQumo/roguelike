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
		LOG_INFO("Main window is set");
	}
	sf::RenderWindow& RenderSystem::GetMainWindow() const
	{
		return *window;
	}

	void RenderSystem::Render(const sf::Drawable& drawable)
	{
		window->draw(drawable);
	}
}