#include "pch.h"
#include <string>
#include "TextUtils.h"

// Этот файл намеренно лежит без BOM: он проверяет, что компилятор читает
// исходник как UTF-8 и без подсказки в начале файла. С BOM тест был бы
// зелёным всегда и не значил бы ничего.
namespace
{
	unsigned char ByteAt(const std::string& text, std::size_t index)
	{
		return index < text.size() ? static_cast<unsigned char>(text[index]) : 0;
	}
}

// Без флага /utf-8 байты UTF-8 читаются как CP1251 и кодируются заново:
// на букву выходит вчетверо больше байт, и это не даёт ни ошибки сборки,
// ни предупреждения - только неверную строку в программе.
TEST(SourceEncodingTest, CyrillicIsRealUtf8)
{
	std::string box = u8"Ящик";

	EXPECT_EQ(box.size(), 8u) << "четыре буквы по два байта, а пришло " << box.size();
	EXPECT_EQ(ByteAt(box, 0), 208u);
	EXPECT_EQ(ByteAt(box, 1), 175u);
}

// Тем же путём текст идёт на экран: узкий литерал разбирается как UTF-8.
// Если байты в литерале не те, разбор даёт не те буквы и не ту длину.
TEST(SourceEncodingTest, TheTextGetsToTheScreenWhole)
{
	sf::String box = XYZEngine::FromUtf8(u8"Ящик");

	EXPECT_EQ(box.getSize(), 4u) << "в слове четыре буквы, а насчитано " << box.getSize();
	EXPECT_EQ(static_cast<unsigned>(box[0]), 0x042Fu) << "первая буква не Я";
}
