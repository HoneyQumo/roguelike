#include "BossEffects.h"
#include "CastMark.h"
#include "ChaseComponent.h"
#include "Enemy.h"
#include <GameWorld.h>
#include "EnemyCatalog.h"
#include "Fx.h"
#include "GameSettings.h"
#include "ParticleCatalog.h"
#include "Projectile.h"
#include <GameObject.h>
#include <ParticleSystem.h>
#include <TransformComponent.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    void SpawnProjectilesOnBossVolley(BossBrainComponent* brain, XYZEngine::GameObject* boss, WeaponId weapon)
    {
        brain->SubscribeShot([boss, weapon](const XYZEngine::Vector2Df& origin, const XYZEngine::Vector2Df& direction,
            float damage, float speed)
        {
            Projectile::Spawn(origin, direction, damage, speed, boss, weapon);
        });
    }

    void SummonMinionsOnBossCall(BossBrainComponent* brain)
    {
        XYZEngine::GameObject* boss = brain->GetGameObject();

        brain->SubscribeAbilityUsed([boss](BossAbility ability)
        {
            if (ability == BossAbility::Summon)
            {
                Fx::SpawnPuppeteerRift(boss->GetTransform()->GetWorldPosition(), BOSS_RIFT_RADIUS);
            }
        });

        brain->SubscribeSummon([brain](const XYZEngine::Vector2Df& point, TileType kind)
        {
            Fx::SpawnPuppeteerRift(point, BOSS_MINION_RIFT_RADIUS);

            const EnemyConfig* config = FindEnemyConfig(kind);
            if (config == nullptr)
            {
                return;
            }

            try
            {
                XYZEngine::GameObject* minion = CreateEnemy(*config, point);
                if (auto chase = minion->GetComponent<ChaseComponent>())
                {
                    XYZEngine::GameObject* target = XYZEngine::GameWorld::Instance()->FindGameObject(chase->GetTargetName());
                    chase->Provoke(target == nullptr ? point : target->GetTransform()->GetWorldPosition());

                    // Свита выходит из портала уже зная, за кем пришла, и цель больше не теряет.
                    chase->SetForcedChase(true);
                }

                brain->RegisterMinion(minion);
            }
            catch (const std::exception& exception)
            {
                LOG_ERROR(std::string("Boss minion is not created: ") + exception.what());
            }
        });
    }

    void PlayEffectsOnBossBlast(BossBrainComponent* brain)
    {
        brain->SubscribeBlast([](const XYZEngine::Vector2Df& center, float radius)
        {
            Fx::SpawnPuppeteerCloud(center, radius);
            Fx::ShakeCamera(CAMERA_SHAKE_HEAVY);
        });
    }

    void PlayEffectsOnBossRage(BossBrainComponent* brain, HitFlashComponent* hitFlash, HealthBarComponent* healthBar,
        XYZEngine::ParticleAuraComponent* rageAura)
    {
        XYZEngine::GameObject* boss = brain->GetGameObject();

        brain->SubscribeEnraged([hitFlash, healthBar, rageAura, boss]()
        {
            if (hitFlash != nullptr)
            {
                hitFlash->Flash();
            }

            if (healthBar != nullptr)
            {
                healthBar->SetColors(BOSS_ENRAGED_BAR_COLOR, VITALS_HUD_BACK_COLOR);
            }

            if (rageAura != nullptr)
            {
                rageAura->SetActive(true);
            }

            if (const XYZEngine::ParticleSpec* burst = FindParticleSpec(ParticleEffect::RageBurst))
            {
                XYZEngine::ParticleSystem::Instance()->Emit(*burst, boss->GetTransform()->GetWorldPosition(), {0.f, 1.f});
            }

            Fx::ShakeCamera(CAMERA_SHAKE_HEAVY);
        });

        brain->SubscribeStateChanged([rageAura](BossState, BossState next)
        {
            if (next == BossState::Death && rageAura != nullptr)
            {
                rageAura->SetActive(false);
            }
        });
    }

    void PlayBossAnimations(BossBrainComponent* brain, BossAnimationComponent* animation, HealthComponent* health)
    {
        animation->SetIdleSlot(BOSS_SLOT_IDLE);
        animation->Play(BOSS_SLOT_IDLE);

        brain->SubscribeStateChanged([animation](BossState, BossState next)
        {
            switch (next)
            {
            case BossState::Chase:
                animation->Play(BOSS_SLOT_GLIDE);
                break;

            case BossState::Idle:
            case BossState::Cooldown:
                animation->Play(BOSS_SLOT_IDLE);
                break;

            case BossState::Enraged:
                animation->Play(BOSS_SLOT_ENRAGE);
                break;

            case BossState::Death:
                animation->PlayTerminal(BOSS_SLOT_DEATH);
                break;

            default:
                break;
            }
        });

        brain->SubscribeAbilityUsed([animation](BossAbility ability)
        {
            if (ability == BossAbility::Summon)
            {
                animation->Play(BOSS_SLOT_SUMMON);
            }
            else if (ability == BossAbility::Blast)
            {
                animation->Play(BOSS_SLOT_CURSE);
            }
        });

        brain->SubscribeSummon([animation](const XYZEngine::Vector2Df&, TileType) { animation->ReleaseWindup(); });
        brain->SubscribeCastMark([animation](const XYZEngine::Vector2Df&, float, float) { animation->ReleaseWindup(); });

        if (health != nullptr)
        {
            health->SubscribeDamage([animation](const DamageInfo& damage)
            {
                Fx::SpawnStringSnap(damage.source.position);

                if (!animation->IsWaitingInWindup())
                {
                    animation->Play(BOSS_SLOT_HURT);
                }
            });
        }
    }

    void ShowMarkOnBossCast(BossBrainComponent* brain)
    {
        XYZEngine::GameObject* boss = brain->GetGameObject();

        brain->SubscribeCastMark([brain, boss](const XYZEngine::Vector2Df& point, float radius, float fuseTime)
        {
            XYZEngine::GameObject* markObject = CreateCastMark(point, radius, fuseTime);

            auto mark = markObject->GetComponent<CastMarkComponent>();
            mark->SetTargetName(PLAYER_OBJECT_NAME);
            mark->SetOwnerName(boss->GetName());
            mark->SetOnDetonate([brain](const XYZEngine::Vector2Df& center) { brain->DetonateBlast(center); });
        });
    }
}
