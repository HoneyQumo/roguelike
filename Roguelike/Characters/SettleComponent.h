#pragma once

#include <functional>
#include <Component.h>

namespace RoguelikeGame
{
    class SettleComponent : public XYZEngine::Component
    {
    public:
        SettleComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetReadyCheck(std::function<bool()> newIsReady);

        bool IsSettled() const;
        void Settle();

    private:
        std::function<bool()> isReady;
        bool isSettled = false;
    };
}
