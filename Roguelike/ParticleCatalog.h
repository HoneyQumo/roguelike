#pragma once

#include <ParticleSpec.h>
#include "SpriteAtlas.h"

namespace RoguelikeGame
{
    enum class ParticleEffect
    {
        HealBurst,
        HitBurst
    };

    struct ParticleEffectDefinition
    {
        ParticleEffect effect;
        const char* name;
        XYZEngine::ParticleSpec spec;
    };

    constexpr XYZEngine::ParticleFrame FxFrame(const FxStrip& strip, int frame = 0)
    {
        return {strip.x + strip.width * frame, strip.y, strip.width, strip.height};
    }

    inline constexpr ParticleEffectDefinition PARTICLE_EFFECTS[] = {
        {
            ParticleEffect::HealBurst, "HealBurst",
            {
                22, 0.f, 0.f,
                0.6f, 0.3f,
                70.f, 15.f,
                {0.f, 60.f},
                12.f, 3.f,
                {130, 255, 160, 220}, {60, 210, 120, 0},
                180.f,
                FxFrame(FX_BLOOD_SPECK),
                XYZEngine::ParticleEmission::Burst,
                true
            }
        },
        {
            ParticleEffect::HitBurst, "HitBurst",
            {
                14, 0.f, 0.f,
                0.35f, 0.4f,
                260.f, 40.f,
                {0.f, -420.f},
                9.f, 2.f,
                {220, 60, 50, 255}, {120, 20, 20, 0},
                55.f,
                FxFrame(FX_BLOOD_SPECK),
                XYZEngine::ParticleEmission::Burst,
                false
            }
        },
    };

    constexpr const XYZEngine::ParticleSpec* FindParticleSpec(ParticleEffect effect)
    {
        for (const ParticleEffectDefinition& definition : PARTICLE_EFFECTS)
        {
            if (definition.effect == effect)
            {
                return &definition.spec;
            }
        }

        return nullptr;
    }

    static_assert(FindParticleSpec(ParticleEffect::HealBurst) != nullptr, "HealBurst is required by the heal feedback");
    static_assert(FindParticleSpec(ParticleEffect::HitBurst) != nullptr, "HitBurst is required by the hit feedback");
}
