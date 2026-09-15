#include "pch.h"
#include "SettleComponent.h"
#include <BoxColliderComponent.h>
#include <GameWorld.h>
#include <MovementComponent.h>
#include <RectangleRendererComponent.h>
#include <RenderSystem.h>

using namespace XYZEngine;
using RoguelikeGame::SettleComponent;

namespace
{
	class TickCounterComponent : public Component
	{
	public:
		TickCounterComponent(GameObject* gameObject) : Component(gameObject) {}

		void Start() override {}
		void Update(float deltaTime) override { ticks++; }
		void Render() override { draws++; }

		int ticks = 0;
		int draws = 0;
	};

	class SettleTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();
			isReady = false;
		}

		void TearDown() override
		{
			GameWorld::Instance()->Clear();
		}

		GameObject* CreateBody(const std::string& name)
		{
			GameObject* body = GameWorld::Instance()->CreateGameObject(name);
			body->AddComponent<MovementComponent>()->SetSpeed(100.f);
			body->AddComponent<TickCounterComponent>();
			body->AddComponent<RectangleRendererComponent>()->SetSize(32.f, 32.f);

			settle = body->AddComponent<SettleComponent>();
			settle->SetReadyCheck([this]() { return isReady; });

			return body;
		}

		void Run(int frames)
		{
			for (int frame = 0; frame < frames; frame++)
			{
				GameWorld::Instance()->Update(0.05f);
			}
		}

		SettleComponent* settle = nullptr;
		bool isReady = false;
	};
}

TEST_F(SettleTest, BodyKeepsWorkingUntilItIsReady)
{
	GameObject* body = CreateBody("Body");

	Run(5);

	EXPECT_FALSE(settle->IsSettled());
	EXPECT_EQ(body->GetComponent<TickCounterComponent>()->ticks, 5);
}

TEST_F(SettleTest, ReadyBodyStopsSpendingFrames)
{
	GameObject* body = CreateBody("Body");
	Run(3);
	isReady = true;
	Run(1);

	int before = body->GetComponent<TickCounterComponent>()->ticks;
	Run(10);

	EXPECT_TRUE(settle->IsSettled());
	EXPECT_EQ(body->GetComponent<TickCounterComponent>()->ticks, before);
}

TEST_F(SettleTest, SettledBodyIsStillDrawn)
{
	GameObject* body = CreateBody("Body");
	isReady = true;
	Run(1);
	ASSERT_TRUE(settle->IsSettled());

	auto counter = body->GetComponent<TickCounterComponent>();
	counter->draws = 0;
	RenderSystem::Instance()->ResetFrameStats();
	GameWorld::Instance()->Render();

	EXPECT_GT(RenderSystem::Instance()->GetDrawnCount(), 0);
	EXPECT_EQ(counter->draws, 0);
	EXPECT_TRUE(body->IsActive());
}

TEST_F(SettleTest, SettleTurnsOffTheWorkingParts)
{
	GameObject* body = CreateBody("Body");
	isReady = true;
	Run(1);

	EXPECT_FALSE(body->GetComponent<MovementComponent>()->IsEnabled());
	EXPECT_FALSE(body->GetComponent<TickCounterComponent>()->IsEnabled());
	EXPECT_FALSE(settle->IsEnabled());
}

TEST_F(SettleTest, TransformStaysOn)
{
	GameObject* body = CreateBody("Body");
	isReady = true;
	Run(1);

	EXPECT_TRUE(body->GetTransform()->IsEnabled());
}

TEST_F(SettleTest, ChildrenSettleTogetherWithTheBody)
{
	GameObject* body = CreateBody("Body");
	GameObject* held = GameWorld::Instance()->CreateGameObject("Weapon", body);
	held->AddComponent<TickCounterComponent>();
	held->AddComponent<RectangleRendererComponent>()->SetSize(16.f, 16.f);

	isReady = true;
	Run(1);

	EXPECT_FALSE(held->GetComponent<TickCounterComponent>()->IsEnabled());
	EXPECT_TRUE(held->GetComponent<RectangleRendererComponent>()->IsEnabled());
}

TEST_F(SettleTest, SettlingTwiceChangesNothing)
{
	GameObject* body = CreateBody("Body");
	isReady = true;
	Run(1);
	ASSERT_TRUE(settle->IsSettled());

	settle->Settle();

	EXPECT_TRUE(settle->IsSettled());
	EXPECT_TRUE(body->GetComponent<RectangleRendererComponent>()->IsEnabled());
}

TEST_F(SettleTest, BodyWithoutACheckSettlesAtOnce)
{
	GameObject* body = CreateBody("Body");
	settle->SetReadyCheck(nullptr);

	Run(1);

	EXPECT_TRUE(settle->IsSettled());
	EXPECT_FALSE(body->GetComponent<MovementComponent>()->IsEnabled());
}
