#pragma once

#include <string>
#include <AnimationClip.h>
#include <Component.h>
#include "BossSpriteAtlas.h"

namespace XYZEngine
{
    class GameObject;
    class SpriteRendererComponent;
}

namespace RoguelikeGame
{
    class BossAnimationComponent : public XYZEngine::Component
    {
    public:
        BossAnimationComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetTextureMap(const std::string& newTextureMapName);
        void SetAnimation(int slot, const BossAnimation& animation);

        void Play(int slot);
        void PlayTerminal(int slot);
        void ReleaseWindup();
        void SetIdleSlot(int newIdleSlot);

        int GetSlot() const;
        int GetFrame() const;
        bool IsWaitingInWindup() const;
        bool IsFinished() const;

    private:
        static constexpr int SLOTS = 7;

        XYZEngine::SpriteRendererComponent* renderer = nullptr;
        XYZEngine::AnimationClip clips[SLOTS];
        BossAnimation animations[SLOTS] = {};
        bool isLoaded[SLOTS] = {};

        std::string textureMapName;
        int slot = -1;
        int idleSlot = -1;
        bool isTerminal = false;
        int frame = 0;
        float frameTime = 0.f;
        bool isReleased = true;
        bool isFinished = true;

        void ShowFrame();
    };
}
