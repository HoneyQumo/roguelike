#pragma once

#include <string>
#include <vector>

namespace RoguelikeGame
{
	// Сколько строк висит разом: враг и напарник могут говорить одновременно,
	// но третий голос уже не читается.
	constexpr std::size_t SUBTITLE_MAX_LINES = 3;

	// Время показа от длины текста: короткое «Стоять!» и длинная фраза не
	// могут висеть одинаково. Ниже - сколько держать самое короткое и
	// сколько добавлять на букву.
	constexpr float SUBTITLE_LEAST_TIME = 1.2f;
	constexpr float SUBTITLE_TIME_PER_LETTER = 0.055f;

	float SubtitleTimeFor(const std::string& text);

	struct Subtitle
	{
		std::string speaker;
		std::string text;
		float timeLeft = 0.f;
	};

	/**
	*	Очередь субтитров: правила показа отдельно от рисования.
	*
	*	Строки живут параллельно и гаснут каждая по своему времени. Когда мест
	*	не хватает, уходит самая старая - в разговоре важнее последнее сказанное.
	*/
	class SubtitleQueue
	{
	public:
		void Say(const std::string& speaker, const std::string& text, float seconds);
		void Update(float deltaTime);
		void Clear();

		const std::vector<Subtitle>& GetLines() const;

	private:
		std::vector<Subtitle> lines;
	};
}
