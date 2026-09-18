#include "pch.h"
#include "RunState.h"

using RoguelikeGame::IsRunOver;
using RoguelikeGame::MayPause;
using RoguelikeGame::RunState;

// Пока забег идёт, пауза - обычное дело.
TEST(RunStateTest, AGameInProgressCanBePaused)
{
	EXPECT_TRUE(MayPause(RunState::Playing));
	EXPECT_FALSE(IsRunOver(RunState::Playing));
}

// Смерть ещё не конец забега: доигрывается анимация, и замереть на ней можно.
TEST(RunStateTest, DyingIsNotYetTheEndOfTheRun)
{
	EXPECT_TRUE(MayPause(RunState::PlayerDied));
	EXPECT_FALSE(IsRunOver(RunState::PlayerDied));
}

// Пауза поверх экрана итогов подменяла его собой и прятала клавишу рестарта.
TEST(RunStateTest, AFinishedRunCannotBePaused)
{
	EXPECT_TRUE(IsRunOver(RunState::GameOver));
	EXPECT_TRUE(IsRunOver(RunState::Victory));

	EXPECT_FALSE(MayPause(RunState::GameOver)) << "экран смерти можно поставить на паузу";
	EXPECT_FALSE(MayPause(RunState::Victory)) << "экран победы можно поставить на паузу";
}
