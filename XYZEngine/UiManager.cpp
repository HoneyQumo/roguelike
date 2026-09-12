#include "pch.h"
#include "UiManager.h"
#include "InputSystem.h"
#include "RenderSystem.h"
#include <algorithm>

namespace XYZEngine
{
	UiManager* UiManager::Instance()
	{
		static UiManager manager;
		return &manager;
	}

	UiManager::UiManager()
	{
		resizeSubscription = RenderSystem::Instance()->SubscribeResize([this](unsigned int, unsigned int) { ResizeScreens(); });
	}

	void UiManager::Push(UiScreen* screen)
	{
		if (screen == nullptr)
		{
			return;
		}

		Remove(screen);
		screens.push_back(screen);
		screen->Resize(GetScreenSize());
	}

	void UiManager::Pop()
	{
		if (screens.empty())
		{
			return;
		}

		screens.pop_back();
	}

	void UiManager::Remove(UiScreen* screen)
	{
		screens.erase(std::remove(screens.begin(), screens.end(), screen), screens.end());
	}

	void UiManager::Clear()
	{
		screens.clear();
		isPointerCaptured = false;
	}

	UiScreen* UiManager::GetTop() const
	{
		return screens.empty() ? nullptr : screens.back();
	}

	std::size_t UiManager::GetCount() const
	{
		return screens.size();
	}

	sf::Vector2f UiManager::GetScreenSize() const
	{
		sf::Vector2u windowSize = RenderSystem::Instance()->GetWindowSize();
		return {static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)};
	}

	void UiManager::ResizeScreens()
	{
		sf::Vector2f screenSize = GetScreenSize();
		for (UiScreen* screen : screens)
		{
			screen->Resize(screenSize);
		}
	}

	void UiManager::HandleInput()
	{
		isPointerCaptured = false;

		UiScreen* top = GetTop();
		if (top == nullptr || !top->IsVisible())
		{
			return;
		}

		auto input = InputSystem::Instance();
		sf::Vector2i mousePosition = input->GetMousePosition();
		sf::Vector2f point = {static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y)};

		isPointerCaptured = top->HandlePointer(point, input->IsButtonHeld(sf::Mouse::Left),
			input->WasButtonReleased(sf::Mouse::Left));
	}

	void UiManager::Update(float deltaTime)
	{
		for (UiScreen* screen : screens)
		{
			screen->Update(deltaTime);
		}
	}

	void UiManager::Render()
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

	bool UiManager::IsPointerCaptured() const
	{
		return isPointerCaptured;
	}
}
