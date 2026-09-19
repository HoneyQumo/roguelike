#pragma once

#include <string>
#include <Vector.h>

namespace XYZEngine
{
    class GameObject;
}

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateLever(const std::string& switchId, const XYZEngine::Vector2Df& position, float holdTime = 0.f);
    XYZEngine::GameObject* CreatePlate(const std::string& plateId, const XYZEngine::Vector2Df& position);
    XYZEngine::GameObject* CreateHatch(const std::string& hatchId, const XYZEngine::Vector2Df& position);
    XYZEngine::GameObject* CreateEscapeCar(const std::string& carId, const XYZEngine::Vector2Df& position);
}
