#pragma once

#include <vector>
#include <SFML/System/Vector2.hpp>
#include "EventList.h"
#include "UiScreen.h"

namespace XYZEngine
{
	class UiManager
	{
	public:
		static UiManager* Instance();

		void Push(UiScreen* screen);
		void Pop();
		void Remove(UiScreen* screen);
		void Clear();

		UiScreen* GetTop() const;
		UiScreen* GetTopVisible() const;
		std::size_t GetCount() const;

		void HandleInput();
		void Update(float deltaTime);
		void Render();

		bool IsPointerCaptured() const;

		void CaptureInput();
		bool IsInputCaptured() const;

	private:
		std::vector<UiScreen*> screens;
		SubscriptionId resizeSubscription = NO_SUBSCRIPTION;
		bool isPointerCaptured = false;
		bool isInputCaptured = false;

		UiManager();
		~UiManager() {}

		UiManager(UiManager const&) = delete;
		UiManager& operator=(UiManager const&) = delete;

		void ResizeScreens();
		sf::Vector2f GetScreenSize() const;
	};
}
