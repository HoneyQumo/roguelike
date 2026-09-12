#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

namespace XYZEngine
{
	using SubscriptionId = std::size_t;
	constexpr SubscriptionId NO_SUBSCRIPTION = 0;

	template <typename... TArgs>
	class EventList
	{
	public:
		SubscriptionId Subscribe(std::function<void(TArgs...)> handler)
		{
			if (handler == nullptr)
			{
				return NO_SUBSCRIPTION;
			}

			subscriptions.push_back({nextId, std::move(handler)});
			return nextId++;
		}

		void Unsubscribe(SubscriptionId id)
		{
			if (id == NO_SUBSCRIPTION)
			{
				return;
			}

			subscriptions.erase(std::remove_if(subscriptions.begin(), subscriptions.end(),
				[id](const Subscription& subscription) { return subscription.id == id; }), subscriptions.end());
		}

		void Invoke(TArgs... args) const
		{
			for (std::size_t i = 0; i < subscriptions.size(); i++)
			{
				subscriptions[i].handler(args...);
			}
		}

		std::size_t GetCount() const
		{
			return subscriptions.size();
		}

		void Clear()
		{
			subscriptions.clear();
		}

	private:
		struct Subscription
		{
			SubscriptionId id;
			std::function<void(TArgs...)> handler;
		};

		std::vector<Subscription> subscriptions;
		SubscriptionId nextId = NO_SUBSCRIPTION + 1;
	};
}
