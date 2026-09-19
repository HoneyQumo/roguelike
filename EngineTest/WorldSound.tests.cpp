#include "pch.h"
#include "GameWorld.h"
#include "WorldSound.h"

using RoguelikeGame::PlaceInWorld;
using RoguelikeGame::SOUND_ATTENUATION;
using RoguelikeGame::SOUND_FULL_DISTANCE;
using XYZEngine::AudioComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;

class WorldSoundTest : public ::testing::Test
{
protected:
	void SetUp() override { GameWorld::Instance()->Clear(); }
	void TearDown() override { GameWorld::Instance()->Clear(); }

	AudioComponent* CreateSource(const XYZEngine::Vector2Df& at)
	{
		owner = GameWorld::Instance()->CreateGameObject("Source");
		owner->GetTransform()->SetWorldPosition(at);

		return owner->AddComponent<AudioComponent>();
	}

	GameObject* owner = nullptr;
};

TEST_F(WorldSoundTest, APlacedSourceStopsStickingToTheListener)
{
	AudioComponent* audio = CreateSource({0.f, 0.f});

	PlaceInWorld(audio);

	EXPECT_FALSE(audio->IsRelativeToListener());
	EXPECT_FLOAT_EQ(audio->GetMinDistance(), SOUND_FULL_DISTANCE);
	EXPECT_FLOAT_EQ(audio->GetAttenuation(), SOUND_ATTENUATION);
}

// Машина побега приезжает издалека и меняет место каждый кадр. Разовой
// установки в момент запуска ей мало - звук остался бы там, где она была.
TEST_F(WorldSoundTest, APlacedSourceRidesWithItsObject)
{
	AudioComponent* audio = CreateSource({100.f, 100.f});
	PlaceInWorld(audio);

	owner->Update(0.016f);
	EXPECT_FLOAT_EQ(audio->GetWorldPosition().x, 100.f);

	owner->GetTransform()->SetWorldPosition({900.f, -300.f});
	owner->Update(0.016f);

	EXPECT_FLOAT_EQ(audio->GetWorldPosition().x, 900.f);
	EXPECT_FLOAT_EQ(audio->GetWorldPosition().y, -300.f);
}

// Звуки игрока остаются на слушателе: слушатель стоит на камере, а она отстаёт
// от героя и прижимается к краю карты - свои выстрелы гуляли бы по панораме.
TEST_F(WorldSoundTest, ASourceLeftOnTheListenerDoesNotRideAnywhere)
{
	AudioComponent* audio = CreateSource({100.f, 100.f});

	owner->GetTransform()->SetWorldPosition({900.f, -300.f});
	owner->Update(0.016f);

	EXPECT_TRUE(audio->IsRelativeToListener());
	EXPECT_FLOAT_EQ(audio->GetWorldPosition().x, 0.f);
	EXPECT_FLOAT_EQ(audio->GetWorldPosition().y, 0.f);
}

TEST_F(WorldSoundTest, PlacingNothingIsNotACrash)
{
	PlaceInWorld(nullptr);
}

// Полный радиус равен половине кадра по ширине: то, что видно, звучит в полную
// силу, а дальше падает. Иначе игра оглохнет уже через клетку.
TEST_F(WorldSoundTest, TheFullDistanceCoversWhatIsOnTheScreen)
{
	EXPECT_FLOAT_EQ(SOUND_FULL_DISTANCE, 640.f);

	EXPECT_FLOAT_EQ(XYZEngine::HearingFactor(RoguelikeGame::TILE_SIZE, SOUND_FULL_DISTANCE, SOUND_ATTENUATION), 1.f);
	EXPECT_FLOAT_EQ(XYZEngine::HearingFactor(SOUND_FULL_DISTANCE, SOUND_FULL_DISTANCE, SOUND_ATTENUATION), 1.f);
	EXPECT_NEAR(XYZEngine::HearingFactor(2.f * SOUND_FULL_DISTANCE, SOUND_FULL_DISTANCE, SOUND_ATTENUATION), 0.5f, 0.01f);
}
