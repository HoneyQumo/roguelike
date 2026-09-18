#include "pch.h"
#include "AimRotationComponent.h"
#include "EnemyCatalog.h"
#include "GameWorld.h"
#include "MathUtils.h"

using XYZEngine::AimRotationComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::ShortestAngleDegrees;
using XYZEngine::TurnTowardsDegrees;

TEST(ShortestAngleTest, TheWayRoundIsAlwaysTheShortOne)
{
	EXPECT_FLOAT_EQ(ShortestAngleDegrees(0.f, 90.f), 90.f);
	EXPECT_FLOAT_EQ(ShortestAngleDegrees(0.f, -90.f), -90.f);
	EXPECT_FLOAT_EQ(ShortestAngleDegrees(0.f, 270.f), -90.f) << "разворот пошёл длинной дорогой";
	EXPECT_FLOAT_EQ(ShortestAngleDegrees(350.f, 10.f), 20.f) << "переход через ноль сломал угол";
}

TEST(TurnTowardsTest, AStepBiggerThanTheGapLandsExactly)
{
	EXPECT_FLOAT_EQ(TurnTowardsDegrees(0.f, 30.f, 90.f), 30.f);
}

TEST(TurnTowardsTest, ABigGapIsCoveredInSteps)
{
	EXPECT_FLOAT_EQ(TurnTowardsDegrees(0.f, 180.f, 45.f), 45.f);
	EXPECT_FLOAT_EQ(TurnTowardsDegrees(0.f, -180.f, 45.f), -45.f);
}

// Ноль означает «без ограничения»: так ведут себя все, кому скорость не задали.
TEST(TurnTowardsTest, NoSpeedMeansInstantly)
{
	EXPECT_FLOAT_EQ(TurnTowardsDegrees(0.f, 170.f, 0.f), 170.f);
	EXPECT_FLOAT_EQ(TurnTowardsDegrees(0.f, 170.f, -5.f), 170.f);
}

TEST(TurnTowardsTest, TurningRoundZeroDoesNotGoTheLongWay)
{
	float angle = TurnTowardsDegrees(350.f, 10.f, 5.f);

	EXPECT_FLOAT_EQ(angle, 355.f) << "разворот через ноль поехал в обратную сторону";
}

namespace
{
	class AimTurnTest : public ::testing::Test
	{
	protected:
		void SetUp() override { GameWorld::Instance()->Clear(); }
		void TearDown() override { GameWorld::Instance()->Clear(); }

		AimRotationComponent* CreateAim(float turnSpeed)
		{
			GameObject* object = GameWorld::Instance()->CreateGameObject("Turner");
			object->GetTransform()->SetWorldPosition({0.f, 0.f});
			object->GetTransform()->SetWorldRotation(0.f);

			auto aim = object->AddComponent<AimRotationComponent>();
			aim->SetMaxDistance(0.f);
			aim->SetTurnSpeed(turnSpeed);

			// Цель строго позади: разворот на 180 градусов.
			aim->AimAtPoint({-100.f, 0.f});

			return aim;
		}
	};
}

// Мышь игрока не должна ждать корпус - ему скорость не задают.
TEST_F(AimTurnTest, WithoutASpeedTheBodySnapsToTheAim)
{
	AimRotationComponent* aim = CreateAim(0.f);

	aim->Update(0.016f);

	EXPECT_NEAR(std::fabs(aim->GetGameObject()->GetTransform()->GetWorldRotation()), 180.f, 0.01f);
}

TEST_F(AimTurnTest, ABodyWithASpeedNeedsTimeToTurnAround)
{
	AimRotationComponent* aim = CreateAim(180.f);

	aim->Update(0.1f);

	float angle = aim->GetGameObject()->GetTransform()->GetWorldRotation();

	EXPECT_NEAR(std::fabs(angle), 18.f, 0.01f) << "корпус развернулся быстрее заданного";

	for (int frame = 0; frame < 100; frame++)
	{
		aim->Update(0.1f);
	}

	EXPECT_NEAR(std::fabs(aim->GetGameObject()->GetTransform()->GetWorldRotation()), 180.f, 0.01f)
		<< "корпус так и не довернулся";
}

// Числа в каталоге - не украшение: нулевой разворот вернул бы мгновенную спину.
TEST(EnemyCatalogTest, EveryEnemyTurnsAtAHumanPace)
{
	for (const RoguelikeGame::EnemyDefinition& enemy : RoguelikeGame::ENEMIES)
	{
		EXPECT_GT(enemy.config.turnSpeed, 0.f) << enemy.tileName << " разворачивается мгновенно";
		EXPECT_LT(enemy.config.turnSpeed, 400.f) << enemy.tileName << " крутится как башня танка";
	}
}
