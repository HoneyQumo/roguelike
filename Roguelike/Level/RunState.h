#pragma once

namespace RoguelikeGame
{
    /**
    *	Состояние забега.
    *
    *	Жило приватным перечислением внутри сцены, и правила вокруг него проверить
    *	было нечем: сцена в тестовый проект не собирается. Отсюда и дефект - пауза
    *	на экране смерти подменяла его собой.
    */
    enum class RunState
    {
        Playing,
        PlayerDied,
        GameOver,
        Victory
    };

    // Забег кончился и ждёт клавишу. Смерть ещё не конец: доигрывается анимация.
    constexpr bool IsRunOver(RunState state)
    {
        return state == RunState::GameOver || state == RunState::Victory;
    }

    /**
    *	Пауза имеет смысл, только пока забег идёт.
    *
    *	На экране итогов замирать нечему, а вот вреда от паузы там достаточно:
    *	она рисуется поверх того же экрана сообщений и закрывает собой и заголовок,
    *	и клавишу рестарта.
    */
    constexpr bool MayPause(RunState state)
    {
        return !IsRunOver(state);
    }
}
