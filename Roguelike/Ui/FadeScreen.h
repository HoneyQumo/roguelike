#pragma once

#include <UiPanel.h>
#include <UiScreen.h>

namespace RoguelikeGame
{
    class FadeScreen : public XYZEngine::UiScreen
    {
    public:
        FadeScreen();

        void FadeOut(float duration);
        void FadeIn(float duration);
        void Blackout();

        void Update(float deltaTime) override;

        float GetPart() const;
        bool IsBlackout() const;
        bool IsCovered() const;
        bool IsClear() const;

    private:
        XYZEngine::UiPanel* panel = nullptr;

        float part = 0.f;
        float target = 0.f;
        float speed = 0.f;

        void Apply();
    };
}
