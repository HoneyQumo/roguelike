#pragma once

#include <GameObject.h>
#include <SpriteAnimationComponent.h>

namespace RoguelikeGame
{
    /**	Лужа под трупом. Кровь отдельный слой, поэтому она строится
    *	до того, как появится её владелец, и только потом к нему цепляется.
    */
    class BloodPool
    {
    public:
        BloodPool();

        void AttachTo(XYZEngine::GameObject* owner);
        XYZEngine::SpriteAnimationComponent* GetAnimation();

    private:
        XYZEngine::GameObject* gameObject;
        XYZEngine::SpriteAnimationComponent* animation = nullptr;
    };
}
