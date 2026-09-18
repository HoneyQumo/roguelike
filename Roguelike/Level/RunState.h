#pragma once

namespace RoguelikeGame
{
    enum class RunState
    {
        Playing,
        PlayerDied,
        GameOver,
        Victory
    };

    // Смерть ещё не конец: доигрывается анимация.
    constexpr bool IsRunOver(RunState state)
    {
        return state == RunState::GameOver || state == RunState::Victory;
    }

    // На экране итогов замирать нечему, а пауза закрывает собой и заголовок, и клавишу рестарта.
    constexpr bool MayPause(RunState state)
    {
        return !IsRunOver(state);
    }
}
