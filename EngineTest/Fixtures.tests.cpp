#include "pch.h"
#include "Fixtures.h"
#include "HatchComponent.h"
#include "SwitchComponent.h"
#include <GameWorld.h>
#include <InputSystem.h>

using RoguelikeGame::HATCH_FRAMES;
using RoguelikeGame::HATCH_OPEN_TIME;
using RoguelikeGame::HatchComponent;
using RoguelikeGame::HatchFrame;
using RoguelikeGame::HatchState;
using RoguelikeGame::LeverFrame;
using RoguelikeGame::LinkSwitches;
using RoguelikeGame::NextHatchState;
using RoguelikeGame::SwitchComponent;

namespace
{
	class FixtureTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			XYZEngine::GameWorld::Instance()->Clear();
		}

		void TearDown() override
		{
			XYZEngine::GameWorld::Instance()->Clear();
		}

		SwitchComponent* CreateLever(const std::string& id)
		{
			XYZEngine::GameObject* object = XYZEngine::GameWorld::Instance()->CreateGameObject("Lever");
			auto lever = object->AddComponent<SwitchComponent>();
			lever->SetSwitchId(id);

			return lever;
		}

		HatchComponent* CreateHatch(const std::string& id)
		{
			XYZEngine::GameObject* object = XYZEngine::GameWorld::Instance()->CreateGameObject("Hatch");
			auto hatch = object->AddComponent<HatchComponent>();
			hatch->SetHatchId(id);

			return hatch;
		}

		void Run(float seconds)
		{
			for (float passed = 0.f; passed < seconds; passed += 0.05f)
			{
				XYZEngine::GameWorld::Instance()->Update(0.05f);
			}
		}
	};
}

TEST(HatchFramesTest, ShutHatchShowsTheFirstFrame)
{
	EXPECT_EQ(HatchFrame(HatchState::Shut, 0.f, HATCH_OPEN_TIME), 0);
	EXPECT_EQ(HatchFrame(HatchState::Shut, 5.f, HATCH_OPEN_TIME), 0);
}

TEST(HatchFramesTest, OpenHatchShowsTheLastFrame)
{
	EXPECT_EQ(HatchFrame(HatchState::Open, 0.f, HATCH_OPEN_TIME), HATCH_FRAMES - 1);
}

TEST(HatchFramesTest, FramesGoForwardWhileOpening)
{
	int previous = -1;
	for (int step = 0; step <= 10; step++)
	{
		float since = HATCH_OPEN_TIME * step / 10.f;
		int frame = HatchFrame(HatchState::Opening, since, HATCH_OPEN_TIME);

		EXPECT_GE(frame, previous);
		EXPECT_GE(frame, 0);
		EXPECT_LT(frame, HATCH_FRAMES);
		previous = frame;
	}

	EXPECT_EQ(previous, HATCH_FRAMES - 1);
}

TEST(HatchFramesTest, OpeningEndsWhenTheTimeIsUp)
{
	EXPECT_EQ(NextHatchState(HatchState::Opening, HATCH_OPEN_TIME * 0.5f, HATCH_OPEN_TIME), HatchState::Opening);
	EXPECT_EQ(NextHatchState(HatchState::Opening, HATCH_OPEN_TIME, HATCH_OPEN_TIME), HatchState::Open);
	EXPECT_EQ(NextHatchState(HatchState::Shut, 10.f, HATCH_OPEN_TIME), HatchState::Shut);
}

TEST(HatchFramesTest, LeverHasAFrameForEachPosition)
{
	EXPECT_EQ(LeverFrame(false), 0);
	EXPECT_EQ(LeverFrame(true), 1);
}

TEST_F(FixtureTest, AShutHatchOffersNothing)
{
	HatchComponent* hatch = CreateHatch("hatch_exit");

	EXPECT_FALSE(hatch->IsAvailable());
	EXPECT_EQ(hatch->GetState(), HatchState::Shut);
}

TEST_F(FixtureTest, TheLeverOpensTheHatchWithTheSameName)
{
	SwitchComponent* lever = CreateLever("hatch_exit");
	HatchComponent* hatch = CreateHatch("hatch_exit");
	LinkSwitches({lever}, {hatch});

	EXPECT_TRUE(lever->IsAvailable());
	lever->Interact(nullptr);

	EXPECT_TRUE(lever->IsPulled());
	EXPECT_EQ(hatch->GetState(), HatchState::Opening);

	Run(HATCH_OPEN_TIME + 0.2f);

	EXPECT_EQ(hatch->GetState(), HatchState::Open);
	EXPECT_TRUE(hatch->IsAvailable());
}

TEST_F(FixtureTest, TheLeverLeavesOtherHatchesAlone)
{
	SwitchComponent* lever = CreateLever("hatch_exit");
	HatchComponent* other = CreateHatch("hatch_vault");
	LinkSwitches({lever}, {other});

	lever->Interact(nullptr);

	EXPECT_EQ(other->GetState(), HatchState::Shut);
}

TEST_F(FixtureTest, ThePulledLeverIsDoneWithForever)
{
	SwitchComponent* lever = CreateLever("hatch_exit");

	EXPECT_TRUE(lever->Interact(nullptr));
	EXPECT_FALSE(lever->IsAvailable());
	EXPECT_FALSE(lever->Interact(nullptr));
}

TEST_F(FixtureTest, TheOpenHatchIsUsedOnceAndAsksForItsOwnKey)
{
	SwitchComponent* lever = CreateLever("hatch_exit");
	HatchComponent* hatch = CreateHatch("hatch_exit");
	LinkSwitches({lever}, {hatch});

	int fled = 0;
	hatch->SubscribeFled([&fled]() { fled++; });

	lever->Interact(nullptr);
	Run(HATCH_OPEN_TIME + 0.2f);

	EXPECT_EQ(hatch->GetAction(), XYZEngine::InputAction::Flee);
	EXPECT_TRUE(hatch->Interact(nullptr));
	EXPECT_FALSE(hatch->Interact(nullptr));
	EXPECT_EQ(fled, 1);
	EXPECT_FALSE(hatch->IsAvailable());
}

TEST_F(FixtureTest, TheHatchCanNotBeUsedBeforeItOpens)
{
	HatchComponent* hatch = CreateHatch("hatch_exit");

	int fled = 0;
	hatch->SubscribeFled([&fled]() { fled++; });

	EXPECT_FALSE(hatch->Interact(nullptr));
	EXPECT_EQ(fled, 0);
}
