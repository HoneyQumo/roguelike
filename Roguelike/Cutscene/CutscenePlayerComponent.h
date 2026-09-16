#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>
#include <Component.h>
#include <EventList.h>
#include "CutsceneTimeline.h"

namespace RoguelikeGame
{
    class CutscenePlayerComponent : public XYZEngine::Component
    {
    public:
        using BeatHandler = std::function<void(float)>;

        CutscenePlayerComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetBeats(std::vector<CutsceneBeat> beats);
        void SetHandler(const std::string& action, BeatHandler handler);

        void Play();
        void Stop();

        bool IsPlaying() const;
        const std::string& GetCurrentAction() const;

        XYZEngine::SubscriptionId SubscribeBeatStarted(std::function<void(const std::string&)> onBeatStarted);
        XYZEngine::SubscriptionId SubscribeFinished(std::function<void()> onFinished);

    private:
        CutsceneTimeline timeline;
        std::map<std::string, BeatHandler> handlers;
        bool isPlaying = false;

        XYZEngine::EventList<const std::string&> beatStartedEvent;
        XYZEngine::EventList<> finishedEvent;

        void RunBeat(float deltaTime);
    };
}
