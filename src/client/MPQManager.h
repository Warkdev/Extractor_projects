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
#ifndef MPQMANAGER_H
#define MPQMANAGER_H

#include <string>
#include <map>
#include "Poco/Logger.h"
#include "MPQArchive.h"

using Poco::Logger;

class MPQManager
{
public:
	MPQManager()
	{
	}

	~MPQManager()
	{
		for (auto it = _mapFiles.begin(); it != _mapFiles.end(); ++it)
		{
			delete it->second;
		}
	}

	void load(std::vector<std::string> files);

	std::vector<std::string> getDBCList();

	bool extractFile(std::string file, std::string path);

protected:
	Logger& _logger = Logger::get("Extractor");
	const std::string PREFIX_DBC = "DBFilesClient\\";
	std::map<std::string, MPQArchive*> _mapFiles; // Maintain a list of files contained in every MPQ and a pointer into which MPQArchive holds it.
	std::vector<std::string> _dbcs;
};

#endif // !EXTRACTOR_H