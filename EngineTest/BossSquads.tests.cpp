#include "pch.h"
#include "BossCatalog.h"
#include "EnemyCatalog.h"

using RoguelikeGame::BossAbility;
using RoguelikeGame::BossAbilitySpec;
using RoguelikeGame::BossMinionAt;
using RoguelikeGame::BossSquadAt;
using RoguelikeGame::BOSS_HARD_SQUAD_FROM;
using RoguelikeGame::BOSS_MINION_LIMIT;
using RoguelikeGame::BOSS_SQUADS;
using RoguelikeGame::BOSS_SQUAD_COUNT;
using RoguelikeGame::BOSS_SQUAD_SIZE;
using RoguelikeGame::EnemyConfig;
using RoguelikeGame::FindBossAbility;
using RoguelikeGame::FindEnemyConfig;
using RoguelikeGame::MinionSquad;
using RoguelikeGame::WeaponId;

namespace
{
	float SquadToughness(const MinionSquad& squad)
	{
		float total = 0.f;
		for (int index = 0; index < BOSS_SQUAD_SIZE; index++)
		{
			const EnemyConfig* config = FindEnemyConfig(BossMinionAt(squad, index));
			total += config == nullptr ? 0.f : config->maxHealth + config->armor;
		}

		return total;
	}

	bool HasShooter(const MinionSquad& squad)
	{
		for (int index = 0; index < BOSS_SQUAD_SIZE; index++)
		{
			const EnemyConfig* config = FindEnemyConfig(BossMinionAt(squad, index));
			if (config != nullptr && config->weapon != WeaponId::Knife)
			{
				return true;
			}
		}

		return false;
	}
}

TEST(BossSquadTests, EverySummonedEnemyExists)
{
	for (const MinionSquad& squad : BOSS_SQUADS)
	{
		for (int index = 0; index < BOSS_SQUAD_SIZE; index++)
		{
			EXPECT_NE(FindEnemyConfig(BossMinionAt(squad, index)), nullptr) << squad.name << " slot " << index;
		}
	}
}

TEST(BossSquadTests, TheSquadFillsTheWholeSummon)
{
	const BossAbilitySpec* summon = FindBossAbility(BossAbility::Summon);

	ASSERT_NE(summon, nullptr);
	EXPECT_EQ(summon->count, BOSS_SQUAD_SIZE) << "the boss calls more minions than a squad holds";
}

TEST(BossSquadTests, TwoCallsInARowBringDifferentCompany)
{
	EXPECT_STRNE(BossSquadAt(0, false).name, BossSquadAt(1, false).name) << "the boss keeps calling the same squad";
}

TEST(BossSquadTests, TheRotationComesBackAround)
{
	EXPECT_STREQ(BossSquadAt(0, false).name, BossSquadAt(BOSS_SQUAD_COUNT, false).name);
	EXPECT_STREQ(BossSquadAt(1, false).name, BossSquadAt(BOSS_SQUAD_COUNT + 1, false).name);
}

TEST(BossSquadTests, ACallBeforeTheFirstOneIsStillASquad)
{
	EXPECT_STREQ(BossSquadAt(-3, false).name, BOSS_SQUADS[0].name);
}

TEST(BossSquadTests, RageLeavesOnlyTheHeavySquads)
{
	for (int call = 0; call < 2 * BOSS_SQUAD_COUNT; call++)
	{
		const MinionSquad& squad = BossSquadAt(call, true);

		bool isHard = false;
		for (int index = BOSS_HARD_SQUAD_FROM; index < BOSS_SQUAD_COUNT; index++)
		{
			isHard = isHard || &squad == &BOSS_SQUADS[index];
		}

		EXPECT_TRUE(isHard) << "an enraged boss called " << squad.name;
	}
}

TEST(BossSquadTests, EveryHeavySquadOutweighsEveryLightOne)
{
	for (int heavy = BOSS_HARD_SQUAD_FROM; heavy < BOSS_SQUAD_COUNT; heavy++)
	{
		for (int light = 0; light < BOSS_HARD_SQUAD_FROM; light++)
		{
			EXPECT_GT(SquadToughness(BOSS_SQUADS[heavy]), SquadToughness(BOSS_SQUADS[light]))
				<< BOSS_SQUADS[heavy].name << " is no harder than " << BOSS_SQUADS[light].name;
		}
	}
}

TEST(BossSquadTests, TheBossCallsSomebodyWhoShoots)
{
	int shooting = 0;
	for (const MinionSquad& squad : BOSS_SQUADS)
	{
		shooting += HasShooter(squad) ? 1 : 0;
	}

	EXPECT_GT(shooting, 1) << "almost nobody the boss calls can shoot";
}

TEST(BossSquadTests, TwoFullSquadsFitUnderTheLimit)
{
	EXPECT_GE(BOSS_MINION_LIMIT, 2 * BOSS_SQUAD_SIZE) << "the second call is cut short by the limit";
}

TEST(BossSquadTests, AnIndexPastTheSquadWrapsInsteadOfReadingRubbish)
{
	const MinionSquad& squad = BOSS_SQUADS[0];

	EXPECT_EQ(BossMinionAt(squad, BOSS_SQUAD_SIZE), BossMinionAt(squad, 0));
	EXPECT_EQ(BossMinionAt(squad, -1), BossMinionAt(squad, 0));
}
