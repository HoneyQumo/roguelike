#pragma once

#include <UiLabel.h>
#include <UiPanel.h>
#include <UiScreen.h>

namespace RoguelikeGame
{
    class MessageScreen : public XYZEngine::UiScreen
    {
    public:
        MessageScreen();

        void Show(const char* title, const char* hint);
        void Hide();
        bool IsShown() const;

    private:
        XYZEngine::UiPanel* background = nullptr;
        XYZEngine::UiLabel* titleLabel = nullptr;
        XYZEngine::UiLabel* hintLabel = nullptr;
    };
}
