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

#include "MPQManager.h"

void MPQManager::load(std::vector<std::string> files)
{
	_logger.information("Loading list of MPQ's");
	for (auto it = files.rbegin(); it != files.rend(); ++it)
	{
		Path path(*it);
		MPQArchive* mpq = new MPQArchive(path);
		_archives.push_back(mpq);
		std::vector<std::string> filesList = mpq->getFilesList();
		int count = 0;
		for (auto itFiles = filesList.begin(); itFiles != filesList.end(); ++itFiles)
		{
			_logger.debug("Handling file: %s", *itFiles);
			if (_mapFiles.find(*itFiles) == _mapFiles.end())
			{ 
				// Add file in our map if it's not there already.
				_mapFiles.insert(make_pair(*itFiles, mpq));
				if (((std::string)*itFiles).rfind(PREFIX_DBC, 0) == 0)
				{
					_dbcs.push_back(*itFiles);
				}
				count++;
			}
		}
		if (!count)
		{
			_logger.information("No newer files loaded from %s", *it);
		}
		_logger.information("Loaded MPQ: %s", *it);
	}
	_logger.information("Total MPQ files loaded: %z", _mapFiles.size());
	_logger.information("Total loaded DBC: %z", _dbcs.size());
}

std::vector<std::string> MPQManager::getDBCList()
{
	return _dbcs;
}

bool MPQManager::extractFile(std::string file, std::string path)
{
	auto it = _mapFiles.find(file);
	if (it != _mapFiles.end())
	{
		return it->second->extractFile(file, path);
	}
	return false;
}

MPQFile* MPQManager::getFile(std::string file, Version version)
{
	auto it = _mapFiles.find(file);
	if (it != _mapFiles.end())
	{
		return it->second->getFile(file, version);
	}
	return NULL;
}