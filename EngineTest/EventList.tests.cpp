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

TEST(EventListTests, HandlerCanUnsubscribeItselfDuringInvoke)
{
	EventList<int> event;
	int selfCalls = 0;
	int otherCalls = 0;
	SubscriptionId selfId = NO_SUBSCRIPTION;
	selfId = event.Subscribe([&event, &selfId, &selfCalls](const int&)
	{
		selfCalls++;
		event.Unsubscribe(selfId);
	});
	event.Subscribe([&otherCalls](const int&) { otherCalls++; });

	event.Invoke(1);
	event.Invoke(1);

	EXPECT_EQ(selfCalls, 1);
	EXPECT_EQ(otherCalls, 2);
	EXPECT_EQ(event.GetCount(), 1u);
}

TEST(EventListTests, HandlerUnsubscribedByAnotherHandlerIsNotCalled)
{
	EventList<int> event;
	int removedCalls = 0;
	SubscriptionId removedId = NO_SUBSCRIPTION;
	event.Subscribe([&event, &removedId](const int&) { event.Unsubscribe(removedId); });
	removedId = event.Subscribe([&removedCalls](const int&) { removedCalls++; });

	event.Invoke(1);

	EXPECT_EQ(removedCalls, 0);
	EXPECT_EQ(event.GetCount(), 1u);
}

TEST(EventListTests, HandlerSubscribedDuringInvokeIsCalledOnNextInvokeOnly)
{
	EventList<int> event;
	int addedCalls = 0;
	bool hasSubscribed = false;
	event.Subscribe([&event, &addedCalls, &hasSubscribed](const int&)
	{
		if (hasSubscribed)
		{
			return;
		}

		hasSubscribed = true;
		event.Subscribe([&addedCalls](const int&) { addedCalls++; });
	});

	event.Invoke(1);
	EXPECT_EQ(addedCalls, 0);

	event.Invoke(1);
	EXPECT_EQ(addedCalls, 1);
}
