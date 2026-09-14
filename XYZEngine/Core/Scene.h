#pragma once

namespace XYZEngine
{
	class Scene
	{
	public:
		virtual ~Scene() = default;

		virtual void Start() = 0;
		virtual void Update(float deltaTime) = 0;
		virtual void Restart() = 0;
		virtual void Stop() = 0;
	};
}