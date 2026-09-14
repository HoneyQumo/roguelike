#include "pch.h"
#include "BossCatalog.h"
#include "EnemyCatalog.h"

using RoguelikeGame::ApplyBossScales;
using RoguelikeGame::BossAbility;
using RoguelikeGame::BossAbilitySlot;
using RoguelikeGame::BossBrainInput;
using RoguelikeGame::BossDefinition;
using RoguelikeGame::BossState;
using RoguelikeGame::BOSSES;
using RoguelikeGame::EnemyConfig;
using RoguelikeGame::FindBoss;
using RoguelikeGame::NextBossState;

namespace
{
	EnemyConfig MakeConfig()
	{
		EnemyConfig config{};
		config.objectName = "Boss";
		config.textureMapName = "enemy_boss";
		config.speed = 90.f;
		config.detectionRadius = 500.f;
		config.stopDistance = 170.f;
		config.maxHealth = 260.f;
		config.armor = 18.f;
		config.attackRange = 260.f;
		config.attackDamage = 16.f;
		config.attackCooldown = 1.f;
		config.projectileSpeed = 900.f;

		return config;
	}

	BossBrainInput Alive()
	{
		BossBrainInput input;
		input.isAlive = true;

		return input;
	}
}

TEST(BossCatalogTest, EveryBossHasTwoDifferentAbilities)
{
	for (const BossDefinition& boss : BOSSES)
	{
		EXPECT_NE(boss.first, boss.second) << boss.id;
		EXPECT_NE(boss.first, BossAbility::None) << boss.id;
		EXPECT_NE(boss.second, BossAbility::None) << boss.id;
		EXPECT_NE(boss.first, BossAbility::Basic) << boss.id;
		EXPECT_NE(boss.second, BossAbility::Basic) << boss.id;
	}
}

TEST(BossCatalogTest, AbilityPairsAreUniqueBetweenBosses)
{
	auto samePair = [](const BossDefinition& left, const BossDefinition& right)
	{
		return (left.first == right.first && left.second == right.second)
			|| (left.first == right.second && left.second == right.first);
	};

	for (const BossDefinition& left : BOSSES)
	{
		for (const BossDefinition& right : BOSSES)
		{
			if (&left == &right)
			{
				continue;
			}

			EXPECT_FALSE(samePair(left, right)) << left.id << " and " << right.id;
		}
	}
}

TEST(BossCatalogTest, BossIdsAreUniqueAndNotEmpty)
{
	for (const BossDefinition& left : BOSSES)
	{
		EXPECT_STRNE(left.id, "");

		int found = 0;
		for (const BossDefinition& right : BOSSES)
		{
			if (std::string_view(left.id) == right.id)
			{
				found++;
			}
		}

		EXPECT_EQ(found, 1) << left.id;
	}
}

TEST(BossCatalogTest, EnragePartIsInsideZeroOne)
{
	for (const BossDefinition& boss : BOSSES)
	{
		EXPECT_GT(boss.enragePart, 0.f) << boss.id;
		EXPECT_LT(boss.enragePart, 1.f) << boss.id;
	}
}

TEST(BossCatalogTest, FindBossResolvesLevelConfigIds)
{
	EXPECT_NE(FindBoss("puppeteer"), nullptr);
	EXPECT_NE(FindBoss("gravedigger"), nullptr);
	EXPECT_NE(FindBoss("colossus"), nullptr);
}

TEST(BossCatalogTest, FindBossRejectsUnknownIds)
{
	EXPECT_EQ(FindBoss(""), nullptr);
	EXPECT_EQ(FindBoss("Boss"), nullptr);
	EXPECT_EQ(FindBoss("Puppeteer"), nullptr);
	EXPECT_EQ(FindBoss("puppeteer "), nullptr);
}

TEST(BossCatalogTest, AbilitySlotIsFoundByAbility)
{
	const BossDefinition* boss = FindBoss("puppeteer");
	ASSERT_NE(boss, nullptr);

	EXPECT_EQ(BossAbilitySlot(*boss, boss->first), 0);
	EXPECT_EQ(BossAbilitySlot(*boss, boss->second), 1);
	EXPECT_EQ(BossAbilitySlot(*boss, BossAbility::None), -1);
	EXPECT_EQ(BossAbilitySlot(*boss, BossAbility::Basic), -1);
}

TEST(BossCatalogTest, ScalesMultiplyHealthAndDamage)
{
	EnemyConfig base = MakeConfig();

	EnemyConfig scaled = ApplyBossScales(base, 2.5f, 1.5f);

	EXPECT_FLOAT_EQ(scaled.maxHealth, 650.f);
	EXPECT_FLOAT_EQ(scaled.attackDamage, 24.f);
}

TEST(BossCatalogTest, ScalesKeepOtherFieldsAndSource)
{
	EnemyConfig base = MakeConfig();

	EnemyConfig scaled = ApplyBossScales(base, 3.f, 2.f);

	EXPECT_FLOAT_EQ(scaled.speed, base.speed);
	EXPECT_FLOAT_EQ(scaled.armor, base.armor);
	EXPECT_FLOAT_EQ(scaled.attackRange, base.attackRange);
	EXPECT_FLOAT_EQ(scaled.detectionRadius, base.detectionRadius);
	EXPECT_FLOAT_EQ(scaled.stopDistance, base.stopDistance);
	EXPECT_FLOAT_EQ(scaled.attackCooldown, base.attackCooldown);
	EXPECT_FLOAT_EQ(scaled.projectileSpeed, base.projectileSpeed);
	EXPECT_EQ(scaled.weapon, base.weapon);
	EXPECT_STREQ(scaled.objectName, base.objectName);

	EXPECT_FLOAT_EQ(base.maxHealth, 260.f);
	EXPECT_FLOAT_EQ(base.attackDamage, 16.f);
}

TEST(BossCatalogTest, ScalesOfOneKeepConfigAsIs)
{
	EnemyConfig base = MakeConfig();

	EnemyConfig scaled = ApplyBossScales(base, 1.f, 1.f);

	EXPECT_FLOAT_EQ(scaled.maxHealth, base.maxHealth);
	EXPECT_FLOAT_EQ(scaled.attackDamage, base.attackDamage);
}

TEST(BossCatalogTest, ScalesWorkOnTheCatalogEntry)
{
	const EnemyConfig* base = RoguelikeGame::FindEnemyConfig(RoguelikeGame::TileType::BossSpawn);
	ASSERT_NE(base, nullptr);

	EnemyConfig scaled = ApplyBossScales(*base, 2.f, 2.f);

	EXPECT_FLOAT_EQ(scaled.maxHealth, base->maxHealth * 2.f);
	EXPECT_FLOAT_EQ(scaled.attackDamage, base->attackDamage * 2.f);
}

TEST(BossStateTest, DeathWinsOverEveryState)
{
	BossBrainInput input = Alive();
	input.isAlive = false;
	input.isTargetDetected = true;
	input.isEnrageDue = true;

	for (BossState state : {BossState::Idle, BossState::Chase, BossState::Attack, BossState::Cooldown, BossState::Enraged})
	{
		EXPECT_EQ(NextBossState(state, input), BossState::Death);
	}
}

TEST(BossStateTest, DeathIsTerminal)
{
	BossBrainInput input = Alive();
	input.isTargetDetected = true;
	input.isRecoveryDone = true;

	EXPECT_EQ(NextBossState(BossState::Death, input), BossState::Death);
}

TEST(BossStateTest, IdleWaitsForTheTarget)
{
	BossBrainInput input = Alive();

	EXPECT_EQ(NextBossState(BossState::Idle, input), BossState::Idle);

	input.isTargetDetected = true;
	EXPECT_EQ(NextBossState(BossState::Idle, input), BossState::Chase);
}

TEST(BossStateTest, ChaseGoesBackToIdleWhenTargetIsLost)
{
	BossBrainInput input = Alive();
	input.chosen = BossAbility::Basic;

	EXPECT_EQ(NextBossState(BossState::Chase, input), BossState::Idle);
}

TEST(BossStateTest, ChaseAttacksOnlyWithChosenAbility)
{
	BossBrainInput input = Alive();
	input.isTargetDetected = true;

	EXPECT_EQ(NextBossState(BossState::Chase, input), BossState::Chase);

	input.chosen = BossAbility::Basic;
	EXPECT_EQ(NextBossState(BossState::Chase, input), BossState::Attack);
}

TEST(BossStateTest, AttackHoldsUntilTheActionIsDone)
{
	BossBrainInput input = Alive();
	input.isTargetDetected = true;

	EXPECT_EQ(NextBossState(BossState::Attack, input), BossState::Attack);

	input.isActionDone = true;
	EXPECT_EQ(NextBossState(BossState::Attack, input), BossState::Cooldown);
}

TEST(BossStateTest, CooldownHoldsUntilRecoveryIsDone)
{
	BossBrainInput input = Alive();
	input.isTargetDetected = true;
	input.chosen = BossAbility::Basic;

	EXPECT_EQ(NextBossState(BossState::Cooldown, input), BossState::Cooldown);

	input.isRecoveryDone = true;
	EXPECT_EQ(NextBossState(BossState::Cooldown, input), BossState::Chase);
}

TEST(BossStateTest, EnrageInterruptsAnyLivingState)
{
	BossBrainInput input = Alive();
	input.isEnrageDue = true;
	input.isTargetDetected = true;

	for (BossState state : {BossState::Idle, BossState::Chase, BossState::Attack, BossState::Cooldown})
	{
		EXPECT_EQ(NextBossState(state, input), BossState::Enraged);
	}
}

TEST(BossStateTest, EnrageDoesNotRestartItself)
{
	BossBrainInput input = Alive();
	input.isEnrageDue = true;

	EXPECT_EQ(NextBossState(BossState::Enraged, input), BossState::Enraged);
}

TEST(BossStateTest, RoarEndsInChase)
{
	BossBrainInput input = Alive();
	input.isRoarDone = true;

	EXPECT_EQ(NextBossState(BossState::Enraged, input), BossState::Chase);
}

TEST(BossStateTest, FullCycleRunsIdleChaseAttackCooldown)
{
	BossBrainInput input = Alive();
	BossState state = BossState::Idle;

	input.isTargetDetected = true;
	state = NextBossState(state, input);
	EXPECT_EQ(state, BossState::Chase);

	input.chosen = BossAbility::Basic;
	state = NextBossState(state, input);
	EXPECT_EQ(state, BossState::Attack);

	input.isActionDone = true;
	state = NextBossState(state, input);
	EXPECT_EQ(state, BossState::Cooldown);

	input.isRecoveryDone = true;
	state = NextBossState(state, input);
	EXPECT_EQ(state, BossState::Chase);
}

TEST(BossAbilityTest, EveryAbilityInTheCatalogHasASpec)
{
	for (const BossDefinition& boss : BOSSES)
	{
		EXPECT_NE(RoguelikeGame::FindBossAbility(boss.first), nullptr) << boss.id;
		EXPECT_NE(RoguelikeGame::FindBossAbility(boss.second), nullptr) << boss.id;
	}

	EXPECT_EQ(RoguelikeGame::FindBossAbility(BossAbility::Basic), nullptr);
	EXPECT_EQ(RoguelikeGame::FindBossAbility(BossAbility::None), nullptr);
}

TEST(BossAbilityTest, SpecsAreSane)
{
	for (const RoguelikeGame::BossAbilitySpec& spec : RoguelikeGame::BOSS_ABILITIES)
	{
		EXPECT_LT(spec.minDistance, spec.maxDistance);
		EXPECT_GT(spec.windup, 0.f);
		EXPECT_GT(spec.duration, 0.f);
		EXPECT_GT(spec.cooldown, spec.windup + spec.duration);
	}
}

TEST(BossAbilityTest, RangeIsCheckedByTheSpec)
{
	EXPECT_TRUE(RoguelikeGame::IsInBossAbilityRange(BossAbility::Volley, 400.f));
	EXPECT_FALSE(RoguelikeGame::IsInBossAbilityRange(BossAbility::Volley, 100.f));
	EXPECT_FALSE(RoguelikeGame::IsInBossAbilityRange(BossAbility::Volley, 900.f));
	EXPECT_FALSE(RoguelikeGame::IsInBossAbilityRange(BossAbility::Basic, 100.f));
}

TEST(BossAbilityTest, FirstSlotWinsWhenBothFit)
{
	const BossDefinition* boss = FindBoss("colossus");
	ASSERT_NE(boss, nullptr);

	EXPECT_EQ(RoguelikeGame::ChooseBossAbility(*boss, 400.f, 260.f, true, true), boss->first);
}

TEST(BossAbilityTest, SecondSlotIsTakenWhenFirstIsOnCooldown)
{
	const BossDefinition* boss = FindBoss("colossus");
	ASSERT_NE(boss, nullptr);

	EXPECT_EQ(RoguelikeGame::ChooseBossAbility(*boss, 400.f, 260.f, false, true), boss->second);
}

TEST(BossAbilityTest, SecondSlotIsTakenWhenFirstIsOutOfRange)
{
	const BossDefinition* boss = FindBoss("gravedigger");
	ASSERT_NE(boss, nullptr);

	EXPECT_EQ(RoguelikeGame::ChooseBossAbility(*boss, 500.f, 260.f, true, true), BossAbility::Dash);
}

TEST(BossAbilityTest, BasicIsTheFallbackInsideAttackRange)
{
	const BossDefinition* boss = FindBoss("colossus");
	ASSERT_NE(boss, nullptr);

	EXPECT_EQ(RoguelikeGame::ChooseBossAbility(*boss, 120.f, 260.f, true, true), BossAbility::Basic);
	EXPECT_EQ(RoguelikeGame::ChooseBossAbility(*boss, 400.f, 260.f, false, false), BossAbility::None);
}

TEST(BossAbilityTest, NothingIsChosenTooFarAway)
{
	const BossDefinition* boss = FindBoss("gravedigger");
	ASSERT_NE(boss, nullptr);

	EXPECT_EQ(RoguelikeGame::ChooseBossAbility(*boss, 5000.f, 260.f, true, true), BossAbility::None);
}

TEST(BossAbilityTest, ChoiceIsDecidedAtCompileTime)
{
	static_assert(RoguelikeGame::ChooseBossAbility(BOSSES[0], 400.f, 260.f, true, true) == BossAbility::Summon);
	static_assert(RoguelikeGame::ChooseBossAbility(BOSSES[0], 200.f, 260.f, false, true) == BossAbility::Blast);
	static_assert(RoguelikeGame::ChooseBossAbility(BOSSES[0], 400.f, 260.f, false, false) == BossAbility::None);
	static_assert(RoguelikeGame::ChooseBossAbility(BOSSES[2], 400.f, 260.f, true, true) == BossAbility::Dash);

	SUCCEED();
}

TEST(BossCatalogTest, OnlyBossesWithoutTheirOwnSheetCarryAWeapon)
{
	for (const BossDefinition& boss : BOSSES)
	{
		EXPECT_EQ(RoguelikeGame::HasWeaponLayer(boss), boss.textureMapName == nullptr) << boss.id;
	}
}

TEST(BossCatalogTest, PuppeteerHasNoWeaponAndNoBasicAttack)
{
	const BossDefinition* puppeteer = FindBoss("puppeteer");
	ASSERT_NE(puppeteer, nullptr);

	EXPECT_FALSE(RoguelikeGame::HasWeaponLayer(*puppeteer));
	EXPECT_FALSE(puppeteer->hasBasicAttack);
}

TEST(BossCatalogTest, BossesOnTheSharedSheetKeepTheirWeapon)
{
	for (const char* id : {"gravedigger", "colossus"})
	{
		const BossDefinition* boss = FindBoss(id);
		ASSERT_NE(boss, nullptr) << id;

		EXPECT_TRUE(RoguelikeGame::HasWeaponLayer(*boss)) << id;
		EXPECT_TRUE(boss->hasBasicAttack) << id;
	}
}

TEST(BossCatalogTest, ProvokedBossDoesNotForgetTheFight)
{
	BossBrainInput input = Alive();
	input.isTargetDetected = false;
	input.isProvoked = true;

	EXPECT_EQ(NextBossState(BossState::Chase, input), BossState::Chase);
}

TEST(BossCatalogTest, WithoutProvocationTheBossStillForgets)
{
	BossBrainInput input = Alive();
	input.isTargetDetected = false;
	input.isProvoked = false;

	EXPECT_EQ(NextBossState(BossState::Chase, input), BossState::Idle);
}

TEST(BossCatalogTest, ProvocationStartsTheFightFromIdle)
{
	BossBrainInput input = Alive();
	input.isTargetDetected = false;
	input.isProvoked = true;

	EXPECT_EQ(NextBossState(BossState::Idle, input), BossState::Chase);
}

TEST(BossCatalogTest, ProvocationDoesNotSaveADeadBoss)
{
	BossBrainInput input = Alive();
	input.isAlive = false;
	input.isProvoked = true;

	EXPECT_EQ(NextBossState(BossState::Chase, input), BossState::Death);
}

TEST(BossCatalogTest, ProvocationDoesNotBlockRage)
{
	BossBrainInput input = Alive();
	input.isEnrageDue = true;
	input.isProvoked = true;
	input.isTargetDetected = false;

	EXPECT_EQ(NextBossState(BossState::Chase, input), BossState::Enraged);
}
