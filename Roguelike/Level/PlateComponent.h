#pragma once

#include "SwitchComponent.h"

namespace XYZEngine
{
    class GameObject;
}

namespace RoguelikeGame
{
    class TreadComponent;

    /**
    *	Нажимная плитка - тот же выключатель, что и рычаг: то же имя, то же
    *	событие, та же связь через Openable. Разница одна - её давит нога,
    *	а не рука, поэтому подсказки она не показывает и нажать её нельзя.
    */
    class PlateComponent : public SwitchComponent
    {
    public:
        PlateComponent(XYZEngine::GameObject* gameObject);

        void SetTread(TreadComponent* tread);

        std::string GetPrompt(XYZEngine::GameObject* actor) const override;
        bool IsAvailable() const override;
        bool Interact(XYZEngine::GameObject* actor) override;

    private:
        void OnStep(XYZEngine::GameObject* walker);
    };
}
