#pragma once
#define NOMINMAX

#include "SFML/Graphics.hpp"
#include "Scene.h"

namespace XYZEngine
{
	class Engine
	{
	public:
		Engine(const Engine& app) = delete;
		Engine& operator= (const Engine&) = delete;

		static Engine* Instance();

		void Run(Scene& scene);

		void SetPaused(bool newIsPaused);
		bool IsPaused() const;

	private:
		bool isPaused = false;

		Engine();
		~Engine() = default;
	};
}