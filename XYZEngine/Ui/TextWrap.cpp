#include "pch.h"
#include "TextWrap.h"

namespace XYZEngine
{
	namespace
	{
		constexpr sf::Uint32 SPACE = U' ';
		constexpr sf::Uint32 NEW_LINE = U'\n';

		// Кернинг спрашиваем ровно потому же, почему его спрашивает sf::Text:
		// счёт обязан идти тем же путём, что и рисование. Roboto через SFML 2.5.1
		// пар не отдаёт - они лежат в GPOS, а читается старая таблица kern.
		float AdvanceOf(sf::Uint32 letter, sf::Uint32 previous, const sf::Font& font, unsigned int characterSize)
		{
			float kerning = previous == 0 ? 0.f : font.getKerning(previous, letter, characterSize);

			return kerning + font.getGlyph(letter, characterSize, false).advance;
		}
	}

	float TextWidth(const sf::String& text, const sf::Font& font, unsigned int characterSize)
	{
		float width = 0.f;
		sf::Uint32 previous = 0;

		for (std::size_t index = 0; index < text.getSize(); index++)
		{
			width += AdvanceOf(text[index], previous, font, characterSize);
			previous = text[index];
		}

		return width;
	}

	std::vector<sf::String> WrapText(const sf::String& text, const sf::Font& font, unsigned int characterSize,
		float maxWidth)
	{
		std::vector<sf::String> lines;
		if (text.isEmpty())
		{
			return lines;
		}

		sf::String line;
		sf::String word;

		auto takeLine = [&lines, &line]()
		{
			lines.push_back(line);
			line.clear();
		};

		// Слово копится отдельно: пока оно не кончилось, неизвестно, влезет ли.
		auto takeWord = [&]()
		{
			if (word.isEmpty())
			{
				return;
			}

			sf::String candidate = line.isEmpty() ? word : line + " " + word;
			if (!line.isEmpty() && maxWidth > 0.f && TextWidth(candidate, font, characterSize) > maxWidth)
			{
				takeLine();
				candidate = word;
			}

			line = candidate;
			word.clear();
		};

		for (std::size_t index = 0; index < text.getSize(); index++)
		{
			sf::Uint32 letter = text[index];

			if (letter == NEW_LINE)
			{
				takeWord();
				takeLine();
				continue;
			}

			if (letter == SPACE)
			{
				takeWord();
				continue;
			}

			word += letter;
		}

		takeWord();
		if (!line.isEmpty())
		{
			takeLine();
		}

		return lines;
	}
}
