#include "pch.h"
#include "BoxColliderComponent.h"
#include "CritRules.h"
#include "FactionComponent.h"
#include "HealthComponent.h"
#include "MeleeWeaponComponent.h"
#include "WeaponSetup.h"
#include <GameWorld.h>
#include <MovementComponent.h>
#include <MathUtils.h>
#include <TransformComponent.h>
#include <cmath>

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

			MeleeAttack quick;
			quick.damage = 10.f;
			quick.chargedDamage = 10.f;
			quick.range = 60.f;
			quick.arcDegrees = 90.f;
			quick.recovery = 0.1f;
			quick.critScale = 2.f;

			melee->SetQuickAttack(quick);

			GameWorld::Instance()->Update(0.f);
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		// Ящик без стороны, но с поворотом: бьём с заданного направления и смотрим урон.
		float StrikeCrate(const XYZEngine::Vector2Df& crateForward)
		{
			GameObject* crate = GameWorld::Instance()->CreateGameObject("Crate");
			crate->GetTransform()->SetWorldPosition({40.f, 0.f});
			crate->GetTransform()->SetWorldRotation(XYZEngine::ToDegrees(std::atan2(crateForward.y, crateForward.x)));
			crate->AddComponent<XYZEngine::BoxColliderComponent>()->SetSize(40.f, 40.f);

			auto health = crate->AddComponent<RoguelikeGame::HealthComponent>();
			health->SetMaxHealth(500.f);

			owner->GetTransform()->SetWorldPosition({0.f, 0.f});
			owner->GetTransform()->SetWorldRotation(0.f);
			GameWorld::Instance()->Update(0.f);
			GameWorld::Instance()->UpdatePhysics();

			// Между ударами оружие отходит: без этого второй удар просто не выйдет.
			for (int frame = 0; frame < 40 && !melee->IsReady(); frame++)
			{
				GameWorld::Instance()->Update(0.05f);
			}

			melee->TryQuickAttack();
			for (int frame = 0; frame < 20 && health->GetHealth() >= 500.f; frame++)
			{
				GameWorld::Instance()->Update(0.05f);
			}

			float taken = 500.f - health->GetHealth();

			GameWorld::Instance()->DestroyGameObject(crate);
			GameWorld::Instance()->LateUpdate();

			return taken;
		}

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

// На отмене держатся перекат, смена оружия, урон, смерть и перехват указателя.
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


// Чистая геометрия: куда смотрит цель и с какой стороны пришёл удар.
// Крит в спину - про того, у кого есть спина. У ящика её нет, а поворот есть,
// и без проверки стороны урон по нему зависел от того, с какой диагонали подошли.
TEST_F(MeleeWeaponTest, ACrateTakesTheSameDamageFromAnySide)
{
	float fromBehind = StrikeCrate({1.f, 0.f});
	float fromTheFace = StrikeCrate({-1.f, 0.f});

	EXPECT_GT(fromBehind, 0.f);
	EXPECT_FLOAT_EQ(fromBehind, fromTheFace) << "у ящика нашлась спина";
}

TEST(BackstabTest, AHitFromBehindIsACrit)
{
	// Цель смотрит вправо, удар пришёл слева и летит вправо - бьют в спину.
	EXPECT_TRUE(RoguelikeGame::IsBackstab({1.f, 0.f}, {1.f, 0.f}));
}

TEST(BackstabTest, AHitInTheFaceIsNotACrit)
{
	EXPECT_FALSE(RoguelikeGame::IsBackstab({1.f, 0.f}, {-1.f, 0.f}));
}

TEST(BackstabTest, AHitFromTheSideIsNotACrit)
{
	EXPECT_FALSE(RoguelikeGame::IsBackstab({1.f, 0.f}, {0.f, 1.f}));
	EXPECT_FALSE(RoguelikeGame::IsBackstab({1.f, 0.f}, {0.f, -1.f}));
}

// Задняя полусфера шириной 120 градусов: по шестьдесят в каждую сторону от спины.
TEST(BackstabTest, TheBackIsAHemisphereAndNotAPoint)
{
	XYZEngine::Vector2Df forward = {1.f, 0.f};

	EXPECT_TRUE(RoguelikeGame::IsBackstab(forward, {0.9f, 0.4f})) << "чуть сбоку уже не спина";
	EXPECT_FALSE(RoguelikeGame::IsBackstab(forward, {0.4f, 0.9f})) << "почти сбоку считается спиной";
}

// У босса и разрушаемого ящика мозга нет, а поворот есть - отдельной ветки не нужно.
TEST(BackstabTest, AThingWithoutAFacingIsNeverHitInTheBack)
{
	EXPECT_FALSE(RoguelikeGame::IsBackstab({0.f, 0.f}, {1.f, 0.f}));
	EXPECT_FALSE(RoguelikeGame::IsBackstab({1.f, 0.f}, {0.f, 0.f}));
}

TEST(BackstabTest, OnlyAMultiplierAboveOneIsACrit)
{
	EXPECT_FLOAT_EQ(RoguelikeGame::BackstabDamage(10.f, true, 2.f), 20.f);
	EXPECT_FLOAT_EQ(RoguelikeGame::BackstabDamage(10.f, false, 2.f), 10.f);
	EXPECT_FLOAT_EQ(RoguelikeGame::BackstabDamage(10.f, true, 1.f), 10.f) << "множитель один даёт прибавку";
	EXPECT_FLOAT_EQ(RoguelikeGame::BackstabDamage(10.f, true, 0.5f), 10.f) << "множитель ниже единицы ослабил удар";
}

namespace
{
	class BackstabStrikeTest : public MeleeWeaponTest
	{
	protected:
		// Боец стоит слева от цели и смотрит вправо, на неё.
		float StrikeAndMeasure(float victimFacingDegrees)
		{
			owner->GetTransform()->SetWorldPosition({0.f, 0.f});
			owner->GetTransform()->SetWorldRotation(0.f);

			GameObject* victim = GameWorld::Instance()->CreateGameObject("Victim");
			victim->GetTransform()->SetWorldPosition({40.f, 0.f});
			victim->GetTransform()->SetWorldRotation(victimFacingDegrees);
			victim->AddComponent<XYZEngine::BoxColliderComponent>()->SetSize(30.f, 30.f);

			// У жертвы есть сторона: крит в спину - про живых, а не про ящики.
			victim->AddComponent<RoguelikeGame::FactionComponent>()->SetFaction(RoguelikeGame::Faction::Enemy);

			auto health = victim->AddComponent<RoguelikeGame::HealthComponent>();
			health->SetMaxHealth(1000.f);

			GameWorld::Instance()->Update(0.016f);
			GameWorld::Instance()->UpdatePhysics();

			EXPECT_TRUE(melee->TryQuickAttack()) << "удар не начался";

			for (int frame = 0; frame < 30 && health->GetHealth() >= 1000.f; frame++)
			{
				GameWorld::Instance()->Update(0.02f);
			}

			return 1000.f - health->GetHealth();
		}
	};
}

// Главный смысл всей затеи: зайти за спину выгодно.
TEST_F(BackstabStrikeTest, AHitInTheBackIsDoubled)
{
	EXPECT_FLOAT_EQ(StrikeAndMeasure(0.f), 20.f) << "удар в спину не получил множителя";
}

TEST_F(BackstabStrikeTest, AHitInTheFaceIsPlain)
{
	EXPECT_FLOAT_EQ(StrikeAndMeasure(180.f), 10.f) << "удар в лицо считается критом";
}

// Сбоку множителя быть не должно, иначе крит получается сам собой.
TEST_F(BackstabStrikeTest, AHitFromTheSideIsPlain)
{
	EXPECT_FLOAT_EQ(StrikeAndMeasure(90.f), 10.f);
}
