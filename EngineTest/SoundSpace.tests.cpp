#include "pch.h"
#include "AudioComponent.h"
#include "CameraComponent.h"
#include "GameWorld.h"
#include "SoundSpace.h"

using XYZEngine::AudioComponent;
using XYZEngine::CameraComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::HearingFactor;
using XYZEngine::SoundPoint;
using XYZEngine::ToSoundPoint;
using XYZEngine::Vector2Df;

namespace
{
	constexpr float TILE = 64.f;
	constexpr float HALF_SCREEN = 640.f;
}

// Для плоского SFML принято переворачивать Y, но здесь переворот уже сделан
// камерой. Лишний минус увёл бы звук сверху вниз, и это слышно панорамой.
TEST(SoundSpaceTest, TheWorldGoesToTheListenerWithoutFlippingY)
{
	SoundPoint above = ToSoundPoint(Vector2Df{0.f, 100.f});
	SoundPoint below = ToSoundPoint(Vector2Df{0.f, -100.f});

	EXPECT_FLOAT_EQ(above.y, 100.f);
	EXPECT_FLOAT_EQ(below.y, -100.f);
	EXPECT_GT(above.y, below.y);
}

TEST(SoundSpaceTest, TheSideOfTheWorldStaysTheSide)
{
	SoundPoint right = ToSoundPoint(Vector2Df{250.f, 0.f});

	EXPECT_FLOAT_EQ(right.x, 250.f);
	EXPECT_FLOAT_EQ(right.z, 0.f);
}

TEST(SoundSpaceTest, NothingFadesInsideTheMinimalDistance)
{
	EXPECT_FLOAT_EQ(HearingFactor(0.f, 300.f, 1.f), 1.f);
	EXPECT_FLOAT_EQ(HearingFactor(150.f, 300.f, 1.f), 1.f);
	EXPECT_FLOAT_EQ(HearingFactor(300.f, 300.f, 1.f), 1.f);
}

TEST(SoundSpaceTest, FartherIsQuieter)
{
	float close = HearingFactor(TILE, 300.f, 1.f);
	float middle = HearingFactor(HALF_SCREEN, 300.f, 1.f);
	float far = HearingFactor(4.f * HALF_SCREEN, 300.f, 1.f);

	EXPECT_GT(close, middle);
	EXPECT_GT(middle, far);
	EXPECT_GT(far, 0.f);
}

// Затухание единицей и минимальной дистанцией по умолчанию у SFML съедает
// звук за одну клетку: 1 / (1 + 63) - это полтора процента.
TEST(SoundSpaceTest, TheDefaultDistanceIsDeafeningForAPixelWorld)
{
	EXPECT_LT(HearingFactor(TILE, 1.f, 1.f), 0.02f);
	EXPECT_GT(HearingFactor(TILE, 300.f, 1.f), 0.9f);
}

TEST(SoundSpaceTest, WithoutAttenuationDistanceDoesNotMatter)
{
	EXPECT_FLOAT_EQ(HearingFactor(10000.f, 300.f, 0.f), 1.f);
	EXPECT_FLOAT_EQ(HearingFactor(10000.f, 0.f, 1.f), 1.f);
}

class SoundSourcePlaceTest : public ::testing::Test
{
protected:
	void SetUp() override { GameWorld::Instance()->Clear(); }
	void TearDown() override { GameWorld::Instance()->Clear(); }
};

// Пока источник относителен слушателю, он звучит ровно так, как звучал до
// появления пространства. На этом держится обещание «микс не поехал».
TEST_F(SoundSourcePlaceTest, ANewSourceSticksToTheListener)
{
	GameObject* owner = GameWorld::Instance()->CreateGameObject("Source");
	AudioComponent* audio = owner->AddComponent<AudioComponent>();

	EXPECT_TRUE(audio->IsRelativeToListener());
	EXPECT_FLOAT_EQ(audio->GetWorldPosition().x, 0.f);
	EXPECT_FLOAT_EQ(audio->GetWorldPosition().y, 0.f);
}

TEST_F(SoundSourcePlaceTest, ASourcePutOnTheMapKeepsItsPlace)
{
	GameObject* owner = GameWorld::Instance()->CreateGameObject("Source");
	AudioComponent* audio = owner->AddComponent<AudioComponent>();

	audio->SetRelativeToListener(false);
	audio->SetWorldPosition({128.f, -256.f});

	EXPECT_FALSE(audio->IsRelativeToListener());
	EXPECT_FLOAT_EQ(audio->GetWorldPosition().x, 128.f);
	EXPECT_FLOAT_EQ(audio->GetWorldPosition().y, -256.f);
}

TEST_F(SoundSourcePlaceTest, TheFadingSettingsSurviveTheRoundTrip)
{
	GameObject* owner = GameWorld::Instance()->CreateGameObject("Source");
	AudioComponent* audio = owner->AddComponent<AudioComponent>();

	audio->SetMinDistance(320.f);
	audio->SetAttenuation(1.5f);

	EXPECT_FLOAT_EQ(audio->GetMinDistance(), 320.f);
	EXPECT_FLOAT_EQ(audio->GetAttenuation(), 1.5f);
}

class ListenerPlaceTest : public ::testing::Test
{
protected:
	void SetUp() override { GameWorld::Instance()->Clear(); }
	void TearDown() override { GameWorld::Instance()->Clear(); }

	CameraComponent* CreateCamera(const Vector2Df& at)
	{
		owner = GameWorld::Instance()->CreateGameObject("Camera");
		owner->GetTransform()->SetWorldPosition(at);

		return owner->AddComponent<CameraComponent>();
	}

	GameObject* owner = nullptr;
};

TEST_F(ListenerPlaceTest, TheListenerGoesWhereTheCameraLooks)
{
	CreateCamera({640.f, -320.f});

	owner->Update(0.016f);

	EXPECT_FLOAT_EQ(XYZEngine::GetListenerPosition().x, 640.f);
	EXPECT_FLOAT_EQ(XYZEngine::GetListenerPosition().y, -320.f);
}

// Кадр рисуется со смещением тряски, и слушатель обязан ехать вместе с ним:
// иначе на взрыве картинка трясётся, а звук стоит.
TEST_F(ListenerPlaceTest, TheListenerShakesTogetherWithThePicture)
{
	CameraComponent* camera = CreateCamera({0.f, 0.f});

	camera->Shake({32.f, 0.5f, 16.f});
	owner->Update(0.016f);

	EXPECT_FALSE(camera->GetShakeOffset().IsZero());
	EXPECT_FLOAT_EQ(XYZEngine::GetListenerPosition().x, camera->GetShakeOffset().x);
	EXPECT_FLOAT_EQ(XYZEngine::GetListenerPosition().y, camera->GetShakeOffset().y);
}
