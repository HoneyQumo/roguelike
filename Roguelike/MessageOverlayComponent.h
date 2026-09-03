#pragma once

#include <SFML/Graphics.hpp>
#include <Component.h>

namespace RoguelikeGame
{
    class MessageOverlayComponent : public XYZEngine::Component
    {
    public:
        MessageOverlayComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void Show(const char* title, const char* hint);
        void Hide();
        bool IsShown() const;

    private:
        sf::RectangleShape background;
        sf::Text titleText;
        sf::Text hintText;
        bool isFontReady = false;
        bool isShown = false;

        static void CenterOrigin(sf::Text& text);
    };
}
