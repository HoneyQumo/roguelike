#pragma once

#include <istream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <LoggerRegistry.h>
#include "EnemyCatalog.h"
#include "LevelData.h"

namespace RoguelikeGame
{
    /**
    *	Строки погони: keep / grow / respawn задают напор,
    *	from <доля пути> <символ><вес> ... - кто выбегает на этом отрезке,
    *	style - дерутся ли преследователи с оглядкой.
    */
    inline void ReadPursuitLine(const std::string& line, int lineNumber, PursuitSpec& pursuit)
    {
        std::istringstream stream(line);
        std::string keyword;
        stream >> keyword;

        if (keyword == "keep" || keyword == "grow")
        {
            int value = 0;
            if (!(stream >> value) || value <= 0)
            {
                LOG_ERROR("Pursuit line " + std::to_string(lineNumber) + " needs a positive count");
                throw std::runtime_error("Bad pursuit count");
            }

            (keyword == "keep" ? pursuit.keep : pursuit.grow) = value;
            return;
        }

        if (keyword == "style")
        {
            ReadFightStyle(stream, lineNumber, pursuit.style);
            return;
        }

        if (keyword == "respawn")
        {
            float seconds = 0.f;
            if (!(stream >> seconds) || seconds < 0.f)
            {
                LOG_ERROR("Pursuit line " + std::to_string(lineNumber) + " needs a respawn time");
                throw std::runtime_error("Bad pursuit respawn");
            }

            pursuit.respawn = seconds;
            return;
        }

        if (keyword != "from")
        {
            LOG_ERROR("Pursuit line " + std::to_string(lineNumber) + " starts with an unknown word: " + keyword);
            throw std::runtime_error("Unknown pursuit line");
        }

        PursuitEchelon echelon;
        if (!(stream >> echelon.fromPart) || echelon.fromPart < 0.f || echelon.fromPart > 1.f)
        {
            LOG_ERROR("Pursuit line " + std::to_string(lineNumber) + " needs a part of the way between 0 and 1");
            throw std::runtime_error("Bad pursuit part");
        }

        std::string group;
        while (stream >> group)
        {
            const EnemyDefinition* enemy = group.empty() ? nullptr : FindEnemyBySymbol(group.front());
            if (enemy == nullptr || group.size() < 2)
            {
                LOG_ERROR("Pursuit line " + std::to_string(lineNumber) + " has a bad group: " + group);
                throw std::runtime_error("Bad pursuit group");
            }

            int weight = 0;
            try
            {
                weight = std::stoi(group.substr(1));
            }
            catch (const std::exception&)
            {
                weight = 0;
            }

            if (weight <= 0)
            {
                LOG_ERROR("Pursuit line " + std::to_string(lineNumber) + " asks for a non-positive weight: " + group);
                throw std::runtime_error("Pursuit weight must be positive");
            }

            echelon.entries.push_back({enemy->tile, weight});
        }

        if (echelon.entries.empty())
        {
            LOG_ERROR("Pursuit line " + std::to_string(lineNumber) + " has nobody to send");
            throw std::runtime_error("Pursuit echelon is empty");
        }

        // Отрезки идут по возрастанию: иначе выбор по прогрессу читал бы не ту строку.
        if (!pursuit.echelons.empty() && echelon.fromPart <= pursuit.echelons.back().fromPart)
        {
            LOG_ERROR("Pursuit line " + std::to_string(lineNumber) + " goes back along the way");
            throw std::runtime_error("Pursuit echelons must grow");
        }

        pursuit.echelons.push_back(std::move(echelon));
    }
}
