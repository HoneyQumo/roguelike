#include "pch.h"
#include "GameWorld.h"
#include "MovementComponent.h"

using XYZEngine::Component;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;

namespace
{
	class CounterComponent : public Component
	{
	public:
		CounterComponent(GameObject* gameObject) : Component(gameObject) {}

		void Start() override { startCount++; }
		void Update(float deltaTime) override { updateCount++; }
		void Render() override { renderCount++; }

		int startCount = 0;
		int updateCount = 0;
		int renderCount = 0;
		int enableCount = 0;
		int disableCount = 0;
	protected:
		void OnEnable() override { enableCount++; }
		void OnDisable() override { disableCount++; }
	};

	class ComponentTest : public ::testing::Test
	{
	protected:
		void SetUp() override { GameWorld::Instance()->Clear(); }
		void TearDown() override { GameWorld::Instance()->Clear(); }
	};
}

TEST_F(ComponentTest, NewComponentIsEnabled)
{
	GameObject* owner = GameWorld::Instance()->CreateGameObject("Owner");
	CounterComponent* counter = owner->AddComponent<CounterComponent>();

	EXPECT_TRUE(counter->IsEnabled());
}

TEST_F(ComponentTest, DisabledComponentSkipsUpdateAndRender)
{
	GameObject* owner = GameWorld::Instance()->CreateGameObject("Owner");
	CounterComponent* counter = owner->AddComponent<CounterComponent>();

	owner->Update(0.1f);
	owner->Render();
	counter->SetEnabled(false);
	owner->Update(0.1f);
	owner->Render();

	EXPECT_EQ(counter->updateCount, 1);
	EXPECT_EQ(counter->renderCount, 1);
}

TEST_F(ComponentTest, EnablingBackResumesUpdate)
{
	GameObject* owner = GameWorld::Instance()->CreateGameObject("Owner");
	CounterComponent* counter = owner->AddComponent<CounterComponent>();

	counter->SetEnabled(false);
	owner->Update(0.1f);
	counter->SetEnabled(true);
	owner->Update(0.1f);

	EXPECT_EQ(counter->updateCount, 1);
	EXPECT_TRUE(counter->IsEnabled());
}

TEST_F(ComponentTest, DisabledComponentDoesNotStart)
{
	GameObject* owner = GameWorld::Instance()->CreateGameObject("Owner");
	CounterComponent* counter = owner->AddComponent<CounterComponent>();
	counter->SetEnabled(false);

	owner->Update(0.1f);
	EXPECT_EQ(counter->startCount, 0);

	counter->SetEnabled(true);
	owner->Update(0.1f);
	EXPECT_EQ(counter->startCount, 1);
}

TEST_F(ComponentTest, StartRunsOnlyOnceAfterToggling)
{
	GameObject* owner = GameWorld::Instance()->CreateGameObject("Owner");
	CounterComponent* counter = owner->AddComponent<CounterComponent>();

	owner->Update(0.1f);
	counter->SetEnabled(false);
	owner->Update(0.1f);
	counter->SetEnabled(true);
	owner->Update(0.1f);

	EXPECT_EQ(counter->startCount, 1);
}

TEST_F(ComponentTest, CallbacksRunOnlyOnStateChange)
{
	GameObject* owner = GameWorld::Instance()->CreateGameObject("Owner");
	CounterComponent* counter = owner->AddComponent<CounterComponent>();

	counter->SetEnabled(true);
	EXPECT_EQ(counter->enableCount, 0);
	EXPECT_EQ(counter->disableCount, 0);

	counter->SetEnabled(false);
	counter->SetEnabled(false);
	EXPECT_EQ(counter->disableCount, 1);

	counter->SetEnabled(true);
	EXPECT_EQ(counter->enableCount, 1);
}

TEST_F(ComponentTest, MovementForgetsDirectionWhenDisabled)
{
	GameObject* owner = GameWorld::Instance()->CreateGameObject("Owner");
	XYZEngine::MovementComponent* movement = owner->AddComponent<XYZEngine::MovementComponent>();
	movement->SetSpeed(100.f);
	movement->SetDirection({1.f, 0.f});

	owner->Update(0.1f);
	EXPECT_FLOAT_EQ(owner->GetTransform()->GetWorldPosition().x, 10.f);

	movement->SetEnabled(false);
	owner->Update(0.1f);

	EXPECT_FLOAT_EQ(owner->GetTransform()->GetWorldPosition().x, 10.f);
	EXPECT_TRUE(movement->GetDirection().IsZero());
	EXPECT_FALSE(movement->IsRunning());
}
