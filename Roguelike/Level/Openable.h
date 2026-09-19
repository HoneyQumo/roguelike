#pragma once

#include <functional>
#include <string>
#include <vector>

namespace RoguelikeGame
{
    class DoorComponent;
    class HatchComponent;

    /**
    *	Рычаг не знает, что он открывает. Их связывает общий id, а как именно
    *	открыться - дело самой цели: люк уезжает, дверь распахивается.
    *
    *	Новый вид цели добавляется своей перегрузкой OpenableOf и больше нигде.
    */
    struct Openable
    {
        std::string id;
        std::function<void()> open;
    };

    Openable OpenableOf(HatchComponent* hatch);
    Openable OpenableOf(DoorComponent* door);

    template <typename Parts>
    void AddOpenables(std::vector<Openable>& targets, const Parts& parts)
    {
        for (auto* part : parts)
        {
            if (part != nullptr)
            {
                targets.push_back(OpenableOf(part));
            }
        }
    }
}
