#pragma once

#include <string>
#include "Awareness.h"

namespace RoguelikeGame
{
    constexpr auto VOICE_KEY_PREFIX = "voice_";
    constexpr auto VOICE_AUDIO_PATH = "Resources/Audio/Voice/";
    constexpr auto VOICE_AUDIO_SUFFIX = ".wav";

    inline std::string VoiceKey(const char* voice, int line)
    {
        return voice == nullptr ? std::string() : VOICE_KEY_PREFIX + std::string(voice) + "_" + std::to_string(line);
    }

    inline std::string VoiceFilePath(const char* voice, int line)
    {
        std::string key = VoiceKey(voice, line);

        return key.empty() ? key : VOICE_AUDIO_PATH + key + VOICE_AUDIO_SUFFIX;
    }
}
