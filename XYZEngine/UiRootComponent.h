#pragma once

#include <vector>
#include "Component.h"
#include "EventList.h"
#include "UiScreen.h"

namespace XYZEngine
{
	class UiRootComponent : public Component
	{
	public:
		UiRootComponent(GameObject* gameObject);
		~UiRootComponent() override;

		void Update(float deltaTime) override;
		void Render() override;

		void AddScreen(UiScreen* screen);
		void RemoveScreen(UiScreen* screen);
		std::size_t GetScreensCount() const;

	private:
		std::vector<UiScreen*> screens;
		SubscriptionId resizeSubscription = NO_SUBSCRIPTION;

		void ResizeScreens();
	};
}
