#include "Noise.h"
#include "ChaseComponent.h"
#include "FactionComponent.h"
#include <GameObject.h>
#include <GameWorld.h>
#include <LoggerRegistry.h>
#include <TransformComponent.h>

namespace RoguelikeGame
{
    void RaiseNoise(const Noise& noise)
    {
        if (noise.radius <= 0.f)
        {
            return;
        }

        int heard = 0;
        for (ChaseComponent* listener : XYZEngine::GameWorld::Instance()->FindComponents<ChaseComponent>())
        {
            if (listener == nullptr || !listener->IsEnabled())
            {
                continue;
            }

            XYZEngine::GameObject* owner = listener->GetGameObject();
            if (!IsHeard(noise, owner->GetTransform()->GetWorldPosition(), GetFactionOf(owner)))
            {
                continue;
            }

            listener->Hear(noise.position);
            heard++;
        }

        if (heard > 0)
        {
            LOG_INFO("Noise at " + std::to_string(static_cast<int>(noise.position.x)) + ";"
                + std::to_string(static_cast<int>(noise.position.y)) + " is heard by " + std::to_string(heard));
        }
    }
}
