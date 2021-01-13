/**
 * MaNGOS is a full featured server for World of Warcraft, supporting
 * the following clients: 1.12.x, 2.4.3, 3.3.5a, 4.3.4a and 5.4.8
 *
 * Copyright (C) 2005-2021 MaNGOS <https://getmangos.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * World of Warcraft, and all World of Warcraft or Warcraft art, images,
 * and lore are copyrighted by Blizzard Entertainment, Inc.
 */

#include <regex>
#include "ClientHelper.h"
#include "Poco/FileStream.h"
#include "Poco/Exception.h"
#include "Poco/Path.h"
#include "Poco/NumberParser.h"

using Poco::FileInputStream;
using Poco::FileNotFoundException;
using Poco::Path;
using Poco::NumberParser;

namespace ClientHelper {
    namespace {
        FileInputStream* openWoWExe(std::string clientPath)
        {
            FileInputStream* fis;
            const std::string exeNames[] = { "WoW.exe", "Wow.exe", "wow.exe" ,"World of Warcraft.exe" };
            Path client(clientPath);

            /// loop through all possible file names
            for (int i = 0; i < (sizeof(exeNames) / sizeof(exeNames[0])); i++)
            {
                try {
                    Path path(client, exeNames[i]);
                    fis = new FileInputStream(path.toString());
                    return fis;
                }
                catch (FileNotFoundException fnfe) {

                }
            }

            return 0; ///< failed to locate WoW executable
        }
    }

    Version getBuildVersion(std::string path)
    {
        FileInputStream* fis;
        if (!(fis = openWoWExe(path)))
        {
            return Version::CLIENT_UNKNOWN;
        }

        std::string content;
        content.reserve(fis->tellg());
        char buffer[16384];
        std::streamsize read;
        bool found = false;
        char* find = "(build ";

        while ((fis->read(buffer, sizeof(buffer)), read = fis->gcount()) && !found)
        {
            content.append(buffer, read);
            if (strstr(buffer, find))
            {
                found = true;
            }
        }

        size_t pos = content.find(find);
        std::string temp = content.substr(pos, 25);
        size_t end = temp.find(")");

        fis->close();
        delete fis;

        std::string build = temp.substr(sizeof(find) - 1, end - (sizeof(find) - 1));
        switch (NumberParser::parse(build))
        {
            case 5875:
            case 6005:
            case 6141:
                return Version::CLIENT_CLASSIC;
            case 8606:
                return Version::CLIENT_TBC;
            case 12340:
                return Version::CLIENT_WOTLK;
            case 15595:
                return Version::CLIENT_CATA;
            case 18414:
                return Version::CLIENT_MOP;
            case 20574:
                return Version::CLIENT_WOD;
            case 21742:
                return Version::CLIENT_LEGION;
            case 31478:
                return Version::CLIENT_BFA;
            case 37130:
                return Version::CLIENT_SHADOWLANDS; // Temporary
            default:
                return Version::CLIENT_UNKNOWN;
        }
    }

    std::string ClientHelper::getVersionName(Version version) 
    {
        switch (version)
        {
            case Version::CLIENT_UNKNOWN:
                return "Unknown";
            case Version::CLIENT_CLASSIC:
                return "Classic";
            case Version::CLIENT_TBC:
                return "The Burning Crusade";
            case Version::CLIENT_WOTLK:
                return "Wrath of the Lich King";
            case Version::CLIENT_CATA:
                return "Cataclysm";
            case Version::CLIENT_MOP:
                return "Mysts of Pandaria";
            case Version::CLIENT_WOD:
                return "Warlords of Draenor";
            case Version::CLIENT_LEGION:
                return "Legion";
            case Version::CLIENT_BFA:
                return "Battle for Azeroth";
            case Version::CLIENT_SHADOWLANDS:
                return "Shadowlands";
        }
        return "Unknown";
    }
}