#include "CutscenePlayerComponent.h"
#include <GameObject.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    CutscenePlayerComponent::CutscenePlayerComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void CutscenePlayerComponent::Update(float deltaTime)
    {
        if (!isPlaying)
        {
            return;
        }

        if (timeline.Advance(deltaTime))
        {
            const std::string& action = timeline.GetCurrentAction();
            LOG_INFO("Cutscene beat: " + action);
            beatStartedEvent.Invoke(action);
        }

        if (timeline.IsOver())
        {
            isPlaying = false;
            LOG_INFO("Cutscene is over");
            finishedEvent.Invoke();
            return;
        }

        RunBeat(deltaTime);
    }

    void CutscenePlayerComponent::Render()
    {
    }

    void CutscenePlayerComponent::SetBeats(std::vector<CutsceneBeat> beats)
    {
        timeline.SetBeats(std::move(beats));
    }

    void CutscenePlayerComponent::SetHandler(const std::string& action, BeatHandler handler)
    {
        handlers[action] = std::move(handler);
    }

    void CutscenePlayerComponent::Play()
    {
        if (timeline.IsEmpty())
        {
            LOG_WARN("Cutscene has nothing to play");
            return;
        }

        isPlaying = true;
    }

    void CutscenePlayerComponent::Stop()
    {
        isPlaying = false;
    }

    bool CutscenePlayerComponent::IsPlaying() const
    {
        return isPlaying;
    }

    const std::string& CutscenePlayerComponent::GetCurrentAction() const
    {
        return timeline.GetCurrentAction();
    }

    XYZEngine::SubscriptionId CutscenePlayerComponent::SubscribeBeatStarted(std::function<void(const std::string&)> onBeatStarted)
    {
        return beatStartedEvent.Subscribe(std::move(onBeatStarted));
    }

    XYZEngine::SubscriptionId CutscenePlayerComponent::SubscribeFinished(std::function<void()> onFinished)
    {
        return finishedEvent.Subscribe(std::move(onFinished));
    }

    void CutscenePlayerComponent::RunBeat(float deltaTime)
    {
        auto found = handlers.find(timeline.GetCurrentAction());
        if (found != handlers.end() && found->second != nullptr)
        {
            found->second(deltaTime);
        }
    }
}
