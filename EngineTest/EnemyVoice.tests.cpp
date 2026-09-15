#include "pch.h"
#include "EnemyCatalog.h"
#include "EnemyVoice.h"

using RoguelikeGame::AwarenessState;
using RoguelikeGame::ENEMIES;
using RoguelikeGame::ShouldSpeakOnSpotting;
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
	EXPECT_TRUE(ShouldSpeakOnSpotting(AwarenessState::Calm, AwarenessState::Provoked));
	EXPECT_TRUE(ShouldSpeakOnSpotting(AwarenessState::Alerted, AwarenessState::Provoked));
}

TEST(EnemyVoiceTest, EnemyKeepsQuietWhileItOnlySuspects)
{
	EXPECT_FALSE(ShouldSpeakOnSpotting(AwarenessState::Calm, AwarenessState::Alerted));
	EXPECT_FALSE(ShouldSpeakOnSpotting(AwarenessState::Calm, AwarenessState::Calm));
}

TEST(EnemyVoiceTest, EnemyDoesNotRepeatItselfWhileChasing)
{
	EXPECT_FALSE(ShouldSpeakOnSpotting(AwarenessState::Provoked, AwarenessState::Provoked));
}

TEST(EnemyVoiceTest, LosingAndSpottingAgainSpeaksAgain)
{
	EXPECT_FALSE(ShouldSpeakOnSpotting(AwarenessState::Provoked, AwarenessState::Alerted));
	EXPECT_TRUE(ShouldSpeakOnSpotting(AwarenessState::Alerted, AwarenessState::Provoked));
}

TEST(EnemyVoiceTest, EveryEnemyInTheCatalogHasAVoice)
{
	for (const RoguelikeGame::EnemyDefinition& enemy : ENEMIES)
	{
		EXPECT_NE(enemy.config.voice, nullptr) << enemy.config.objectName;
		EXPECT_GT(enemy.config.voiceLines, 0) << enemy.config.objectName;
	}
}
