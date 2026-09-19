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
    class CameraDirectorComponent;

    class CutscenePlayerComponent : public XYZEngine::Component
    {
    public:
        using BeatHandler = std::function<void(float)>;

        // Забрать у игрока управление и вернуть обратно. Как именно - решает уровень.
        using ControlLock = std::function<void(bool)>;

        // Проигрыватель не знает про речь напрямую: иначе он потянет за
        // собой каталог и звук и перестанет проверяться без них.
        using Speaker = std::function<void(const std::string&, XYZEngine::GameObject*)>;
        using Hush = std::function<void()>;

        CutscenePlayerComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetBeats(std::vector<CutsceneBeat> beats);
        void SetHandler(const std::string& action, BeatHandler handler);
        void SetCamera(CameraDirectorComponent* newCamera);
        void SetControlLock(ControlLock newLock);
        void SetSpeaker(Speaker newSpeaker);
        void SetHush(Hush newHush);
        void SetTargetFinder(std::function<XYZEngine::GameObject*(const std::string&)> newFinder);

        void Play();
        void Stop();

        bool IsPlaying() const;
        const std::string& GetCurrentAction() const;

        XYZEngine::SubscriptionId SubscribeBeatStarted(std::function<void(const std::string&)> onBeatStarted);
        XYZEngine::SubscriptionId SubscribeFinished(std::function<void()> onFinished);

    private:
        CutsceneTimeline timeline;
        std::map<std::string, BeatHandler> handlers;
        CameraDirectorComponent* camera = nullptr;
        ControlLock controlLock;
        Speaker speaker;
        Hush hush;
        std::function<XYZEngine::GameObject*(const std::string&)> findTarget;
        bool isPlaying = false;
        bool hasTakenControl = false;

        XYZEngine::EventList<const std::string&> beatStartedEvent;
        XYZEngine::EventList<> finishedEvent;

        void RunBeat(float deltaTime);
        void RunCommand(const CutsceneBeat& beat);
        void ReleaseControl();
    };
}
