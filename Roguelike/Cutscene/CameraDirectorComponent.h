#pragma once

#include <string>
#include <Component.h>
#include <Vector.h>

namespace XYZEngine
{
    class GameObject;
    class TransformComponent;
}

namespace RoguelikeGame
{
    /**
    *	Кто решает, куда смотрит камера.
    *
    *	Обычно она держится за игроком, но на время сцены может уехать к люку,
    *	к машине или к любой точке и потом вернуться. Переход плавный: камера
    *	доезжает за отведённое время, а не прыгает.
    */
    class CameraDirectorComponent : public XYZEngine::Component
    {
    public:
        CameraDirectorComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        // За кем камера держится, когда её никуда не увели.
        void SetFollow(XYZEngine::GameObject* newFollow);

        void LookAt(const XYZEngine::Vector2Df& point, float seconds);
        void LookAt(XYZEngine::GameObject* target, float seconds);
        void LookAtFollow(float seconds);

        // Немедленно вернуть камеру герою: на случай, когда сцену оборвали.
        void Release();

        bool IsAway() const;
        bool IsMoving() const;
        const XYZEngine::Vector2Df& GetAim() const;

    private:
        XYZEngine::TransformComponent* transform = nullptr;
        XYZEngine::GameObject* follow = nullptr;
        XYZEngine::GameObject* target = nullptr;

        XYZEngine::Vector2Df point = {0.f, 0.f};
        XYZEngine::Vector2Df from = {0.f, 0.f};
        XYZEngine::Vector2Df aim = {0.f, 0.f};

        float travel = 0.f;
        float travelled = 0.f;
        bool isAway = false;

        XYZEngine::Vector2Df Destination() const;
        void StartMove(float seconds);
    };
}
