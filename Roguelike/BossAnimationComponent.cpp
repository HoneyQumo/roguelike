#include "BossAnimationComponent.h"
#include <GameObject.h>
#include <SpriteRendererComponent.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    BossAnimationComponent::BossAnimationComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void BossAnimationComponent::Start()
    {
        renderer = gameObject->GetComponent<XYZEngine::SpriteRendererComponent>();
        if (renderer == nullptr)
        {
            LOG_ERROR("Boss animation needs a sprite renderer on " + gameObject->GetName());
            gameObject->DestroyComponent(this);
        }
    }

    void BossAnimationComponent::SetTextureMap(const std::string& newTextureMapName)
    {
        textureMapName = newTextureMapName;
    }

    void BossAnimationComponent::SetAnimation(int newSlot, const BossAnimation& animation)
    {
        if (newSlot < 0 || newSlot >= SLOTS || textureMapName.empty())
        {
            return;
        }

        int firstFrame = BossAtlasFrameIndex(animation.row, 0);
        if (!clips[newSlot].Load(textureMapName, firstFrame, animation.frames, 0.1f))
        {
            LOG_ERROR("Boss animation row " + std::to_string(animation.row) + " is not loaded from " + textureMapName);
            return;
        }

        clips[newSlot].SetFrameSeconds(animation.frameSeconds);
        animations[newSlot] = animation;
        isLoaded[newSlot] = true;
    }

    void BossAnimationComponent::SetIdleSlot(int newIdleSlot)
    {
        idleSlot = newIdleSlot;
    }

    void BossAnimationComponent::PlayTerminal(int newSlot)
    {
        Play(newSlot);
        isTerminal = true;
    }

    void BossAnimationComponent::Play(int newSlot)
    {
        if (newSlot < 0 || newSlot >= SLOTS || !isLoaded[newSlot] || slot == newSlot || isTerminal)
        {
            return;
        }

        slot = newSlot;
        frame = 0;
        frameTime = 0.f;
        isFinished = false;
        isReleased = animations[slot].windupLoopFirst < 0;

        ShowFrame();
    }

    void BossAnimationComponent::ReleaseWindup()
    {
        isReleased = true;
    }

    void BossAnimationComponent::Update(float deltaTime)
    {
        if (slot < 0 || isFinished)
        {
            return;
        }

        const BossAnimation& animation = animations[slot];

        frameTime += deltaTime;
        while (frameTime >= clips[slot].GetFrameSeconds(frame))
        {
            frameTime -= clips[slot].GetFrameSeconds(frame);

            if (!isReleased && frame >= animation.windupLoopLast)
            {
                frame = animation.windupLoopFirst;
                continue;
            }

            if (frame + 1 >= animation.frames)
            {
                if (animation.looped)
                {
                    frame = 0;
                    continue;
                }

                isFinished = true;
                break;
            }

            frame++;
        }

        ShowFrame();

        if (isFinished && !isTerminal && idleSlot >= 0 && slot != idleSlot)
        {
            Play(idleSlot);
        }
    }

    void BossAnimationComponent::Render()
    {
    }

    void BossAnimationComponent::ShowFrame()
    {
        if (renderer == nullptr || slot < 0)
        {
            return;
        }

        const sf::Texture* texture = clips[slot].GetFrame(frame);
        if (texture != nullptr)
        {
            renderer->SetTexture(*texture);
        }
    }

    int BossAnimationComponent::GetSlot() const
    {
        return slot;
    }

    int BossAnimationComponent::GetFrame() const
    {
        return frame;
    }

    bool BossAnimationComponent::IsWaitingInWindup() const
    {
        return slot >= 0 && !isReleased;
    }

    bool BossAnimationComponent::IsFinished() const
    {
        return isFinished;
    }
}
