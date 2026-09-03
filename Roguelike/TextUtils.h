#pragma once

#include <SFML/System/String.hpp>
#include <cstring>

namespace RoguelikeGame
{
    inline sf::String FromUtf8(const char* text)
    {
        return sf::String::fromUtf8(text, text + std::strlen(text));
    }
}
