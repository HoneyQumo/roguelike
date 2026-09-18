#include "pch.h"
#include "FogOfWar.h"
#include "FogVisibilityComponent.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include "LevelLoader.h"
#include "TileFogComponent.h"
#include <GameWorld.h>
#include <VertexArrayRendererComponent.h>
#include <sstream>

using RoguelikeGame::FogOfWar;
using RoguelikeGame::FogState;
using RoguelikeGame::FogVisibilityComponent;
using RoguelikeGame::LevelGrid;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::TileFogComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::VertexArrayRendererComponent;
using XYZEngine::Vector2Df;

namespace
{
	const std::string CORRIDOR =
		"[map]\n"
		"#############\n"
		"#...........#\n"
		"#############\n";

	// Считает, сколько раз его позвали: так видно, дошла ли отрисовка до компонента.
	class CountingComponent : public XYZEngine::Component
	{
	public:
		CountingComponent(GameObject* gameObject) : Component(gameObject) {}

		void Update(float deltaTime) override { updates++; }
		void Render() override { renders++; }

		int updates = 0;
		int renders = 0;
	};

	class FogSceneTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			std::istringstream input(CORRIDOR);
			LevelGrid::SetCurrent(LevelGrid::Build(LevelLoader::Parse(input, "fog")));

			FogOfWar::Reset(LevelGrid::Current().GetWidth(), LevelGrid::Current().GetHeight(), 3);
		}

		void TearDown() override
		{
			GameWorld::Instance()->Clear();
			FogOfWar::Reset(0, 0, 0);
		}

		void Reveal(int column, int row)
		{
			FogOfWar::Current().Reveal(LevelGrid::Current(), LevelGrid::Current().ToWorld(column, row));
		}
	};
}

TEST_F(FogSceneTest, AHiddenObjectDoesNotDrawButKeepsThinking)
{
	GameObject* object = GameWorld::Instance()->CreateGameObject("Hidden");
	auto counter = object->AddComponent<CountingComponent>();

	object->SetVisible(false);
	object->Render();
	object->Update(0.f);

	EXPECT_EQ(counter->renders, 0) << "невидимый объект всё равно рисуется";
	EXPECT_EQ(counter->updates, 1) << "спрятанный объект перестал жить";
}

TEST_F(FogSceneTest, HidingAnObjectHidesWhatHangsOnIt)
{
	GameObject* object = GameWorld::Instance()->CreateGameObject("Parent");
	GameObject* child = GameWorld::Instance()->CreateGameObject("Child", object);
	auto counter = child->AddComponent<CountingComponent>();

	object->SetVisible(false);
	child->Render();

	EXPECT_FALSE(child->IsVisible());
	EXPECT_EQ(counter->renders, 0) << "обвеска осталась висеть в воздухе";
}

TEST_F(FogSceneTest, AnObjectOutOfSightIsHidden)
{
	GameObject* object = GameWorld::Instance()->CreateGameObject("Crate");
	object->GetTransform()->SetWorldPosition(LevelGrid::Current().ToWorld(10, 1));
	auto fog = object->AddComponent<FogVisibilityComponent>();

	Reveal(1, 1);
	fog->Update(0.f);

	EXPECT_TRUE(fog->IsHidden());
	EXPECT_FALSE(object->IsVisible());
}

TEST_F(FogSceneTest, AnObjectComesBackWhenItIsSeenAgain)
{
	GameObject* object = GameWorld::Instance()->CreateGameObject("Crate");
	object->GetTransform()->SetWorldPosition(LevelGrid::Current().ToWorld(10, 1));
	auto fog = object->AddComponent<FogVisibilityComponent>();

	Reveal(1, 1);
	fog->Update(0.f);
	ASSERT_TRUE(fog->IsHidden());

	Reveal(10, 1);
	fog->Update(0.f);

	EXPECT_FALSE(fog->IsHidden());
	EXPECT_TRUE(object->IsVisible());
}

TEST_F(FogSceneTest, RememberedGeometryIsNotEnoughToShowAnObject)
{
	GameObject* object = GameWorld::Instance()->CreateGameObject("Crate");
	object->GetTransform()->SetWorldPosition(LevelGrid::Current().ToWorld(1, 1));
	auto fog = object->AddComponent<FogVisibilityComponent>();

	Reveal(1, 1);
	fog->Update(0.f);
	ASSERT_FALSE(fog->IsHidden());

	Reveal(10, 1);
	fog->Update(0.f);

	ASSERT_EQ(FogOfWar::Current().GetState(1, 1), FogState::Known);
	EXPECT_TRUE(fog->IsHidden()) << "ящик рисуется по памяти, хотя его там может уже не быть";
}

TEST_F(FogSceneTest, TheCanvasIsPaintedByWhatIsSeenAndRemembered)
{
	GameObject* chunk = GameWorld::Instance()->CreateGameObject("LevelTiles");
	auto renderer = chunk->AddComponent<VertexArrayRendererComponent>();
	auto fog = chunk->AddComponent<TileFogComponent>();
	fog->SetRenderer(renderer);

	const Vector2Df tileSize = {RoguelikeGame::TILE_SIZE, RoguelikeGame::TILE_SIZE};
	for (int column : {1, 5, 10})
	{
		fog->AddCell(renderer->GetQuadsCount(), column, 1, sf::Color::White);
		renderer->AddQuad(LevelGrid::Current().ToWorld(column, 1), tileSize, sf::Color::White);
	}

	Reveal(10, 1);
	Reveal(5, 1);
	fog->Update(0.f);

	const sf::VertexArray& vertices = renderer->GetVertices();

	EXPECT_EQ(vertices[0].color.a, 0) << "неразведанная клетка видна";
	EXPECT_EQ(vertices[4].color, sf::Color::White) << "клетка под ногами приглушена";
	EXPECT_EQ(vertices[8].color.a, 255);
	EXPECT_LT(vertices[8].color.r, 255) << "разведанное светит как видимое";
	EXPECT_GT(vertices[8].color.r, 0) << "разведанное потухло совсем";
}

TEST_F(FogSceneTest, AChunkWithNothingKnownIsNotDrawnAtAll)
{
	GameObject* chunk = GameWorld::Instance()->CreateGameObject("LevelTiles");
	auto renderer = chunk->AddComponent<VertexArrayRendererComponent>();
	auto fog = chunk->AddComponent<TileFogComponent>();
	fog->SetRenderer(renderer);

	fog->AddCell(0u, 10, 1, sf::Color::White);
	renderer->AddQuad(LevelGrid::Current().ToWorld(10, 1),
		{RoguelikeGame::TILE_SIZE, RoguelikeGame::TILE_SIZE}, sf::Color::White);

	Reveal(1, 1);
	fog->Update(0.f);

	EXPECT_FALSE(renderer->IsEnabled()) << "кусок карты без единой разведанной клетки уходит в отрисовку";

	Reveal(10, 1);
	fog->Update(0.f);

	EXPECT_TRUE(renderer->IsEnabled());
}

TEST_F(FogSceneTest, TheCanvasIsRepaintedOnlyWhenTheFogMoves)
{
	GameObject* chunk = GameWorld::Instance()->CreateGameObject("LevelTiles");
	auto renderer = chunk->AddComponent<VertexArrayRendererComponent>();
	auto fog = chunk->AddComponent<TileFogComponent>();
	fog->SetRenderer(renderer);

	fog->AddCell(0u, 1, 1, sf::Color::White);
	renderer->AddQuad(LevelGrid::Current().ToWorld(1, 1),
		{RoguelikeGame::TILE_SIZE, RoguelikeGame::TILE_SIZE}, sf::Color::White);

	Reveal(1, 1);
	fog->Update(0.f);
	fog->Update(0.f);
	fog->Update(0.f);

	EXPECT_EQ(fog->GetPaintCount(), 1) << "полотно перекрашивается каждый кадр";

	Reveal(10, 1);
	fog->Update(0.f);

	EXPECT_EQ(fog->GetPaintCount(), 2);
}


// Квад на краю обзора растягивает градиент сам: углы красятся порознь.
TEST_F(FogSceneTest, TheQuadOnTheEdgeOfSightHasCornersOfItsOwn)
{
	GameObject* chunk = GameWorld::Instance()->CreateGameObject("LevelTiles");
	auto renderer = chunk->AddComponent<VertexArrayRendererComponent>();
	auto fog = chunk->AddComponent<TileFogComponent>();
	fog->SetRenderer(renderer);

	const Vector2Df tileSize = {RoguelikeGame::TILE_SIZE, RoguelikeGame::TILE_SIZE};
	for (int column = 1; column <= 11; column++)
	{
		fog->AddCell(renderer->GetQuadsCount(), column, 1, sf::Color::White);
		renderer->AddQuad(LevelGrid::Current().ToWorld(column, 1), tileSize, sf::Color::White);
	}

	Reveal(1, 1);
	fog->Update(0.f);

	const sf::VertexArray& vertices = renderer->GetVertices();

	bool hasGradient = false;
	for (std::size_t quad = 0u; quad < renderer->GetQuadsCount(); quad++)
	{
		std::size_t first = quad * 4u;
		if (vertices[first].color != vertices[first + 1u].color)
		{
			hasGradient = true;
			break;
		}
	}

	EXPECT_TRUE(hasGradient) << "все углы одного цвета - градиента внутри тайла нет";
}

TEST_F(FogSceneTest, TheRendererPaintsEveryCornerOnItsOwn)
{
	GameObject* chunk = GameWorld::Instance()->CreateGameObject("Quad");
	auto renderer = chunk->AddComponent<VertexArrayRendererComponent>();

	renderer->AddQuad({0.f, 0.f}, {10.f, 10.f}, sf::Color::White);
	renderer->SetQuadCorners(0u, sf::Color::Red, sf::Color::Green, sf::Color::Blue, sf::Color::Yellow);

	const sf::VertexArray& vertices = renderer->GetVertices();

	EXPECT_EQ(vertices[0].color, sf::Color::Red);
	EXPECT_EQ(vertices[1].color, sf::Color::Green);
	EXPECT_EQ(vertices[2].color, sf::Color::Blue);
	EXPECT_EQ(vertices[3].color, sf::Color::Yellow);
}
