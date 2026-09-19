#include "CutscenePlayerComponent.h"
#include "CameraDirectorComponent.h"
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
            const CutsceneBeat* beat = timeline.GetCurrentBeat();
            if (beat != nullptr)
            {
                RunCommand(*beat);
            }

            const std::string& action = timeline.GetCurrentAction();
            LOG_INFO("Cutscene beat: " + action);
            beatStartedEvent.Invoke(action);
        }

        if (timeline.IsOver())
        {
            isPlaying = false;

            // Управление возвращается само: сцена, забывшая его отдать, вешает игру.
            ReleaseControl();

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

    // Сцену могли оборвать смертью или сменой локации: и камера, и управление возвращаются.
    void CutscenePlayerComponent::Stop()
    {
        isPlaying = false;

        // Начатая реплика не должна пережить сцену: на экране она осталась бы
        // висеть поверх игры, а звук из пула не остановить вовсе.
        if (hush != nullptr)
        {
            hush();
        }

        if (camera != nullptr)
        {
            camera->Release();
        }

        ReleaseControl();
    }

    void CutscenePlayerComponent::SetCamera(CameraDirectorComponent* newCamera)
    {
        camera = newCamera;
    }

    void CutscenePlayerComponent::SetControlLock(ControlLock newLock)
    {
        controlLock = std::move(newLock);
    }

    void CutscenePlayerComponent::SetSpeaker(Speaker newSpeaker)
    {
        speaker = std::move(newSpeaker);
    }

    void CutscenePlayerComponent::SetHush(Hush newHush)
    {
        hush = std::move(newHush);
    }

    void CutscenePlayerComponent::SetTargetFinder(std::function<XYZEngine::GameObject*(const std::string&)> newFinder)
    {
        findTarget = std::move(newFinder);
    }

    void CutscenePlayerComponent::ReleaseControl()
    {
        if (!hasTakenControl)
        {
            return;
        }

        hasTakenControl = false;

        if (controlLock != nullptr)
        {
            controlLock(false);
        }
    }

    void CutscenePlayerComponent::RunCommand(const CutsceneBeat& beat)
    {
        switch (beat.command)
        {
        case CutsceneCommand::TakeControl:
            if (!hasTakenControl)
            {
                hasTakenControl = true;
                if (controlLock != nullptr)
                {
                    controlLock(true);
                }
            }
            break;

        case CutsceneCommand::GiveControl:
            ReleaseControl();
            break;

        case CutsceneCommand::LookAtPoint:
            if (camera != nullptr)
            {
                camera->LookAt(beat.point, beat.travel);
            }
            break;

        case CutsceneCommand::LookAtTarget:
            if (camera != nullptr && findTarget != nullptr)
            {
                camera->LookAt(findTarget(beat.target), beat.travel);
            }
            break;

        case CutsceneCommand::Say:
            if (speaker != nullptr)
            {
                // Говорящего ищем тем же способом, что и цель камеры: реплику
                // может подать и герой, и любой объект на карте.
                speaker(beat.line, findTarget != nullptr && !beat.target.empty()
                    ? findTarget(beat.target) : nullptr);
            }
            break;

        case CutsceneCommand::LookAtHero:
            if (camera != nullptr)
            {
                camera->LookAtFollow(beat.travel);
            }
            break;

        default:
            break;
        }
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
