#include "pch.h"
#include "GameWorld.h"
#include "CameraComponent.h"

using XYZEngine::CameraComponent;
using XYZEngine::CameraShake;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;

namespace
{
	class CameraShakeTest : public ::testing::Test
	{
	protected:
		void SetUp() override { GameWorld::Instance()->Clear(); }
		void TearDown() override { GameWorld::Instance()->Clear(); }

		GameObject* owner = nullptr;

		CameraComponent* CreateCamera()
		{
			owner = GameWorld::Instance()->CreateGameObject("Camera");
			owner->GetTransform()->SetWorldPosition({100.f, 200.f});

			return owner->AddComponent<CameraComponent>();
		}

		float Length(const XYZEngine::Vector2Df& value) const
		{
			return std::sqrt(value.x * value.x + value.y * value.y);
		}
	};
}

TEST_F(CameraShakeTest, CalmCameraHasNoOffset)
{
	CameraComponent* camera = CreateCamera();

	owner->Update(0.016f);

	EXPECT_TRUE(camera->GetShakeOffset().IsZero());
}

TEST_F(CameraShakeTest, ShakeMovesTheViewAndFadesToExactZero)
{
	CameraComponent* camera = CreateCamera();
	camera->Shake({10.f, 0.2f, 20.f});

	owner->Update(0.02f);
	EXPECT_GT(Length(camera->GetShakeOffset()), 0.f);

	for (int step = 0; step < 20; step++)
	{
		owner->Update(0.02f);
	}

	EXPECT_TRUE(camera->GetShakeOffset().IsZero());
}

TEST_F(CameraShakeTest, ShakeDoesNotMoveTheObject)
{
	CameraComponent* camera = CreateCamera();
	camera->Shake({20.f, 0.3f, 20.f});

	for (int step = 0; step < 5; step++)
	{
		owner->Update(0.02f);
	}

	EXPECT_FLOAT_EQ(owner->GetTransform()->GetWorldPosition().x, 100.f);
	EXPECT_FLOAT_EQ(owner->GetTransform()->GetWorldPosition().y, 200.f);
}

TEST_F(CameraShakeTest, AmplitudeIsCappedFromAbove)
{
	CameraComponent* camera = CreateCamera();
	camera->SetMaxShakeAmplitude(15.f);
	camera->Shake({1000.f, 0.5f, 20.f});

	float peak = 0.f;
	for (int step = 0; step < 20; step++)
	{
		owner->Update(0.01f);
		peak = std::max(peak, Length(camera->GetShakeOffset()));
	}

	EXPECT_LE(peak, 15.f * std::sqrt(2.f) + 0.001f);
}

TEST_F(CameraShakeTest, RepeatedLightShakesDoNotPileUp)
{
	CameraComponent* camera = CreateCamera();
	CameraShake light = {6.f, 0.15f, 20.f};

	float peak = 0.f;
	for (int hit = 0; hit < 5; hit++)
	{
		camera->Shake(light);
		for (int step = 0; step < 3; step++)
		{
			owner->Update(0.01f);
			peak = std::max(peak, Length(camera->GetShakeOffset()));
		}
	}

	EXPECT_LE(peak, light.amplitude * std::sqrt(2.f) + 0.001f);
}

TEST_F(CameraShakeTest, StrongerShakeOverridesTheWeakerOne)
{
	CameraComponent* camera = CreateCamera();
	camera->Shake({4.f, 0.3f, 20.f});
	owner->Update(0.01f);
	float weakPeak = Length(camera->GetShakeOffset());

	camera->Shake({30.f, 0.3f, 20.f});
	float strongPeak = 0.f;
	for (int step = 0; step < 8; step++)
	{
		owner->Update(0.01f);
		strongPeak = std::max(strongPeak, Length(camera->GetShakeOffset()));
	}

	EXPECT_GT(strongPeak, weakPeak);
}

TEST_F(CameraShakeTest, StopShakeClearsOffsetImmediately)
{
	CameraComponent* camera = CreateCamera();
	camera->Shake({20.f, 0.5f, 20.f});
	owner->Update(0.02f);

	camera->StopShake();

	EXPECT_TRUE(camera->GetShakeOffset().IsZero());
}
