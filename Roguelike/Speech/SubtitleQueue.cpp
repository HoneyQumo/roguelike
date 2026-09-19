#include "SubtitleQueue.h"
#include <algorithm>

namespace RoguelikeGame
{
	float SubtitleTimeFor(const std::string& text)
	{
		// Считаем по буквам, а не по байтам: в кириллице на букву два байта,
		// и по байтам русская фраза висела бы вдвое дольше английской.
		std::size_t letters = 0;
		for (char symbol : text)
		{
			letters += (static_cast<unsigned char>(symbol) & 0xC0) != 0x80 ? 1 : 0;
		}

		return SUBTITLE_LEAST_TIME + SUBTITLE_TIME_PER_LETTER * static_cast<float>(letters);
	}

	void SubtitleQueue::Say(const std::string& speaker, const std::string& text, float seconds)
	{
		if (text.empty() || seconds <= 0.f)
		{
			return;
		}

		if (lines.size() >= SUBTITLE_MAX_LINES)
		{
			lines.erase(lines.begin());
		}

		lines.push_back({speaker, text, seconds});
	}

	void SubtitleQueue::Update(float deltaTime)
	{
		if (deltaTime <= 0.f)
		{
			return;
		}

		for (Subtitle& line : lines)
		{
			line.timeLeft -= deltaTime;
		}

		lines.erase(std::remove_if(lines.begin(), lines.end(),
			[](const Subtitle& line) { return line.timeLeft <= 0.f; }), lines.end());
	}

	void SubtitleQueue::Clear()
	{
		lines.clear();
	}

	const std::vector<Subtitle>& SubtitleQueue::GetLines() const
	{
		return lines;
	}
}
