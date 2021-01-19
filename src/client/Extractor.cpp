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

#include "Extractor.h"
#include "Poco/File.h"
#include "mpq/DBC.h"

using Poco::File;

void Extractor::init(std::string clientPath)
{
	//_logger.information("You should not see this message");
}

void Extractor::loadMPQs()
{
	_mpqManager->load(_listMPQ);
}

void Extractor::exportDBC(std::string outputPath)
{
	Path path(outputPath + PATH_DBC);
	File file(path);
	file.createDirectories();
	std::vector<std::string> dbcList = _mpqManager->getDBCList();
	int count = 0;

	for (auto it = dbcList.begin(); it != dbcList.end(); ++it)
	{
		Path temp(*it);
		Path dbc(path.toString() + "/" + temp.getFileName());
		if (!_mpqManager->extractFile(*it, dbc.toString()))
		{
			_logger.error("Error extracting DBC file: %s", *it);
		}
		else {
			count++;
		}
	}
	_logger.information("DBC Extraction Summary: %i / %z", count, dbcList.size());
}

void Extractor::exportMaps(std::string outputPath)
{
	_logger.information("You should not end up here");
}

void Extractor::readMaps()
{
	DBC* dbc = (DBC*)_mpqManager->getFile(DBC_MAPS, _version);

	if (!dbc)
	{
		_logger.error("DBC file not found");
		delete dbc;
		return;
	}

	if (!dbc->parse())
	{
		_logger.error("Error while parsing the DBC file");
		delete dbc;
		return;
	}

	unsigned int id;
	unsigned int stringId;
	std::string name;

	_maps.clear();

	for (int i = 0; i < dbc->getRecordCount(); i++)
	{
		_logger.debug("Retrieving record %i", i);
		unsigned char* record = dbc->getRecord(i);
		id = *(unsigned int*) record;
		stringId = *(unsigned int*)(record + 4);
		name = dbc->getString(stringId);

		_logger.debug("Map ID: %u", id);
		_logger.debug("Map Name: %s", name);

		_maps.insert(std::make_pair(id, name));
	}

	delete dbc;
}

void Extractor::readAreaTable()
{
	DBC* dbc = (DBC*)_mpqManager->getFile(DBC_AREATABLE, _version);

	if (!dbc)
	{
		_logger.error("DBC file not found");
		delete dbc;
		return;
	}

	if (!dbc->parse())
	{
		_logger.error("Error while parsing the DBC file");
		delete dbc;
		return;
	}

	unsigned int id;
	unsigned int flags;

	_areas.clear();

	for (int i = 0; i < dbc->getRecordCount(); i++)
	{
		_logger.debug("Retrieving record %i", i);
		unsigned char* record = dbc->getRecord(i);
		id = *(int*)record;
		flags = *(int*)(record + 3 * 4);

		_logger.debug("Area ID: %u", id);
		_logger.debug("Area flags: %u", flags);

		_areas.insert(std::make_pair(id, flags));
	}

	delete dbc;
}

void Extractor::readLiquidType()
{
	DBC* dbc = (DBC*)_mpqManager->getFile(DBC_LIQUIDTYPE, _version);

	if (!dbc)
	{
		_logger.error("DBC file not found");
		delete dbc;
		return;
	}

	if (!dbc->parse())
	{
		_logger.error("Error while parsing the DBC file");
		delete dbc;
		return;
	}

	unsigned int id;
	unsigned int flags;

	_liquids.clear();

	for (int i = 0; i < dbc->getRecordCount(); i++)
	{
		_logger.debug("Retrieving record %i", i);
		unsigned char* record = dbc->getRecord(i);
		id = *(int*)record;
		flags = *(int*)(record + 2 * 4);

		_logger.debug("Liquid Type ID: %u", id);
		_logger.debug("Liquid flags: %u", flags);

		_liquids.insert(std::make_pair(id, flags));
	}

	delete dbc;
}