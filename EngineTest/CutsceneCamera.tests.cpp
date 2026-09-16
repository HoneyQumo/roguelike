#include "pch.h"
#include "CameraDirectorComponent.h"
#include "CutscenePlayerComponent.h"
#include "GameWorld.h"

using RoguelikeGame::CameraDirectorComponent;
using RoguelikeGame::CutsceneBeat;
using RoguelikeGame::CutsceneCommand;
using RoguelikeGame::CutscenePlayerComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::Vector2Df;

namespace
{
	constexpr float STEP = 0.05f;
	constexpr float TRAVEL = 0.4f;

	class CameraTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			hero = GameWorld::Instance()->CreateGameObject("Hero");
			hero->GetTransform()->SetWorldPosition({0.f, 0.f});

			hatch = GameWorld::Instance()->CreateGameObject("Hatch");
			hatch->GetTransform()->SetWorldPosition({600.f, 0.f});

			GameObject* eye = GameWorld::Instance()->CreateGameObject("Camera");
			camera = eye->AddComponent<CameraDirectorComponent>();
			camera->SetFollow(hero);

			GameWorld::Instance()->Update(STEP);
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		void Run(float seconds)
		{
			for (float passed = 0.f; passed < seconds; passed += STEP)
			{
				GameWorld::Instance()->Update(STEP);
			}
		}

		GameObject* hero = nullptr;
		GameObject* hatch = nullptr;
		CameraDirectorComponent* camera = nullptr;
	};
}

TEST_F(CameraTest, TheCameraKeepsUpWithTheHero)
{
	hero->GetTransform()->SetWorldPosition({250.f, 120.f});
	Run(0.2f);

	EXPECT_FLOAT_EQ(camera->GetAim().x, 250.f);
	EXPECT_FLOAT_EQ(camera->GetAim().y, 120.f);
	EXPECT_FALSE(camera->IsAway());
}

TEST_F(CameraTest, ItTakesTimeToDriveOverToTheHatch)
{
	camera->LookAt(hatch, TRAVEL);

	Run(TRAVEL * 0.5f);
	float midway = camera->GetAim().x;

	EXPECT_GT(midway, 0.f) << "the camera has not moved at all";
	EXPECT_LT(midway, 600.f) << "the camera jumped instead of driving";
	EXPECT_TRUE(camera->IsMoving());

	Run(TRAVEL);

	EXPECT_FLOAT_EQ(camera->GetAim().x, 600.f);
	EXPECT_FALSE(camera->IsMoving());
	EXPECT_TRUE(camera->IsAway());
}

TEST_F(CameraTest, WhileAwayItStopsCaringWhereTheHeroRuns)
{
	camera->LookAt(hatch, TRAVEL);
	Run(TRAVEL * 2.f);

	hero->GetTransform()->SetWorldPosition({-900.f, 0.f});
	Run(0.3f);

	EXPECT_FLOAT_EQ(camera->GetAim().x, 600.f) << "the camera was dragged back by the hero";
}

TEST_F(CameraTest, ItComesBackToWhereverTheHeroIsNow)
{
	camera->LookAt(hatch, TRAVEL);
	Run(TRAVEL * 2.f);

	hero->GetTransform()->SetWorldPosition({-300.f, 0.f});
	camera->LookAtFollow(TRAVEL);
	Run(TRAVEL * 2.f);

	EXPECT_FLOAT_EQ(camera->GetAim().x, -300.f);
	EXPECT_FALSE(camera->IsAway());
}

TEST_F(CameraTest, AnAbandonedSceneSnapsTheCameraBack)
{
	camera->LookAt(hatch, TRAVEL);
	Run(TRAVEL * 0.5f);

	camera->Release();

	EXPECT_FLOAT_EQ(camera->GetAim().x, 0.f) << "the camera stayed away after the scene was cut short";
	EXPECT_FALSE(camera->IsAway());
	EXPECT_FALSE(camera->IsMoving());
}

TEST_F(CameraTest, LookingAtNobodyChangesNothing)
{
	camera->LookAt(static_cast<GameObject*>(nullptr), TRAVEL);
	Run(0.3f);

	EXPECT_FALSE(camera->IsAway()) << "the camera went off to look at a target that is not there";
}

namespace
{
	class SceneTest : public CameraTest
	{
	protected:
		void SetUp() override
		{
			CameraTest::SetUp();

			GameObject* holder = GameWorld::Instance()->CreateGameObject("Cutscene");
			scene = holder->AddComponent<CutscenePlayerComponent>();
			scene->SetCamera(camera);

			taken = 0;
			given = 0;
			scene->SetControlLock([this](bool isTaken) { isTaken ? taken++ : given++; });
			scene->SetTargetFinder([](const std::string& name)
			{
				return GameWorld::Instance()->FindGameObject(name);
			});
		}

		CutsceneBeat Beat(CutsceneCommand command, float seconds, const std::string& target = {})
		{
			CutsceneBeat beat;
			beat.command = command;
			beat.seconds = seconds;
			beat.target = target;
			beat.travel = TRAVEL;

			return beat;
		}

		CutscenePlayerComponent* scene = nullptr;
		int taken = 0;
		int given = 0;
	};
}

TEST_F(SceneTest, TheSceneTakesControlAndGivesItBack)
{
	scene->SetBeats({
		Beat(CutsceneCommand::TakeControl, 0.1f),
		Beat(CutsceneCommand::GiveControl, 0.1f),
	});
	scene->Play();

	// Первый шаг длится 0.1 с, поэтому смотреть надо внутри него, а не после.
	Run(STEP);
	EXPECT_EQ(taken, 1);
	EXPECT_EQ(given, 0) << "control came back before the scene was done with it";

	Run(0.3f);

	EXPECT_EQ(given, 1);
	EXPECT_FALSE(scene->IsPlaying());
}

TEST_F(SceneTest, ASceneThatForgetsToGiveControlBackStillGivesItBack)
{
	scene->SetBeats({Beat(CutsceneCommand::TakeControl, 0.1f)});
	scene->Play();

	Run(0.5f);

	EXPECT_FALSE(scene->IsPlaying());
	EXPECT_EQ(given, 1) << "the player would be left unable to move";
}

TEST_F(SceneTest, CuttingTheSceneShortReturnsEverything)
{
	scene->SetBeats({
		Beat(CutsceneCommand::TakeControl, 0.1f),
		Beat(CutsceneCommand::LookAtTarget, 5.f, "Hatch"),
	});
	scene->Play();

	Run(0.3f);
	ASSERT_TRUE(camera->IsAway());

	scene->Stop();

	EXPECT_EQ(given, 1) << "control stayed taken after the scene was cut";
	EXPECT_FALSE(camera->IsAway()) << "the camera stayed away after the scene was cut";
}

TEST_F(SceneTest, ControlIsTakenOnceEvenIfTheSceneAsksTwice)
{
	scene->SetBeats({
		Beat(CutsceneCommand::TakeControl, 0.1f),
		Beat(CutsceneCommand::TakeControl, 0.1f),
		Beat(CutsceneCommand::GiveControl, 0.1f),
	});
	scene->Play();

	Run(0.6f);

	EXPECT_EQ(taken, 1);
	EXPECT_EQ(given, 1);
}

TEST_F(SceneTest, TheSceneDrivesTheCameraToItsTargetAndBack)
{
	scene->SetBeats({
		Beat(CutsceneCommand::LookAtTarget, TRAVEL * 2.f, "Hatch"),
		Beat(CutsceneCommand::LookAtHero, TRAVEL * 2.f),
	});
	scene->Play();

	Run(TRAVEL * 1.5f);
	EXPECT_FLOAT_EQ(camera->GetAim().x, 600.f) << "the camera never reached the hatch";

	Run(TRAVEL * 3.f);

	EXPECT_FLOAT_EQ(camera->GetAim().x, 0.f) << "the camera never came back to the hero";
}

TEST_F(SceneTest, ASceneCanLookAtAPlainSpotWithNothingOnIt)
{
	CutsceneBeat spot = Beat(CutsceneCommand::LookAtPoint, TRAVEL * 2.f);
	spot.point = {-400.f, 200.f};

	scene->SetBeats({spot});
	scene->Play();

	Run(TRAVEL * 1.5f);

	EXPECT_FLOAT_EQ(camera->GetAim().x, -400.f);
	EXPECT_FLOAT_EQ(camera->GetAim().y, 200.f);
}

TEST_F(SceneTest, AMissingTargetLeavesTheCameraWhereItWas)
{
	scene->SetBeats({Beat(CutsceneCommand::LookAtTarget, 0.5f, "NoSuchThing")});
	scene->Play();

	Run(0.4f);

	EXPECT_FLOAT_EQ(camera->GetAim().x, 0.f) << "the camera went off chasing a target that does not exist";
}
