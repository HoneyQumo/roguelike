#include "Noise.h"
#include "ChaseComponent.h"
#include "FactionComponent.h"
#include "LevelGrid.h"
#include <GameObject.h>
#include <GameWorld.h>
#include <LoggerRegistry.h>
#include <TransformComponent.h>

namespace RoguelikeGame
{
    void RaiseNoise(const Noise& noise, const XYZEngine::GameObject* except)
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
            if (owner == except)
            {
                continue;
            }

            XYZEngine::Vector2Df place = owner->GetTransform()->GetWorldPosition();
            int walls = LevelGrid::Current().CountWallsBetween(noise.position, place);

            if (!IsHeard(noise, place, GetFactionOf(owner), walls))
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
