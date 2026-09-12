#include "pch.h"
#include "UiRootComponent.h"
#include "GameObject.h"
#include "RenderSystem.h"
#include <algorithm>

namespace XYZEngine
{
	UiRootComponent::UiRootComponent(GameObject* gameObject) : Component(gameObject)
	{
		resizeSubscription = RenderSystem::Instance()->SubscribeResize([this](unsigned int, unsigned int) { ResizeScreens(); });
	}

	UiRootComponent::~UiRootComponent()
	{
		RenderSystem::Instance()->UnsubscribeResize(resizeSubscription);
	}

	void UiRootComponent::AddScreen(UiScreen* screen)
	{
		if (screen == nullptr)
		{
			return;
		}

		screens.push_back(screen);

		sf::Vector2u windowSize = RenderSystem::Instance()->GetWindowSize();
		screen->Resize({static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)});
	}

	void UiRootComponent::RemoveScreen(UiScreen* screen)
	{
		screens.erase(std::remove(screens.begin(), screens.end(), screen), screens.end());
	}

	std::size_t UiRootComponent::GetScreensCount() const
	{
		return screens.size();
	}

	void UiRootComponent::ResizeScreens()
	{
		sf::Vector2u windowSize = RenderSystem::Instance()->GetWindowSize();
		sf::Vector2f screenSize = {static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)};

		for (UiScreen* screen : screens)
		{
			screen->Resize(screenSize);
		}
	}

	void UiRootComponent::Update(float deltaTime)
	{
		for (UiScreen* screen : screens)
		{
			screen->Update(deltaTime);
		}
	}

	void UiRootComponent::Render()
	{
		if (screens.empty())
		{
			return;
		}

		RenderSystem::Instance()->BeginUiPass();

		for (UiScreen* screen : screens)
		{
			screen->Draw();
		}

		RenderSystem::Instance()->EndUiPass();
	}
}
