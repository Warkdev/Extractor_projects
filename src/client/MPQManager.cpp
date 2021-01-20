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
#include "Poco/String.h"

void MPQManager::load(std::vector<std::string> files)
{
	_logger.information("Loading list of MPQ's");
	std::string temp;
	std::string file;
	int digitCount;
	system("pause");
	for (auto it = files.rbegin(); it != files.rend(); ++it)
	{
		Path path(*it);
		MPQArchive* mpq = new MPQArchive(path);
		_archives.push_back(mpq);
		std::vector<std::string> filesList = mpq->getFilesList();
		int count = 0;
		for (auto itFiles = filesList.begin(); itFiles != filesList.end(); ++itFiles)
		{
			_logger.debug("Handling file: %s (%s)", *itFiles, *it);
			
			file.assign(Poco::toLower(*itFiles));
			if (!_mapFiles.count(file))
			{ 
				// Add file in our map if it's not there already.
				_mapFiles[file] = mpq;
				if (Poco::endsWith(file, mpq->EXT_DBC))
				{
					_dbcs.push_back(file);
				}
				if (Poco::endsWith(file, mpq->EXT_WMO))
				{
					// We've a WMO, let's check if that's a root one and add it to our list if that's the case.
					digitCount = 0;
					temp.assign((file).substr(0, (file).find_last_of(".")));
					reverse(temp.begin(), temp.end());
					for (int i = 0; i < 3; i++)
					{
						if (!isdigit(temp[i]))
						{
							i = 3;
							continue;
						}
						digitCount++;
					}

					if (digitCount < 3)
					{
						_wmos.push_back(file);
					}
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
	_logger.information("Total loaded WMO: %z", _wmos.size());
}

std::vector<std::string> MPQManager::getDBCList()
{
	return _dbcs;
}

std::vector<std::string> MPQManager::getWMOList()
{
	return _wmos;
}

bool MPQManager::extractFile(std::string file, std::string path)
{
	file = Poco::toLower(file);
	auto it = _mapFiles.find(file);
	if (it != _mapFiles.end())
	{
		return it->second->extractFile(file, path);
	}
	return false;
}

MPQFile* MPQManager::getFile(std::string file, Version version)
{	
	file = Poco::toLower(file);
	auto it = _mapFiles.find(file);
	if (it != _mapFiles.end())
	{
		return it->second->getFile(file, version);
	}
	return NULL;
}