#pragma once

#include "Component.h"

namespace XYZEngine
{
	/**
	*	Всё, что звучит, независимо от того, чем оно звучит.
	*
	*	Короткий звук и потоковая музыка сидят на разных источниках SFML, а значит
	*	и на разных шаблонных компонентах - общего типа у них не было, и выключить
	*	звук миром целиком было нечем. Каждый, кому это требовалось, перебирал
	*	знакомые ему компоненты руками и забывал остальные.
	*/
	class SoundSource : public Component
	{
	public:
		SoundSource(GameObject* gameObject) : Component(gameObject) {}

		virtual void Pause() = 0;
		virtual void Resume() = 0;
		virtual void Stop() = 0;
		virtual bool IsPlaying() const = 0;
	};
}
