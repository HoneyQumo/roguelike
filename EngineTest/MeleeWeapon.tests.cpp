#include "pch.h"
#include "MeleeWeaponComponent.h"
#include "WeaponSetup.h"
#include <GameWorld.h>
#include <MovementComponent.h>

using RoguelikeGame::MeleeAttack;
using RoguelikeGame::MeleeWeaponComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::MovementComponent;

namespace
{
	class MeleeWeaponTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			owner = GameWorld::Instance()->CreateGameObject("Fighter");
			movement = owner->AddComponent<MovementComponent>();
			movement->SetSpeed(150.f);

			melee = owner->AddComponent<MeleeWeaponComponent>();

			MeleeAttack heavy;
			heavy.damage = 30.f;
			heavy.chargedDamage = 54.f;
			heavy.range = 60.f;
			heavy.arcDegrees = 110.f;
			heavy.recovery = 0.25f;

			melee->SetHeavyAttack(heavy);
			melee->SetChargeTime(0.5f);

			GameWorld::Instance()->Update(0.f);
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		GameObject* owner = nullptr;
		MovementComponent* movement = nullptr;
		MeleeWeaponComponent* melee = nullptr;
	};
}

// Замах вкапывает бойца в землю намеренно: тяжёлый удар бьют стоя.
TEST_F(MeleeWeaponTest, AHeavySwingHoldsTheFighterInPlace)
{
	ASSERT_TRUE(movement->IsEnabled());

	ASSERT_TRUE(melee->TryStartHeavyAttack());

	EXPECT_FALSE(movement->IsEnabled()) << "тяжёлый удар бьют на ходу";
	EXPECT_TRUE(melee->IsCharging());
}

/**
*	Отмена обязана вернуть движение.
*
*	На этом держатся все ветки, которые обрывают удар со стороны: перекат, смена
*	оружия, полученный урон, смерть и перехват указателя интерфейсом. Стоило одной
*	из них не позвать отмену - и боец оставался вкопанным, не держа кнопку.
*/
TEST_F(MeleeWeaponTest, CancellingASwingGivesTheLegsBack)
{
	ASSERT_TRUE(melee->TryStartHeavyAttack());
	ASSERT_FALSE(movement->IsEnabled());

	melee->CancelAttack();

	EXPECT_TRUE(movement->IsEnabled()) << "боец остался вкопанным после отмены";
	EXPECT_FALSE(melee->IsAttacking());
}

// Отмена без удара - обычное дело: её зовут из общих веток, не спрашивая.
TEST_F(MeleeWeaponTest, CancellingNothingChangesNothing)
{
	ASSERT_TRUE(movement->IsEnabled());

	melee->CancelAttack();

	EXPECT_TRUE(movement->IsEnabled());
	EXPECT_FALSE(melee->IsAttacking());
}

TEST_F(MeleeWeaponTest, TheChargeGrowsWhileTheSwingIsHeld)
{
	ASSERT_TRUE(melee->TryStartHeavyAttack());
	EXPECT_FLOAT_EQ(melee->GetChargeProgress(), 0.f);

	melee->Update(0.25f);
	EXPECT_GT(melee->GetChargeProgress(), 0.f);
	EXPECT_LT(melee->GetChargeProgress(), 1.f);

	melee->Update(0.5f);
	EXPECT_FLOAT_EQ(melee->GetChargeProgress(), 1.f);
	EXPECT_TRUE(melee->IsCharged());
}
