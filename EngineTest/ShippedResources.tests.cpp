#include "pch.h"
#include "GameSettings.h"
#include "ProjectFiles.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <string>

using namespace RoguelikeGame;

namespace
{
	// Плоские файлы: всё, что лежит в подпапках Audio, грузится по ключу и живёт своей жизнью.
	const std::set<std::string> DECLARED_AUDIO = {
		CAR_ENGINE_SOUND_FILE,
		CAR_SKID_SOUND_FILE,
		DOOR_OPEN_SOUND_FILE,
		HATCH_SOUND_FILE,
		HURT_SOUND_FILE,
		LEVER_SOUND_FILE,
		MAIN_THEME_FILE,
	};

	std::string PathInFolder(const std::filesystem::directory_entry& entry, const std::string& folder)
	{
		return folder + entry.path().filename().string();
	}

	// Заголовок разбираем руками: sf::SoundBuffer тянет за собой звуковое
	// устройство, а на раннере его нет - тест падал бы не по делу.
	int ChannelsOf(const std::filesystem::path& file)
	{
		std::ifstream input(file, std::ios::binary);
		char header[12] = {};

		input.read(header, sizeof(header));
		if (!input || std::string(header, 4) != "RIFF" || std::string(header + 8, 4) != "WAVE")
		{
			return -1;
		}

		while (input)
		{
			char id[4] = {};
			unsigned int size = 0;

			input.read(id, sizeof(id));
			input.read(reinterpret_cast<char*>(&size), sizeof(size));
			if (!input)
			{
				break;
			}

			if (std::string(id, sizeof(id)) == "fmt ")
			{
				unsigned short format = 0;
				unsigned short channels = 0;

				input.read(reinterpret_cast<char*>(&format), sizeof(format));
				input.read(reinterpret_cast<char*>(&channels), sizeof(channels));

				return input ? channels : -1;
			}

			input.seekg(size + (size & 1), std::ios::cur);
		}

		return -1;
	}
}

class ShippedResourcesTest : public ProjectFiles::Test
{
};

TEST_F(ShippedResourcesTest, EveryDeclaredFileIsOnDisk)
{
	ASSERT_TRUE(isFound) << previous.string();

	EXPECT_TRUE(std::filesystem::exists(HUD_FONT_FILE)) << HUD_FONT_FILE;

	for (const auto& file : DECLARED_AUDIO)
	{
		EXPECT_TRUE(std::filesystem::exists(file)) << file;
	}
}

TEST_F(ShippedResourcesTest, NoStrayAudioLiesNextToTheDeclaredOne)
{
	ASSERT_TRUE(isFound) << previous.string();

	for (const auto& entry : std::filesystem::directory_iterator(AUDIO_PATH))
	{
		if (entry.is_directory())
		{
			continue;
		}

		std::string path = PathInFolder(entry, AUDIO_PATH);
		EXPECT_EQ(DECLARED_AUDIO.count(path), 1u)
			<< path << " не упоминается в коде - подключить или удалить";
	}
}

TEST_F(ShippedResourcesTest, NoStrayFontLiesNextToTheDeclaredOne)
{
	ASSERT_TRUE(isFound) << previous.string();

	for (const auto& entry : std::filesystem::directory_iterator(FONTS_PATH))
	{
		std::string path = PathInFolder(entry, FONTS_PATH);
		EXPECT_EQ(path, HUD_FONT_FILE)
			<< path << " не упоминается в коде - подключить или удалить";
	}
}

// Стерео-буфер SFML не пространствляет: позиция, затухание и панорама к нему
// не применяются. Один такой файл тихо выключает звуку место в мире.
// Музыка под правило не идёт - у фоновой дорожки места в мире и нет.
TEST_F(ShippedResourcesTest, EverySoundIsMono)
{
	ASSERT_TRUE(isFound) << previous.string();

	int checked = 0;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(AUDIO_PATH))
	{
		if (!entry.is_regular_file() || entry.path().extension() != ".wav")
		{
			continue;
		}

		EXPECT_EQ(ChannelsOf(entry.path()), 1)
			<< entry.path().filename().string() << " - стерео не спространствить, свести в моно";
		checked++;
	}

	EXPECT_GT(checked, 30) << "проверять стало нечего - папка со звуком не найдена?";
}
