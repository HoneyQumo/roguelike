#include "pch.h"
#include "CameraFollow.h"
#include "GameSettings.h"

using RoguelikeGame::ApproachPoint;
using RoguelikeGame::ClampAxis;
using RoguelikeGame::ClampToBounds;
using XYZEngine::Vector2Df;

// Камера догоняет, а не прыгает: иначе это та же жёсткая привязка.
TEST(CameraFollowTest, TheCameraCoversOnlyPartOfTheWayInOneStep)
{
	Vector2Df moved = ApproachPoint({0.f, 0.f}, {100.f, 0.f}, 0.12f, 0.016f);

	EXPECT_GT(moved.x, 0.f) << "камера не сдвинулась";
	EXPECT_LT(moved.x, 100.f) << "камера приклеена к цели";
}

TEST(CameraFollowTest, TheCameraArrivesIfYouWaitLongEnough)
{
	Vector2Df at = {0.f, 0.f};
	for (int frame = 0; frame < 200; frame++)
	{
		at = ApproachPoint(at, {100.f, 50.f}, 0.12f, 0.016f);
	}

	EXPECT_NEAR(at.x, 100.f, 0.5f);
	EXPECT_NEAR(at.y, 50.f, 0.5f);
}

TEST(CameraFollowTest, TheSpeedDoesNotDependOnTheFrameRate)
{
	Vector2Df slow = {0.f, 0.f};
	for (int frame = 0; frame < 6; frame++)
	{
		slow = ApproachPoint(slow, {100.f, 0.f}, 0.12f, 1.f / 30.f);
	}

	Vector2Df fast = {0.f, 0.f};
	for (int frame = 0; frame < 24; frame++)
	{
		fast = ApproachPoint(fast, {100.f, 0.f}, 1.f * 0.12f, 1.f / 120.f);
	}

	EXPECT_NEAR(slow.x, fast.x, 0.5f) << "камера едет по-разному на разной частоте кадров";
}

// Ноль означает прежнее поведение - жёсткую привязку.
TEST(CameraFollowTest, WithoutSmoothingTheCameraSticks)
{
	Vector2Df at = ApproachPoint({0.f, 0.f}, {100.f, 0.f}, 0.f, 0.016f);

	EXPECT_FLOAT_EQ(at.x, 100.f);
}

TEST(CameraClampTest, TheEdgeOfTheMapStopsTheCamera)
{
	// Карта шире кадра: 1000 при половине кадра 100.
	EXPECT_FLOAT_EQ(ClampAxis(-500.f, 100.f, 0.f, 1000.f), 100.f) << "камера уехала за левый край";
	EXPECT_FLOAT_EQ(ClampAxis(5000.f, 100.f, 0.f, 1000.f), 900.f) << "камера уехала за правый край";
	EXPECT_FLOAT_EQ(ClampAxis(500.f, 100.f, 0.f, 1000.f), 500.f) << "камеру зажало в середине карты";
}

TEST(CameraClampTest, ASmallMapIsCentredInsteadOfClamped)
{
	EXPECT_FLOAT_EQ(ClampAxis(-500.f, 300.f, 0.f, 200.f), 100.f);
	EXPECT_FLOAT_EQ(ClampAxis(5000.f, 300.f, 0.f, 200.f), 100.f);
}

TEST(CameraClampTest, BothAxesAreHeldAtOnce)
{
	Vector2Df at = ClampToBounds({-900.f, 9000.f}, {100.f, 80.f}, {0.f, 0.f}, {1000.f, 800.f});

	EXPECT_FLOAT_EQ(at.x, 100.f);
	EXPECT_FLOAT_EQ(at.y, 720.f);
}

// Числа настроек - тоже часть правила: ноль вернул бы жёсткую привязку молча.
TEST(CameraFollowTest, TheFollowTimeIsSetToSomethingSoft)
{
	EXPECT_GT(RoguelikeGame::CAMERA_FOLLOW_TIME, 0.f) << "камера снова приклеена";
	EXPECT_LT(RoguelikeGame::CAMERA_FOLLOW_TIME, 0.5f) << "камера отстаёт так, что это уже не слежение";
}
