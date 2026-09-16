#include "Fire.h"
#include "FireComponent.h"
#include "Fx.h"
#include "GameSettings.h"
#include "SpriteAtlas.h"
#include <GameObject.h>
#include <GameWorld.h>
#include <LoggerRegistry.h>
#include <TransformComponent.h>
#include <randomizer.h>

namespace RoguelikeGame
{
    namespace
    {
        XYZEngine::GameObject* Make(const std::string& name, const std::string& texture, const FxStrip& strip,
            float seconds, float radius, float damage)
        {
            auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(name);
            gameObject->SetRenderLayer(FIRE_RENDER_LAYER);

            Fx::AddAnimation(gameObject, texture, strip);

            auto fire = gameObject->AddComponent<FireComponent>();
            fire->SetRadius(radius);
            fire->SetDamage(damage);
            fire->SetBeatTime(FIRE_BEAT_TIME);
            fire->Light(seconds);

            fire->SubscribeBurnedOut([gameObject]()
            {
                XYZEngine::GameWorld::Instance()->DestroyGameObject(gameObject);
            });

            return gameObject;
        }
    }

    XYZEngine::GameObject* CreateFire(const XYZEngine::Vector2Df& position, float seconds)
    {
        XYZEngine::GameObject* fire = Make(FIRE_OBJECT_NAME, FIRE_BIG_TEXTURE, FX_FIRE_BIG, seconds, FIRE_RADIUS, FIRE_DAMAGE);
        fire->GetTransform()->SetWorldPosition(position);

        return fire;
    }

    XYZEngine::GameObject* CreateEmber(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to, float seconds)
    {
        XYZEngine::GameObject* ember = Make(FIRE_OBJECT_NAME, FIRE_SMALL_TEXTURE, FX_FIRE_SMALL, seconds, EMBER_RADIUS, EMBER_DAMAGE);
        ember->GetComponent<FireComponent>()->Throw(from, to, EMBER_FLIGHT_TIME);

        return ember;
    }

    int ScatterEmbers(const XYZEngine::Vector2Df& center, float radius, int count)
    {
        int scattered = 0;

        for (int index = 0; index < count; index++)
        {
            // Куда именно полетит очаг, решает случай: ровное кольцо выглядело бы как узор.
            float angle = random<float>(0.f, 6.2832f);
            float away = radius * random<float>(EMBER_NEAR_PART, 1.f);
            float seconds = random<float>(EMBER_BURN_MIN, EMBER_BURN_MAX);

            XYZEngine::Vector2Df place = {center.x + std::cos(angle) * away, center.y + std::sin(angle) * away};

            if (CreateEmber(center, place, seconds) != nullptr)
            {
                scattered++;
            }
        }

        LOG_INFO("Embers scattered: " + std::to_string(scattered));

        return scattered;
    }
}
