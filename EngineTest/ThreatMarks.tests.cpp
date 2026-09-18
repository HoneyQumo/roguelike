#include "pch.h"
#include "GameSettings.h"
#include "HudScreen.h"
#include "ThreatMarks.h"
#include "ChaseComponent.h"
#include "ThreatWatchComponent.h"
#include <GameWorld.h>
#include <UiManager.h>

using RoguelikeGame::BuildThreatMarks;
using RoguelikeGame::HudScreen;
using RoguelikeGame::SectorOf;
using RoguelikeGame::ThreatKind;
using RoguelikeGame::ThreatMark;
using RoguelikeGame::ThreatMarkOffset;
using RoguelikeGame::ThreatSource;
using RoguelikeGame::ThreatWatchComponent;
using RoguelikeGame::THREAT_MARK_SECTORS;
using RoguelikeGame::THREAT_NOISE_TIME;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::Vector2Df;

namespace
{
	constexpr int SECTORS = 8;
}

TEST(ThreatSectorTest, NorthIsZeroAndTheRestGoClockwise)
{
	EXPECT_EQ(SectorOf({0.f, 1.f}, SECTORS), 0) << "север не ноль";
	EXPECT_EQ(SectorOf({1.f, 1.f}, SECTORS), 1);
	EXPECT_EQ(SectorOf({1.f, 0.f}, SECTORS), 2) << "восток не четверть";
	EXPECT_EQ(SectorOf({0.f, -1.f}, SECTORS), 4) << "юг не половина";
	EXPECT_EQ(SectorOf({-1.f, 0.f}, SECTORS), 6) << "запад не три четверти";
}

TEST(ThreatSectorTest, DistanceDoesNotChangeTheSector)
{
	EXPECT_EQ(SectorOf({3.f, 0.f}, SECTORS), SectorOf({900.f, 0.f}, SECTORS));
}

TEST(ThreatSectorTest, StandingOnTopOfTheSourceIsHarmless)
{
	EXPECT_EQ(SectorOf({0.f, 0.f}, SECTORS), 0);
	EXPECT_EQ(SectorOf({1.f, 0.f}, 0), 0);
}

// Пять стрелок под одним углом читаются как каша, а нужно «опасность оттуда».
TEST(ThreatMarksTest, ManySourcesInOneSectorGiveOneMark)
{
	std::vector<ThreatSource> sources = {
		{{0.f, 1.f}, ThreatKind::Enemy, 1.f},
		{{0.1f, 1.f}, ThreatKind::Enemy, 1.f},
		{{-0.1f, 1.f}, ThreatKind::Enemy, 1.f}};

	std::vector<ThreatMark> marks = BuildThreatMarks(sources, SECTORS);

	ASSERT_EQ(marks.size(), 1u);
	EXPECT_EQ(marks[0].sector, 0);
}

TEST(ThreatMarksTest, DifferentSectorsKeepTheirOwnMarks)
{
	std::vector<ThreatSource> sources = {
		{{0.f, 1.f}, ThreatKind::Enemy, 1.f},
		{{1.f, 0.f}, ThreatKind::Enemy, 1.f},
		{{-1.f, 0.f}, ThreatKind::Noise, 1.f}};

	std::vector<ThreatMark> marks = BuildThreatMarks(sources, SECTORS);

	EXPECT_EQ(marks.size(), 3u);
}

TEST(ThreatMarksTest, AnEnemyPushesTheNoiseOutOfItsSector)
{
	std::vector<ThreatSource> quiet = {
		{{0.f, 1.f}, ThreatKind::Noise, 1.f},
		{{0.f, 1.f}, ThreatKind::Enemy, 1.f}};

	ASSERT_EQ(BuildThreatMarks(quiet, SECTORS).size(), 1u);
	EXPECT_EQ(BuildThreatMarks(quiet, SECTORS)[0].kind, ThreatKind::Enemy);

	std::vector<ThreatSource> loud = {
		{{0.f, 1.f}, ThreatKind::Enemy, 1.f},
		{{0.f, 1.f}, ThreatKind::Noise, 1.f}};

	EXPECT_EQ(BuildThreatMarks(loud, SECTORS)[0].kind, ThreatKind::Enemy) << "порядок источников решает";
}

// Слабеющий шум не должен затирать свежий в том же секторе.
TEST(ThreatMarksTest, TheLouderOfTwoNoisesWins)
{
	std::vector<ThreatSource> sources = {
		{{0.f, 1.f}, ThreatKind::Noise, 0.2f},
		{{0.f, 1.f}, ThreatKind::Noise, 0.9f}};

	std::vector<ThreatMark> marks = BuildThreatMarks(sources, SECTORS);

	ASSERT_EQ(marks.size(), 1u);
	EXPECT_FLOAT_EQ(marks[0].strength, 0.9f);
}

TEST(ThreatMarksTest, AFadedSourceGivesNoMark)
{
	std::vector<ThreatSource> sources = {{{0.f, 1.f}, ThreatKind::Noise, 0.f}};

	EXPECT_TRUE(BuildThreatMarks(sources, SECTORS).empty());
}

TEST(ThreatMarksTest, MarksSitOnTheEdgeInTheDirectionOfTheirSector)
{
	sf::Vector2f north = ThreatMarkOffset(0, SECTORS, 100.f, 50.f);
	sf::Vector2f east = ThreatMarkOffset(2, SECTORS, 100.f, 50.f);
	sf::Vector2f south = ThreatMarkOffset(4, SECTORS, 100.f, 50.f);

	EXPECT_LT(north.y, 0.f) << "север не сверху";
	EXPECT_GT(east.x, 0.f) << "восток не справа";
	EXPECT_GT(south.y, 0.f) << "юг не снизу";
	EXPECT_NEAR(east.y, 0.f, 0.001f);
}

namespace
{
	class ThreatHudTest : public ::testing::Test
	{
	protected:
		void SetUp() override { GameWorld::Instance()->Clear(); }
		void TearDown() override { GameWorld::Instance()->Clear(); }
	};
}

TEST_F(ThreatHudTest, OnlyTheSectorsThatHaveAMarkAreShown)
{
	HudScreen screen;

	screen.SetThreatMarks({{0, ThreatKind::Enemy, 1.f}, {3, ThreatKind::Noise, 0.5f}});

	EXPECT_EQ(screen.GetThreatMarksShown(), 2);
	EXPECT_TRUE(screen.GetThreatMark(0).IsVisible());
	EXPECT_TRUE(screen.GetThreatMark(3).IsVisible());
	EXPECT_FALSE(screen.GetThreatMark(1).IsVisible());
}

TEST_F(ThreatHudTest, NoThreatMeansNoMarksAtAll)
{
	HudScreen screen;

	screen.SetThreatMarks({{0, ThreatKind::Enemy, 1.f}});
	ASSERT_EQ(screen.GetThreatMarksShown(), 1);

	screen.SetThreatMarks({});

	EXPECT_EQ(screen.GetThreatMarksShown(), 0) << "метка осталась висеть без угрозы";
}

TEST_F(ThreatHudTest, AFadingNoiseIsShownFainter)
{
	HudScreen screen;

	screen.SetThreatMarks({{0, ThreatKind::Noise, 1.f}});
	int loud = screen.GetThreatMark(0).GetShape().getFillColor().a;

	screen.SetThreatMarks({{0, ThreatKind::Noise, 0.25f}});
	int quiet = screen.GetThreatMark(0).GetShape().getFillColor().a;

	EXPECT_LT(quiet, loud) << "тающий шум не тускнеет";
}

TEST_F(ThreatHudTest, AnEnemyAndANoiseLookDifferent)
{
	HudScreen screen;

	// Размер попадает в фигуру только после раскладки.
	sf::FloatRect frame(0.f, 0.f, RoguelikeGame::SCREEN_WIDTH, RoguelikeGame::SCREEN_HEIGHT);

	screen.SetThreatMarks({{0, ThreatKind::Enemy, 1.f}});
	screen.GetRoot().Layout(frame);
	sf::Color enemy = screen.GetThreatMark(0).GetShape().getFillColor();
	float enemySize = screen.GetThreatMark(0).GetShape().getSize().x;

	screen.SetThreatMarks({{0, ThreatKind::Noise, 1.f}});
	screen.GetRoot().Layout(frame);
	sf::Color noise = screen.GetThreatMark(0).GetShape().getFillColor();
	float noiseSize = screen.GetThreatMark(0).GetShape().getSize().x;

	EXPECT_NE(enemy, noise) << "врага не отличить по цвету";
	EXPECT_GT(enemySize, noiseSize) << "врага не отличить по размеру";
}

namespace
{
	class ThreatWatchTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();
			RoguelikeGame::ClearNoiseListeners();

			player = GameWorld::Instance()->CreateGameObject("Player");
			player->GetTransform()->SetWorldPosition({0.f, 0.f});
			watch = player->AddComponent<ThreatWatchComponent>();

			GameWorld::Instance()->Update(0.016f);
		}

		void TearDown() override
		{
			RoguelikeGame::ClearNoiseListeners();
			GameWorld::Instance()->Clear();
		}

		GameObject* player = nullptr;
		ThreatWatchComponent* watch = nullptr;
	};
}

TEST_F(ThreatWatchTest, AHeardNoiseBecomesASourceAndFadesByItself)
{
	watch->Hear({0.f, 300.f});
	watch->Update(0.f);

	ASSERT_EQ(watch->GetSources().size(), 1u);
	EXPECT_EQ(watch->GetSources()[0].kind, ThreatKind::Noise);
	EXPECT_GT(watch->GetSources()[0].strength, 0.9f);

	watch->Update(THREAT_NOISE_TIME * 0.5f);
	EXPECT_LT(watch->GetSources()[0].strength, 0.6f) << "шум не тает";

	watch->Update(THREAT_NOISE_TIME);
	EXPECT_TRUE(watch->GetSources().empty()) << "шум не погас";
}

TEST_F(ThreatWatchTest, ANoiseKeepsTheDirectionItCameFrom)
{
	watch->Hear({500.f, 0.f});
	watch->Update(0.f);

	ASSERT_EQ(watch->GetSources().size(), 1u);
	EXPECT_EQ(SectorOf(watch->GetSources()[0].direction, THREAT_MARK_SECTORS), 2) << "звук с востока пришёл не оттуда";
}

TEST_F(ThreatWatchTest, AFrozenWatchHearsNothing)
{
	watch->SetEnabled(false);

	RoguelikeGame::Noise noise;
	noise.position = {0.f, 100.f};
	noise.radius = 400.f;
	RoguelikeGame::RaiseNoise(noise);

	watch->SetEnabled(true);
	watch->Update(0.f);

	EXPECT_TRUE(watch->GetSources().empty()) << "выключенный компонент всё равно услышал";
}


// Главное правило: метка за то, что тебя засекли, а не за то, что кто-то рядом.
// Иначе туман войны отменяется прибором.
TEST_F(ThreatWatchTest, AnEnemyWhoHasNotSpottedThePlayerGivesNoMark)
{
	GameObject* enemy = GameWorld::Instance()->CreateGameObject("Enemy");
	enemy->GetTransform()->SetWorldPosition({400.f, 0.f});
	auto chase = enemy->AddComponent<RoguelikeGame::ChaseComponent>();
	chase->SetDetectionRadius(400.f);
	chase->SetAlertTime(5.f);

	GameWorld::Instance()->Update(0.016f);
	watch->Update(0.f);

	ASSERT_EQ(chase->GetAwarenessState(), RoguelikeGame::AwarenessState::Calm);
	EXPECT_TRUE(watch->GetSources().empty()) << "спокойный враг выдал себя";

	// Без цели осведомлённость тает уже на следующем кадре, поэтому смотрим сразу.
	chase->Provoke(player->GetTransform()->GetWorldPosition());
	watch->Update(0.f);

	ASSERT_EQ(chase->GetAwarenessState(), RoguelikeGame::AwarenessState::Provoked);
	ASSERT_EQ(watch->GetSources().size(), 1u);
	EXPECT_EQ(watch->GetSources()[0].kind, ThreatKind::Enemy);
	EXPECT_EQ(SectorOf(watch->GetSources()[0].direction, THREAT_MARK_SECTORS), 2) << "враг справа, а метка не там";
}

TEST_F(ThreatWatchTest, AMarkGoesOutWhenTheEnemyLosesThePlayer)
{
	GameObject* enemy = GameWorld::Instance()->CreateGameObject("Enemy");
	enemy->GetTransform()->SetWorldPosition({400.f, 0.f});
	auto chase = enemy->AddComponent<RoguelikeGame::ChaseComponent>();
	chase->SetDetectionRadius(400.f);
	chase->SetAlertTime(5.f);

	GameWorld::Instance()->Update(0.016f);
	chase->Provoke(player->GetTransform()->GetWorldPosition());
	watch->Update(0.f);

	ASSERT_EQ(watch->GetSources().size(), 1u);

	chase->SetEnabled(false);
	watch->Update(0.f);

	EXPECT_TRUE(watch->GetSources().empty()) << "метка осталась от выключенного врага";
}
