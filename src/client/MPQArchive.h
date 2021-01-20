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

#include "Poco/Path.h"
#include "Poco/Logger.h"
#include "mpq/MPQFile.h"
#include "Version.h"

// Win 32 specific headers
#ifdef _WIN32
#include <Windows.h>
#endif

// Linux specific headers
#ifdef __linux__
typedef void* HANDLE;
#endif

using Poco::Path;
using Poco::Logger;

/**
* @file MPQArchive.cc
* 
* @brief Represent a MPQ archive altogether with its Path and Handler
* 
* @author Warkdev
*/
class MPQArchive
{
	public:
		const std::string EXT_DBC = ".dbc";
		const std::string EXT_WDT = ".wdt";
		const std::string EXT_ADT = ".adt";
		const std::string EXT_M2 = ".m2";
		const std::string EXT_MDX = ".mdx";
		const std::string EXT_WMO = ".wmo";

		MPQArchive(Path path) : _path(path)
		{
			open();
		}

		~MPQArchive()
		{
			close();
		}

		std::string getName();
		std::vector<std::string> getFilesList();
		MPQFile* getFile(std::string file, Version version);
		bool extractFile(std::string file, std::string path);

	private:
		Logger& _logger = Logger::get("Extractor");
		HANDLE _mpqHandle;
		Path _path;

		void open();
		void close();
		bool isAllowedExt(std::string file);
		bool isRootWMO(std::string file);
};