#include "pch.h"
#include "WeaponSetup.h"

using namespace RoguelikeGame;

namespace
{
	constexpr MeleeAttackProfile PROFILE = {1.5f, 3.f, 60.f, 90.f, 0.25f};
}

TEST(WeaponSetupTests, QuickAttackScalesDamageAndTakesRecoveryFromCaller)
{
	XYZEngine::MeleeAttack attack = MakeQuickAttack(PROFILE, 20.f, 0.7f);

	EXPECT_FLOAT_EQ(attack.damage, 30.f);
	EXPECT_FLOAT_EQ(attack.chargedDamage, 30.f);
	EXPECT_FLOAT_EQ(attack.range, 60.f);
	EXPECT_FLOAT_EQ(attack.arcDegrees, 90.f);
	EXPECT_FLOAT_EQ(attack.recovery, 0.7f);
	EXPECT_EQ(attack.hitFrame, MELEE_HIT_FRAME);
	EXPECT_FLOAT_EQ(attack.windup, MELEE_HIT_FRAME / MELEE_ANIMATION.framesPerSecond);
}

TEST(WeaponSetupTests, HeavyAttackUsesChargedScaleAndProfileRecovery)
{
	XYZEngine::MeleeAttack attack = MakeHeavyAttack(PROFILE, 20.f);

	EXPECT_FLOAT_EQ(attack.damage, 30.f);
	EXPECT_FLOAT_EQ(attack.chargedDamage, 60.f);
	EXPECT_FLOAT_EQ(attack.recovery, 0.25f);
	EXPECT_EQ(attack.hitFrame, HEAVY_HIT_FIRST_FRAME);
	EXPECT_FLOAT_EQ(attack.windup, HEAVY_FRAME_SECONDS[HEAVY_RELEASE_FRAME]);
}

TEST(WeaponSetupTests, CatalogMeleeProfilesProduceSaneAttacks)
{
	for (const MeleeDefinition& melee : MELEE_WEAPONS)
	{
		XYZEngine::MeleeAttack quick = MakeQuickAttack(melee.quick, PLAYER_MELEE_DAMAGE, melee.quick.recovery);
		XYZEngine::MeleeAttack heavy = MakeHeavyAttack(melee.heavy, PLAYER_MELEE_DAMAGE);

		EXPECT_GT(quick.damage, 0.f) << GetWeapon(melee.weapon).id;
		EXPECT_GT(quick.range, 0.f) << GetWeapon(melee.weapon).id;
		EXPECT_GT(quick.windup, 0.f) << GetWeapon(melee.weapon).id;
		EXPECT_GE(heavy.chargedDamage, heavy.damage) << GetWeapon(melee.weapon).id;
	}
}
