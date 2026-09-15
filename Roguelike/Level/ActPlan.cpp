#include "ActPlan.h"
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
                continue;
            }

            if (line == LIBRARY_SECTION)
            {
                isActSection = false;
                isRoomsSection = false;
                isLibrarySection = true;
                continue;
            }

            if (line == ROOMS_SECTION)
            {
                isActSection = false;
                isRoomsSection = true;
                isLibrarySection = false;
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
