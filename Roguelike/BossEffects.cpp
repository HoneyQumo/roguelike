#include "BossEffects.h"
#include "Enemy.h"
#include "EnemyCatalog.h"
#include "Fx.h"
#include "GameSettings.h"
#include "Projectile.h"
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
        brain->SubscribeSummon([brain](const XYZEngine::Vector2Df& point)
        {
            const EnemyConfig* config = FindEnemyConfig(BOSS_MINION_TILE);
            if (config == nullptr)
            {
                return;
            }

            try
            {
                brain->RegisterMinion(CreateEnemy(*config, point));
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
            Fx::SpawnExplosion(center, radius);
            Fx::ShakeCamera(CAMERA_SHAKE_HEAVY);
        });
    }

    void PlayEffectsOnBossRage(BossBrainComponent* brain, HitFlashComponent* hitFlash, HealthBarComponent* healthBar)
    {
        brain->SubscribeEnraged([hitFlash, healthBar]()
        {
            if (hitFlash != nullptr)
            {
                hitFlash->Flash();
            }

            if (healthBar != nullptr)
            {
                healthBar->SetColors(BOSS_ENRAGED_BAR_COLOR, VITALS_HUD_BACK_COLOR);
            }

            Fx::ShakeCamera(CAMERA_SHAKE_HEAVY);
        });
    }
}
