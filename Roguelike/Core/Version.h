#pragma once

#include <string>

namespace RoguelikeGame
{
    /**
    *	Версия игры по semver: major.minor.patch и необязательная метка предрелиза.
    *
    *	Это единственный источник правды. Отсюда её берут заголовок окна, лог и
    *	сборка релиза, поэтому числа лежат тремя простыми строками - их читает
    *	не только компилятор, но и скрипт выпуска.
    *
    *	Что меняем и когда:
    *	  major - несовместимое изменение, для игры это смена акта или формата сохранений;
    *	  minor - новая возможность или контент, обратно совместимо;
    *	  patch - только починки.
    *
    *	До 1.0.0 обещаний совместимости нет: игра ещё собирается.
    */
    constexpr int VERSION_MAJOR = 0;
    constexpr int VERSION_MINOR = 1;
    constexpr int VERSION_PATCH = 0;

    // Пусто у обычного выпуска. У предрелизного - alpha, beta, rc.1 и подобное.
    constexpr const char* VERSION_STAGE = "alpha";

    constexpr const char* GAME_NAME = "Roguelike by HoneyQumo";

    /**
    *	Собирает строку версии.
    *
    *	Вынесено в функцию от чисел, а не от констант, чтобы можно было проверить
    *	и предрелизный вид, и обычный, не подменяя константы.
    */
    inline std::string FormatVersion(int major, int minor, int patch, const char* stage)
    {
        std::string version = std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);

        if (stage != nullptr && *stage != '\0')
        {
            version += "-";
            version += stage;
        }

        return version;
    }

    inline std::string VersionString()
    {
        return FormatVersion(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_STAGE);
    }

    // Одно имя на заголовок окна и на лог: расходиться им незачем.
    inline std::string GameTitle()
    {
        return std::string(GAME_NAME) + " " + VersionString();
    }
}
