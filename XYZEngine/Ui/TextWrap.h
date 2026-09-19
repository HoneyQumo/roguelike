#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/System/String.hpp>
#include <vector>

namespace XYZEngine
{
	// Ширину считаем так же, как рисует SFML: по шагу глифа и кернингу.
	// Любой другой счёт разойдётся с картинкой, и перенос ляжет не там.
	float TextWidth(const sf::String& text, const sf::Font& font, unsigned int characterSize);

	/**
	*	Режет текст по ширине.
	*
	*	Перенос идёт по пробелам. Слово, которое само длиннее ширины, не рубится:
	*	лучше вылезти одним словом, чем показать половину слова на одной строке
	*	и половину на другой.
	*/
	std::vector<sf::String> WrapText(const sf::String& text, const sf::Font& font, unsigned int characterSize,
		float maxWidth);
}
