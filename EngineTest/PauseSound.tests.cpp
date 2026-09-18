#include "pch.h"
#include "Engine.h"
#include "GameWorld.h"
#include "AudioComponent.h"
#include "SoundSource.h"

using XYZEngine::Engine;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::SoundSource;

namespace
{
	/**
	*	Подставной источник вместо настоящего звука.
	*
	*	Настоящему нужна звуковая карта и загруженный буфер, а проверить надо не
	*	SFML, а то, что движок доходит до каждого источника в мире.
	*/
	class CountingSound : public SoundSource
	{
	public:
		CountingSound(GameObject* gameObject) : SoundSource(gameObject) {}

		void Update(float deltaTime) override {}
		void Render() override {}

		void Pause() override { paused++; }
		void Resume() override { resumed++; }
		void Stop() override { stopped++; }
		bool IsPlaying() const override { return true; }

		int paused = 0;
		int resumed = 0;
		int stopped = 0;
	};

	class PauseSoundTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();
			Engine::Instance()->SetPaused(false);
		}

		void TearDown() override
		{
			Engine::Instance()->SetPaused(false);
			GameWorld::Instance()->Clear();
		}

		CountingSound* CreateSound(const std::string& name)
		{
			return GameWorld::Instance()->CreateGameObject(name)->AddComponent<CountingSound>();
		}
	};
}

// Зацикленный мотор машины продолжал реветь на паузе: сцена глушила только музыку.
TEST_F(PauseSoundTest, PausingTheGameSilencesEverySource)
{
	CountingSound* music = CreateSound("Music");
	CountingSound* engineLoop = CreateSound("Car");

	Engine::Instance()->SetPaused(true);

	EXPECT_EQ(music->paused, 1);
	EXPECT_EQ(engineLoop->paused, 1) << "источник остался звучать на паузе";
}

TEST_F(PauseSoundTest, ResumingBringsTheSoundBack)
{
	CountingSound* sound = CreateSound("Car");

	Engine::Instance()->SetPaused(true);
	Engine::Instance()->SetPaused(false);

	EXPECT_EQ(sound->paused, 1);
	EXPECT_EQ(sound->resumed, 1);
}

// У игрока три источника на одном объекте - обход по одному с объекта пропускал два.
TEST_F(PauseSoundTest, EverySourceOnTheSameObjectIsSilenced)
{
	GameObject* player = GameWorld::Instance()->CreateGameObject("Player");

	CountingSound* shot = player->AddComponent<CountingSound>();
	CountingSound* reload = player->AddComponent<CountingSound>();
	CountingSound* melee = player->AddComponent<CountingSound>();

	Engine::Instance()->SetPaused(true);

	EXPECT_EQ(shot->paused, 1);
	EXPECT_EQ(reload->paused, 1) << "второй источник на объекте пропущен";
	EXPECT_EQ(melee->paused, 1) << "третий источник на объекте пропущен";
}

// Потеря фокуса ставит паузу каждый кадр, пока окно не вернут.
TEST_F(PauseSoundTest, PausingTwiceDoesNotPauseTwice)
{
	CountingSound* sound = CreateSound("Car");

	Engine::Instance()->SetPaused(true);
	Engine::Instance()->SetPaused(true);

	EXPECT_EQ(sound->paused, 1) << "повторная пауза дошла до источника";
}

// Снятие паузы проходит по всем источникам подряд, в том числе по давно
// доигравшим. Настоящий источник обязан на это не реагировать - иначе после
// паузы разом заговорит каждый звук, когда-либо созданный на уровне.
TEST_F(PauseSoundTest, ResumeDoesNotStartWhatWasNotPlaying)
{
	GameObject* owner = GameWorld::Instance()->CreateGameObject("Silent");
	XYZEngine::AudioComponent* audio = owner->AddComponent<XYZEngine::AudioComponent>();

	audio->Resume();

	EXPECT_FALSE(audio->IsPlaying());
}
