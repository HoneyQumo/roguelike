#include "pch.h"
#include "EventList.h"

using XYZEngine::EventList;
using XYZEngine::NO_SUBSCRIPTION;
using XYZEngine::SubscriptionId;

TEST(EventListTests, InvokeCallsEverySubscriber)
{
	EventList<int> event;
	int first = 0;
	int second = 0;
	event.Subscribe([&first](const int& value) { first += value; });
	event.Subscribe([&second](const int& value) { second += value; });

	event.Invoke(3);

	EXPECT_EQ(event.GetCount(), 2u);
	EXPECT_EQ(first, 3);
	EXPECT_EQ(second, 3);
}

TEST(EventListTests, UnsubscribeRemovesOnlyOneSubscriber)
{
	EventList<int> event;
	int kept = 0;
	int removed = 0;
	SubscriptionId keptId = event.Subscribe([&kept](const int&) { kept++; });
	SubscriptionId removedId = event.Subscribe([&removed](const int&) { removed++; });

	event.Unsubscribe(removedId);
	event.Invoke(1);

	EXPECT_NE(keptId, removedId);
	EXPECT_EQ(event.GetCount(), 1u);
	EXPECT_EQ(kept, 1);
	EXPECT_EQ(removed, 0);
}

TEST(EventListTests, SameHandlerTwiceGetsDifferentIds)
{
	EventList<int> event;
	int calls = 0;
	auto handler = [&calls](const int&) { calls++; };

	SubscriptionId first = event.Subscribe(handler);
	SubscriptionId second = event.Subscribe(handler);
	event.Unsubscribe(first);
	event.Invoke(1);

	EXPECT_NE(first, second);
	EXPECT_EQ(calls, 1);
}

TEST(EventListTests, UnknownIdChangesNothing)
{
	EventList<int> event;
	int calls = 0;
	SubscriptionId id = event.Subscribe([&calls](const int&) { calls++; });

	event.Unsubscribe(NO_SUBSCRIPTION);
	event.Unsubscribe(id + 100);
	event.Invoke(1);

	EXPECT_EQ(event.GetCount(), 1u);
	EXPECT_EQ(calls, 1);
}

TEST(EventListTests, EmptyHandlerIsNotStored)
{
	EventList<int> event;

	SubscriptionId id = event.Subscribe(nullptr);

	EXPECT_EQ(id, NO_SUBSCRIPTION);
	EXPECT_EQ(event.GetCount(), 0u);
}

TEST(EventListTests, ClearRemovesEverySubscriber)
{
	EventList<int> event;
	int calls = 0;
	event.Subscribe([&calls](const int&) { calls++; });
	event.Subscribe([&calls](const int&) { calls++; });

	event.Clear();
	event.Invoke(1);

	EXPECT_EQ(event.GetCount(), 0u);
	EXPECT_EQ(calls, 0);
}
