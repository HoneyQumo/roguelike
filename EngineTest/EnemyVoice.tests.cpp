#include "pch.h"
#include "SpeechCatalogLoader.h"
#include "SpeechDirector.h"
#include <sstream>
#include "EnemyCatalog.h"
#include "EnemyVoice.h"

using RoguelikeGame::AwarenessState;
using RoguelikeGame::ENEMIES;
using RoguelikeGame::IsSpottedNow;
using RoguelikeGame::VoiceFilePath;
using RoguelikeGame::VoiceKey;

TEST(EnemyVoiceTest, KeyAndFileFollowTheVoiceName)
{
	EXPECT_EQ(VoiceKey("guard", 2), "voice_guard_2");
	EXPECT_EQ(VoiceFilePath("guard", 2), "Resources/Audio/Voice/voice_guard_2.wav");
}

TEST(EnemyVoiceTest, EnemyWithoutAVoiceHasNoLines)
{
	EXPECT_TRUE(VoiceKey(nullptr, 1).empty());
	EXPECT_TRUE(VoiceFilePath(nullptr, 1).empty());
}

TEST(EnemyVoiceTest, EveryLineHasItsOwnFile)
{
	EXPECT_NE(VoiceFilePath("radio", 1), VoiceFilePath("radio", 2));
	EXPECT_NE(VoiceFilePath("radio", 1), VoiceFilePath("guard", 1));
}

TEST(EnemyVoiceTest, EnemySpeaksTheMomentItSpotsTheTarget)
{
	EXPECT_TRUE(IsSpottedNow(AwarenessState::Calm, AwarenessState::Provoked));
	EXPECT_TRUE(IsSpottedNow(AwarenessState::Alerted, AwarenessState::Provoked));
}

TEST(EnemyVoiceTest, EnemyKeepsQuietWhileItOnlySuspects)
{
	EXPECT_FALSE(IsSpottedNow(AwarenessState::Calm, AwarenessState::Alerted));
	EXPECT_FALSE(IsSpottedNow(AwarenessState::Calm, AwarenessState::Calm));
}

TEST(EnemyVoiceTest, EnemyDoesNotRepeatItselfWhileChasing)
{
	EXPECT_FALSE(IsSpottedNow(AwarenessState::Provoked, AwarenessState::Provoked));
}

TEST(EnemyVoiceTest, LosingAndSpottingAgainSpeaksAgain)
{
	EXPECT_FALSE(IsSpottedNow(AwarenessState::Provoked, AwarenessState::Alerted));
	EXPECT_TRUE(IsSpottedNow(AwarenessState::Alerted, AwarenessState::Provoked));
}

// Набор врага обязан существовать в каталоге реплик: опечатка в имени даёт
// немого врага, и заметить это можно только на слух.
TEST(EnemyVoiceTest, EverySetNamedByAnEnemyIsInTheCatalog)
{
	std::istringstream file(
		"[line a]\nspeaker Кто\ntext Раз\n"
		"[set guard_spotted]\nline a\n"
		"[set heavy_spotted]\nline a\n"
		"[set radio_spotted]\nline a\n");
	RoguelikeGame::SpeechCatalog catalog = RoguelikeGame::SpeechCatalogLoader::Parse(file, "test");

	for (const RoguelikeGame::EnemyDefinition& enemy : ENEMIES)
	{
		ASSERT_NE(enemy.config.spottedSpeech, nullptr) << enemy.config.objectName;
		EXPECT_NE(catalog.FindSet(enemy.config.spottedSpeech), nullptr)
			<< enemy.config.objectName << " говорит набором " << enemy.config.spottedSpeech;
	}
}

// Враг говорит через общее ядро речи, а не своим путём: иначе субтитр
// появится у одних и не появится у других.
TEST(EnemyVoiceTest, WhatTheEnemySaysReachesTheSubtitles)
{
	std::istringstream file(
		"[line guard_1]\nspeaker Охранник\ntext Вот ты где!\n"
		"[set guard_spotted]\nline guard_1\n");
	RoguelikeGame::SpeechCatalog catalog = RoguelikeGame::SpeechCatalogLoader::Parse(file, "test");
	RoguelikeGame::SubtitleQueue queue;

	RoguelikeGame::SpeechDirector::Current().SetCatalog(&catalog);
	RoguelikeGame::SpeechDirector::Current().SetQueue(&queue);

	RoguelikeGame::SpeechDirector::Current().SayFromSet("guard_spotted", nullptr,
		RoguelikeGame::SoundPlace::InWorld);

	ASSERT_EQ(queue.GetLines().size(), 1u);
	EXPECT_EQ(queue.GetLines()[0].speaker, "Охранник");

	RoguelikeGame::SpeechDirector::Current().Silence();
	RoguelikeGame::SpeechDirector::Current().SetCatalog(nullptr);
	RoguelikeGame::SpeechDirector::Current().SetQueue(nullptr);
}

TEST(EnemyVoiceTest, EveryEnemyInTheCatalogHasAVoice)
{
	for (const RoguelikeGame::EnemyDefinition& enemy : ENEMIES)
	{
		EXPECT_NE(enemy.config.spottedSpeech, nullptr) << enemy.config.objectName;
	}
}
