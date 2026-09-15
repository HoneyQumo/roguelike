#pragma once

namespace RoguelikeGame
{
    constexpr float ARMOR_ABSORB_SHARE = 0.6f;

    struct ArmorHit
    {
        float toArmor = 0.f;
        float toHealth = 0.f;
    };

    constexpr ArmorHit SplitDamage(float damage, float armor, float share = ARMOR_ABSORB_SHARE)
    {
        if (damage <= 0.f)
        {
            return {};
        }

        if (armor <= 0.f || share <= 0.f)
        {
            return {0.f, damage};
        }

        float wanted = damage * (share > 1.f ? 1.f : share);
        float taken = wanted < armor ? wanted : armor;

        return {taken, damage - taken};
    }
}
