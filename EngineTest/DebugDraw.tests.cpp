#include "pch.h"
#include "DebugDraw.h"

using XYZEngine::DebugDraw;

TEST(DebugDrawTests, ShapesAreQueuedOnlyWhenEnabled)
{
	auto debugDraw = DebugDraw::Instance();
	debugDraw->SetEnabled(false);

	debugDraw->DrawCircle({0.f, 0.f}, 10.f, sf::Color::Red);
	EXPECT_EQ(debugDraw->GetQueuedShapesCount(), 0u);

	debugDraw->SetEnabled(true);
	debugDraw->DrawCircle({0.f, 0.f}, 10.f, sf::Color::Red);
	debugDraw->DrawRect({0.f, 0.f, 1.f, 1.f}, sf::Color::Red);
	debugDraw->DrawLine({0.f, 0.f}, {1.f, 1.f}, sf::Color::Red);
	EXPECT_EQ(debugDraw->GetQueuedShapesCount(), 3u);

	debugDraw->SetEnabled(false);
	EXPECT_EQ(debugDraw->GetQueuedShapesCount(), 0u);
}

TEST(DebugDrawTests, ToggleFlipsEnabled)
{
	auto debugDraw = DebugDraw::Instance();
	debugDraw->SetEnabled(false);

	debugDraw->Toggle();
	EXPECT_TRUE(debugDraw->IsEnabled());

	debugDraw->Toggle();
	EXPECT_FALSE(debugDraw->IsEnabled());
}
