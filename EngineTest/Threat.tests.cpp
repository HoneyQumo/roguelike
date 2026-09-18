#include "pch.h"
#include "FogOfWar.h"
#include "GameSettings.h"
#include "ThreatMarkComponent.h"
#include "ThreatPlacement.h"
#include "ChaseComponent.h"
#include "LevelGrid.h"
#include "LevelLoader.h"
#include "ThreatWatchComponent.h"
#include <GameWorld.h>
#include <sstream>

using RoguelikeGame::BuildThreatMarks;
using RoguelikeGame::FogOfWar;
using RoguelikeGame::IsInsideView;
using RoguelikeGame::PinToView;
using RoguelikeGame::ThreatKind;
using RoguelikeGame::ThreatMark;
using RoguelikeGame::ThreatMarkComponent;
using RoguelikeGame::ThreatSource;
using RoguelikeGame::ThreatWatchComponent;
using RoguelikeGame::THREAT_MARK_LIFT;
using RoguelikeGame::THREAT_NOISE_TIME;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::Vector2Df;

namespace
{
	const sf::FloatRect VIEW(0.f, 0.f, 1280.f, 720.f);
	const Vector2Df MIDDLE = {640.f, 360.f};
}

TEST(ThreatMarksTest, AThreatOnTheScreenIsMarkedOverItsHead)
{
	std::vector<ThreatSource> sources = {{{200.f, 100.f}, ThreatKind::Provoked, 1.f}};

	std::vector<ThreatMark> marks = BuildThreatMarks(sources, MIDDLE, VIEW);

	ASSERT_EQ(marks.size(), 1u);
	EXPECT_FLOAT_EQ(marks[0].position.x, 840.f);
	EXPECT_FLOAT_EQ(marks[0].position.y, 460.f + THREAT_MARK_LIFT) << "знак стоит не над головой";
	EXPECT_FLOAT_EQ(marks[0].strength, 1.f);
}

// Иначе про того, кто обычно и стреляет, игрок не узнал бы ничего.
TEST(ThreatMarksTest, AThreatBeyondTheEdgeIsMarkedOnTheEdge)
{
	std::vector<ThreatSource> sources = {{{5000.f, 0.f}, ThreatKind::Provoked, 1.f}};

	std::vector<ThreatMark> marks = BuildThreatMarks(sources, MIDDLE, VIEW);

	ASSERT_EQ(marks.size(), 1u);
	EXPECT_TRUE(IsInsideView(marks[0].position, VIEW)) << "знак остался за кадром";
	EXPECT_GT(marks[0].position.x, VIEW.width * 0.5f) << "знак сел не с той стороны";
	EXPECT_LT(marks[0].strength, 1.f) << "ушедший за край показан в полную силу";
}

TEST(ThreatMarksTest, PinningKeepsTheSideItCameFrom)
{
	EXPECT_LT(PinToView({-9000.f, 360.f}, VIEW, 40.f).x, VIEW.width * 0.5f);
	EXPECT_LT(PinToView({640.f, -9000.f}, VIEW, 40.f).y, VIEW.height * 0.5f);
	EXPECT_GT(PinToView({640.f, 9000.f}, VIEW, 40.f).y, VIEW.height * 0.5f);
}

TEST(ThreatMarksTest, PinningSurvivesAViewSmallerThanTheMargin)
{
	sf::FloatRect tiny(0.f, 0.f, 10.f, 10.f);

	Vector2Df pinned = PinToView({900.f, 900.f}, tiny, 60.f);

	EXPECT_FLOAT_EQ(pinned.x, 5.f);
	EXPECT_FLOAT_EQ(pinned.y, 5.f);
}

TEST(ThreatMarksTest, AFadedSourceIsNotMarked)
{
	std::vector<ThreatSource> sources = {{{10.f, 10.f}, ThreatKind::Noise, 0.f}};

	EXPECT_TRUE(BuildThreatMarks(sources, MIDDLE, VIEW).empty());
}

// Два знака и два смысла: «?» - что-то происходит, «!» - тебя ведут.
TEST(ThreatMarksTest, TheAlertedAskAndTheProvokedShout)
{
	EXPECT_STREQ(ThreatMarkComponent::GlyphOf(ThreatKind::Alerted), "?");
	EXPECT_STREQ(ThreatMarkComponent::GlyphOf(ThreatKind::Noise), "?");
	EXPECT_STREQ(ThreatMarkComponent::GlyphOf(ThreatKind::Provoked), "!");
}

TEST(ThreatMarksTest, TheQuestionIsWarmAndTheShoutIsRed)
{
	sf::Color asking = ThreatMarkComponent::ColorOf(ThreatKind::Alerted, 1.f);
	sf::Color shouting = ThreatMarkComponent::ColorOf(ThreatKind::Provoked, 1.f);

	EXPECT_GT(asking.g, shouting.g) << "«?» не теплее «!»";
	EXPECT_GT(shouting.r, shouting.g * 2) << "«!» не красный";
	EXPECT_EQ(asking.a, 255);
}

TEST(ThreatMarksTest, AFadingSourceIsShownFainter)
{
	EXPECT_LT(ThreatMarkComponent::ColorOf(ThreatKind::Noise, 0.25f).a,
		ThreatMarkComponent::ColorOf(ThreatKind::Noise, 1.f).a);
}

namespace
{
	class ThreatMarkSceneTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();
			object = GameWorld::Instance()->CreateGameObject("Marks");
			marks = object->AddComponent<ThreatMarkComponent>();
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		GameObject* object = nullptr;
		ThreatMarkComponent* marks = nullptr;
	};
}

TEST_F(ThreatMarkSceneTest, NoThreatMeansNoMarks)
{
	marks->SetMarks({{MIDDLE, ThreatKind::Provoked, 1.f}});
	ASSERT_EQ(marks->GetMarks().size(), 1u);

	marks->SetMarks({});

	EXPECT_TRUE(marks->GetMarks().empty()) << "знак остался висеть без угрозы";
}

namespace
{
	const std::string OPEN_ROOM =
		"[map]\n"
		"###########\n"
		"#.........#\n"
		"#.........#\n"
		"#.........#\n"
		"###########\n";

	class ThreatWatchTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();
			RoguelikeGame::ClearNoiseListeners();

			// Туман включён и ничего ещё не открыто: иначе всё считается видимым
			// и ни один противник источником не станет.
			std::istringstream input(OPEN_ROOM);
			grid = RoguelikeGame::LevelGrid::Build(RoguelikeGame::LevelLoader::Parse(input, "glow"));
			RoguelikeGame::LevelGrid::SetCurrent(grid);
			FogOfWar::Reset(grid.GetWidth(), grid.GetHeight(), 6);

			player = GameWorld::Instance()->CreateGameObject("Player");
			player->GetTransform()->SetWorldPosition(grid.ToWorld(2, 2));
			watch = player->AddComponent<ThreatWatchComponent>();

			GameWorld::Instance()->Update(0.016f);
		}

		void TearDown() override
		{
			RoguelikeGame::ClearNoiseListeners();
			GameWorld::Instance()->Clear();
		}

		RoguelikeGame::ChaseComponent* CreateEnemy(float x, float y)
		{
			GameObject* enemy = GameWorld::Instance()->CreateGameObject("Enemy");
			enemy->GetTransform()->SetWorldPosition({x, y});

			auto chase = enemy->AddComponent<RoguelikeGame::ChaseComponent>();
			chase->SetDetectionRadius(400.f);
			chase->SetAlertTime(5.f);

			GameWorld::Instance()->Update(0.016f);

			return chase;
		}

		RoguelikeGame::LevelGrid grid;
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
	Vector2Df stand = player->GetTransform()->GetWorldPosition();

	watch->Hear({stand.x + 500.f, stand.y});
	watch->Update(0.f);

	ASSERT_EQ(watch->GetSources().size(), 1u);
	EXPECT_FLOAT_EQ(watch->GetSources()[0].direction.x, 500.f) << "звук с востока пришёл не оттуда";
	EXPECT_NEAR(watch->GetSources()[0].direction.y, 0.f, 0.001f);
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

// Главное правило: источник за то, что тебя засекли, а не за то, что кто-то рядом.
TEST_F(ThreatWatchTest, AnEnemyWhoHasNotSpottedThePlayerIsNoSource)
{
	RoguelikeGame::ChaseComponent* chase = CreateEnemy(400.f, 0.f);

	watch->Update(0.f);
	ASSERT_EQ(chase->GetAwarenessState(), RoguelikeGame::AwarenessState::Calm);
	EXPECT_TRUE(watch->GetSources().empty()) << "спокойный противник выдал себя";

	// Без цели осведомлённость тает уже на следующем кадре, поэтому смотрим сразу.
	chase->Provoke(player->GetTransform()->GetWorldPosition());
	watch->Update(0.f);

	ASSERT_EQ(chase->GetAwarenessState(), RoguelikeGame::AwarenessState::Provoked);
	ASSERT_EQ(watch->GetSources().size(), 1u);
	EXPECT_EQ(watch->GetSources()[0].kind, ThreatKind::Provoked);
	EXPECT_GT(watch->GetSources()[0].direction.x, 0.f) << "противник справа, а источник не там";
}

TEST_F(ThreatWatchTest, ASourceGoesOutWhenTheEnemyIsGone)
{
	RoguelikeGame::ChaseComponent* chase = CreateEnemy(400.f, 0.f);

	chase->Provoke(player->GetTransform()->GetWorldPosition());
	watch->Update(0.f);
	ASSERT_EQ(watch->GetSources().size(), 1u);

	chase->SetEnabled(false);
	watch->Update(0.f);

	EXPECT_TRUE(watch->GetSources().empty()) << "источник остался от выключенного противника";
}

// Пятно на том, кого игрок и так видит, было бы шумом.
TEST_F(ThreatWatchTest, AnEnemyThePlayerCanSeeIsNoSource)
{
	Vector2Df stand = grid.ToWorld(2, 2);
	Vector2Df seen = grid.ToWorld(4, 2);

	RoguelikeGame::ChaseComponent* chase = CreateEnemy(seen.x, seen.y);
	FogOfWar::Current().Reveal(grid, stand);

	ASSERT_EQ(FogOfWar::Current().GetStateAt(seen), RoguelikeGame::FogState::Seen);

	chase->Provoke(stand);
	watch->Update(0.f);

	EXPECT_TRUE(watch->GetSources().empty()) << "видимый противник всё равно подсвечен";
}
