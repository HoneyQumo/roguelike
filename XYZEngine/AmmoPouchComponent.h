#pragma once

#include <map>
#include "Component.h"

namespace XYZEngine
{
    class AmmoPouchComponent : public Component
    {
    public:
        AmmoPouchComponent(GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetAmmo(int ammoKind, int count);
        void AddAmmo(int ammoKind, int count);
        int GetAmmo(int ammoKind) const;

        int TakeAmmo(int ammoKind, int count);

    private:
        std::map<int, int> reserve;
    };
}
