#include "FadeScreen.h"
#include "GameSettings.h"
#include <algorithm>

namespace RoguelikeGame
{
    FadeScreen::FadeScreen()
    {
        panel = GetRoot().AddChild<XYZEngine::UiPanel>();
        panel->SetStretch(true, true);
        panel->SetFillColor(LEVEL_FADE_COLOR);

        SetVisible(false);
    }

    void FadeScreen::FadeOut(float duration)
    {
        target = 1.f;
        speed = duration > 0.f ? 1.f / duration : 0.f;

        if (speed <= 0.f)
        {
            part = target;
        }

        SetVisible(true);
        Apply();
    }

    void FadeScreen::FadeIn(float duration)
    {
        target = 0.f;
        speed = duration > 0.f ? 1.f / duration : 0.f;

        if (speed <= 0.f)
        {
            part = target;
        }

        SetVisible(true);
        Apply();
    }

    void FadeScreen::Blackout()
    {
        part = 1.f;
        target = 1.f;
        speed = 0.f;

        SetVisible(true);
        Apply();
    }

    void FadeScreen::Update(float deltaTime)
    {
        if (part == target)
        {
            SetVisible(part > 0.f);
            return;
        }

        float step = speed * deltaTime;
        part = target > part ? std::min(target, part + step) : std::max(target, part - step);

        Apply();
        SetVisible(part > 0.f);
    }

    void FadeScreen::Apply()
    {
        sf::Color color = LEVEL_FADE_COLOR;
        color.a = static_cast<sf::Uint8>(std::clamp(part, 0.f, 1.f) * LEVEL_FADE_COLOR.a);

        panel->SetFillColor(color);
    }

    float FadeScreen::GetPart() const
    {
        return part;
    }

    bool FadeScreen::IsBlackout() const
    {
        return part >= 1.f;
    }

    bool FadeScreen::IsCovered() const
    {
        return target >= 1.f && part >= 1.f;
    }

    bool FadeScreen::IsClear() const
    {
        return part <= 0.f;
    }
}
