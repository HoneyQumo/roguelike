#include "pch.h"
#include "GameSettings.h"
#include "ProjectFiles.h"
#include <filesystem>
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
