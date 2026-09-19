#include "pch.h"
#include "FrameClock.h"
#include "SoundPool.h"

using XYZEngine::ChooseSoundSlot;
using XYZEngine::FrameClock;
using XYZEngine::SoundPool;
using XYZEngine::SoundSlot;

namespace
{
	constexpr int SHOT = 1;
	constexpr int VOICE = 2;

	SoundSlot Busy(int category, float startedAt, float endsAt)
	{
		SoundSlot slot;
		slot.category = category;
		slot.startedAt = startedAt;
		slot.endsAt = endsAt;

		return slot;
	}
}

TEST(SoundSlotTest, AFreeSlotIsTakenFirst)
{
	std::vector<SoundSlot> slots = {Busy(SHOT, 0.f, 10.f), Busy(SHOT, 1.f, 10.f), SoundSlot()};

	EXPECT_EQ(ChooseSoundSlot(slots, 5.f, SHOT), 2);
}

TEST(SoundSlotTest, ASlotThatJustFinishedCountsAsFree)
{
	std::vector<SoundSlot> slots = {Busy(SHOT, 0.f, 5.f)};

	EXPECT_EQ(ChooseSoundSlot(slots, 5.f, SHOT), 0);
}

// Очередь перебивает свой же хвост: из занятых уходит самый старый выстрел,
// а не тот, что начался только что.
TEST(SoundSlotTest, WhenFullTheOldestOfTheSameKindGivesWay)
{
	std::vector<SoundSlot> slots = {Busy(SHOT, 3.f, 10.f), Busy(SHOT, 1.f, 10.f), Busy(SHOT, 2.f, 10.f)};

	EXPECT_EQ(ChooseSoundSlot(slots, 5.f, SHOT), 1);
}

// Лучше промолчать, чем оборвать чужое: выстрел не съедает чужой крик.
TEST(SoundSlotTest, ANeighbourOfAnotherKindIsNotPushedOut)
{
	std::vector<SoundSlot> slots = {Busy(VOICE, 1.f, 10.f), Busy(VOICE, 2.f, 10.f)};

	EXPECT_EQ(ChooseSoundSlot(slots, 5.f, SHOT), -1);
}

TEST(SoundSlotTest, WithoutSlotsThereIsNowhereToPlay)
{
	std::vector<SoundSlot> slots;

	EXPECT_EQ(ChooseSoundSlot(slots, 5.f, SHOT), -1);
}

TEST(SoundPoolTest, ThePoolStartsWithPlacesToPlay)
{
	EXPECT_GT(SoundPool::Instance()->GetSlots(), 0u);
}

TEST(SoundPoolTest, PlayingNothingIsNotACrash)
{
	SoundPool::Instance()->PlayAtListener(nullptr, 100.f, SHOT);
	SoundPool::Instance()->PlayAt(nullptr, {10.f, 10.f}, 100.f, SHOT, 640.f, 1.f);

	for (const SoundSlot& slot : SoundPool::Instance()->GetBusySlots())
	{
		EXPECT_FLOAT_EQ(slot.endsAt, 0.f);
	}
}

// Звук не замедляется вместе с игрой, поэтому слоты считаются по времени
// без хит-стопа: иначе на каждом замедлении пул держит места дольше, чем надо.
TEST(SoundPoolTest, TheClockForSlotsIgnoresSlowMotion)
{
	FrameClock::Instance()->Reset();
	FrameClock::Instance()->SetTimeScale(0.25f);

	FrameClock::Instance()->Advance(0.1f);

	EXPECT_FLOAT_EQ(FrameClock::Instance()->GetUnscaledElapsedSeconds(), 0.1f);
	EXPECT_LT(FrameClock::Instance()->GetElapsedSeconds(), 0.1f);

	FrameClock::Instance()->Reset();
}
