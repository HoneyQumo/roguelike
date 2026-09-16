#include "ActPlan.h"
#include "EnemyCatalog.h"
#include <LoggerRegistry.h>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace RoguelikeGame
{
    namespace
    {
        const std::string ACT_SECTION = "[act]";
        const std::string ROOMS_SECTION = "[rooms]";
        const std::string LIBRARY_SECTION = "[library]";
        const std::string WAVES_SECTION = "[waves]";
        const std::string PURSUIT_SECTION = "[pursuit]";
        const std::string UTF8_BOM = "\xEF\xBB\xBF";
        constexpr char COMMENT_SYMBOL = ';';

        std::string Trim(const std::string& line)
        {
            size_t first = line.find_first_not_of(" \t");
            if (first == std::string::npos)
            {
                return "";
            }

            size_t last = line.find_last_not_of(" \t\r");

            return line.substr(first, last - first + 1);
        }

        void ReadActLine(const std::string& line, int lineNumber, LevelInfo& info)
        {
            std::istringstream stream(line);
            std::string key;
            stream >> key;

            if (key == "title")
            {
                std::string rest;
                std::getline(stream, rest);
                info.title = Trim(rest);
                return;
            }

            if (key == "next")
            {
                stream >> info.nextLevelId;
                return;
            }

            if (key == "tileset")
            {
                stream >> info.tileset;
                return;
            }

            if (key == "music")
            {
                stream >> info.music;
                return;
            }

            if (key == "ambient")
            {
                stream >> info.ambient;
                return;
            }

            if (key == "boss")
            {
                if (!(stream >> info.boss.bossId))
                {
                    LOG_ERROR("Act boss has no id, line " + std::to_string(lineNumber));
                    throw std::runtime_error("Act boss has no id");
                }

                if (!(stream >> info.boss.healthScale))
                {
                    info.boss.healthScale = 1.f;
                }

                if (!(stream >> info.boss.damageScale))
                {
                    info.boss.damageScale = 1.f;
                }

                stream >> info.boss.drop;

                return;
            }

            LOG_WARN("Unknown act field at line " + std::to_string(lineNumber) + ": " + key);
        }

        void ReadLibraryLine(const std::string& line, int lineNumber, std::map<std::string, std::string>& library)
        {
            std::istringstream stream(line);

            std::string id;
            std::string path;
            if (!(stream >> id >> path))
            {
                LOG_ERROR("Act library needs a room id and a file, line " + std::to_string(lineNumber));
                throw std::runtime_error("Act library needs a room id and a file");
            }

            library[id] = path;
        }

        /**
        *	Строка волны акта: wave <пауза> <символ><сколько> ...
        *	Символ здесь - из каталога врагов, а не из легенды комнаты: у акта своей легенды нет.
        */

        /**
        *	Строки погони: keep / grow / respawn задают напор,
        *	from <доля пути> <символ><вес> ... - кто выбегает на этом отрезке.
        */
        void ReadPursuitLine(const std::string& line, int lineNumber, PursuitSpec& pursuit)
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

        void ReadActWaveLine(const std::string& line, int lineNumber, std::vector<WaveSpec>& waves)
        {
            std::istringstream stream(line);
            std::string keyword;
            stream >> keyword;

            if (keyword != "wave")
            {
                LOG_ERROR("Act wave line " + std::to_string(lineNumber) + " does not start with wave");
                throw std::runtime_error("Unknown act wave line");
            }

            WaveSpec wave;
            if (!(stream >> wave.delay) || wave.delay < 0.f)
            {
                LOG_ERROR("Act wave line " + std::to_string(lineNumber) + " has no delay");
                throw std::runtime_error("Act wave line has no delay");
            }

            std::string group;
            while (stream >> group)
            {
                const EnemyDefinition* enemy = group.empty() ? nullptr : FindEnemyBySymbol(group.front());
                if (enemy == nullptr || group.size() < 2)
                {
                    LOG_ERROR("Act wave line " + std::to_string(lineNumber) + " has a bad group: " + group);
                    throw std::runtime_error("Bad act wave group");
                }

                int count = 0;
                try
                {
                    count = std::stoi(group.substr(1));
                }
                catch (const std::exception&)
                {
                    count = 0;
                }

                if (count <= 0)
                {
                    LOG_ERROR("Act wave line " + std::to_string(lineNumber) + " asks for a non-positive count: " + group);
                    throw std::runtime_error("Act wave count must be positive");
                }

                wave.entries.push_back({enemy->tile, count});
            }

            if (wave.entries.empty())
            {
                LOG_ERROR("Act wave line " + std::to_string(lineNumber) + " is empty");
                throw std::runtime_error("Act wave has no enemies");
            }

            waves.push_back(std::move(wave));
        }

        void ReadRoomLine(const std::string& line, int lineNumber, std::vector<RoomPlacement>& rooms)
        {
            std::istringstream stream(line);

            RoomPlacement room;
            if (!(stream >> room.roomId >> room.column >> room.row))
            {
                LOG_ERROR("Act room needs id, column and row, line " + std::to_string(lineNumber));
                throw std::runtime_error("Act room needs id, column and row");
            }

            std::string option;
            while (stream >> option)
            {
                if (option == "turn")
                {
                    if (!(stream >> room.quarters))
                    {
                        LOG_ERROR("Act room turn needs a number of quarters, line " + std::to_string(lineNumber));
                        throw std::runtime_error("Act room turn needs a number of quarters");
                    }

                    continue;
                }

                if (option == "mirror")
                {
                    room.isMirrored = true;
                    continue;
                }

                LOG_WARN("Unknown act room option at line " + std::to_string(lineNumber) + ": " + option);
            }

            rooms.push_back(room);
        }
    }

    ActPlan ActLoader::Load(const std::string& filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            LOG_ERROR("Can't open act file: " + filePath);
            throw std::runtime_error("Act file is not available: " + filePath);
        }

        return Parse(file, filePath);
    }

    ActPlan ActLoader::Parse(std::istream& input, const std::string& sourceName)
    {
        ActPlan plan;
        bool isActSection = false;
        bool isRoomsSection = false;
        bool isLibrarySection = false;
        bool isWavesSection = false;
        bool isPursuitSection = false;

        std::string line;
        int lineNumber = 0;
        while (std::getline(input, line))
        {
            lineNumber++;

            if (lineNumber == 1 && line.compare(0, UTF8_BOM.size(), UTF8_BOM) == 0)
            {
                line.erase(0, UTF8_BOM.size());
            }

            line = Trim(line);
            if (line.empty() || line.front() == COMMENT_SYMBOL)
            {
                continue;
            }

            if (line == ACT_SECTION)
            {
                isActSection = true;
                isRoomsSection = false;
                isLibrarySection = false;
                isWavesSection = false;
                isPursuitSection = false;
                continue;
            }

            if (line == LIBRARY_SECTION)
            {
                isActSection = false;
                isRoomsSection = false;
                isLibrarySection = true;
                isWavesSection = false;
                isPursuitSection = false;
                continue;
            }

            if (line == ROOMS_SECTION)
            {
                isActSection = false;
                isRoomsSection = true;
                isLibrarySection = false;
                isWavesSection = false;
                isPursuitSection = false;
                continue;
            }

            if (line == WAVES_SECTION)
            {
                isActSection = false;
                isRoomsSection = false;
                isLibrarySection = false;
                isWavesSection = true;
                isPursuitSection = false;
                continue;
            }

            if (line == PURSUIT_SECTION)
            {
                isActSection = false;
                isRoomsSection = false;
                isLibrarySection = false;
                isWavesSection = false;
                isPursuitSection = true;
                continue;
            }

            if (isActSection)
            {
                ReadActLine(line, lineNumber, plan.info);
                continue;
            }

            if (isLibrarySection)
            {
                ReadLibraryLine(line, lineNumber, plan.library);
                continue;
            }

            if (isWavesSection)
            {
                ReadActWaveLine(line, lineNumber, plan.waves);
                continue;
            }

            if (isPursuitSection)
            {
                ReadPursuitLine(line, lineNumber, plan.pursuit);
                continue;
            }

            if (isRoomsSection)
            {
                ReadRoomLine(line, lineNumber, plan.rooms);
            }
        }

        if (plan.rooms.empty())
        {
            LOG_ERROR("Act has no rooms: " + sourceName);
            throw std::runtime_error("Act has no rooms: " + sourceName);
        }

        LOG_INFO("Act loaded: " + sourceName + ", rooms " + std::to_string(plan.rooms.size()));

        return plan;
    }
}
