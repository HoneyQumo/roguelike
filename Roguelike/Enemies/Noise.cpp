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
    namespace
    {
        XYZEngine::EventList<const Noise&> noiseEvent;
    }

    XYZEngine::SubscriptionId SubscribeNoiseRaised(std::function<void(const Noise&)> onNoise)
    {
        return noiseEvent.Subscribe(std::move(onNoise));
    }

    void UnsubscribeNoiseRaised(XYZEngine::SubscriptionId subscription)
    {
        noiseEvent.Unsubscribe(subscription);
    }

    void ClearNoiseListeners()
    {
        noiseEvent = XYZEngine::EventList<const Noise&>();
    }

    std::size_t NoiseListenerCount()
    {
        return noiseEvent.GetCount();
    }

    void RaiseNoise(const Noise& noise, const XYZEngine::GameObject* except)
    {
        if (noise.radius <= 0.f)
        {
            return;
        }

        noiseEvent.Invoke(noise);

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

            // Две дешёвые отсечки до подсчёта стен: шаги бегущего поднимают шум
            // несколько раз в секунду, и обход всей карты на каждый шаг дорог.
            Faction side = GetFactionOf(owner);
            if (!ReachesSide(noise, side))
            {
                continue;
            }

            XYZEngine::Vector2Df place = owner->GetTransform()->GetWorldPosition();
            if ((place - noise.position).GetLengthSquared() > noise.radius * noise.radius)
            {
                continue;
            }

            int walls = LevelGrid::Current().CountWallsBetween(noise.position, place);

            float loudness = LoudnessAt(noise, place, side, walls);
            if (loudness <= NOISE_HEARD_AT)
            {
                continue;
            }

            listener->Hear(noise.position, loudness);
            heard++;
        }

        if (heard > 0)
        {
            LOG_INFO("Noise at " + std::to_string(static_cast<int>(noise.position.x)) + ";"
                + std::to_string(static_cast<int>(noise.position.y)) + " is heard by " + std::to_string(heard));
        }
    }
}
