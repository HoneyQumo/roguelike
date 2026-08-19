#pragma once

#include <memory>
#include "Scene.h"
#include "Music.h"
#include "LevelBuilder.h"

using namespace XYZEngine;

namespace RoguelikeGame
{
	class DeveloperLevel : public Scene
	{
	public:
		void Start() override;
		void Restart() override;
		void Stop() override;
	private:
		LevelBuilder levelBuilder;
		std::unique_ptr<Music> music;
	};
}
