#include "pch.h"
#include "Shake.h"
#include <CameraComponent.h>
#include <GameWorld.h>

using RoguelikeGame::ShakeEveryCamera;
using XYZEngine::CameraComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;

namespace
{
	class ShakeTest : public ::testing::Test
	{
	protected:
		void SetUp() override { GameWorld::Instance()->Clear(); }
		void TearDown() override { GameWorld::Instance()->Clear(); }

		CameraComponent* CreateCamera(const std::string& name)
		{
			owner = GameWorld::Instance()->CreateGameObject(name);

			return owner->AddComponent<CameraComponent>();
		}

		bool IsShaking(CameraComponent* camera) const
		{
			owner->Update(0.02f);

			return !camera->GetShakeOffset().IsZero();
		}

		GameObject* owner = nullptr;
	};
}

TEST_F(ShakeTest, TheCameraShakesWhereverItLives)
{
	CameraComponent* camera = CreateCamera("Camera");

	ShakeEveryCamera({10.f, 0.2f, 20.f});

	EXPECT_TRUE(IsShaking(camera)) << "the camera was looked for somewhere it does not live";
}

TEST_F(ShakeTest, AWorldWithoutACameraSurvivesTheBlast)
{
	GameWorld::Instance()->CreateGameObject("Player");

	ShakeEveryCamera({10.f, 0.2f, 20.f});
}

TEST_F(ShakeTest, AnEmptyPlayerDoesNotHideTheRealCamera)
{
	GameWorld::Instance()->CreateGameObject("Player");
	CameraComponent* camera = CreateCamera("Camera");

	ShakeEveryCamera({10.f, 0.2f, 20.f});

	EXPECT_TRUE(IsShaking(camera));
}
