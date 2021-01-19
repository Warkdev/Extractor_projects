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

#include "WDT.h"
#include "Poco/BinaryReader.h"
#include "Poco/MemoryStream.h"

using Poco::BinaryReader;
using Poco::MemoryInputStream;

WDT::WDT(std::string name, unsigned char* data, long size)
{
	_name = name;
	_size = size;
	_data = data;
}

WDT::~WDT()
{
	delete _data;
}

bool WDT::parse()
{
	_logger.information("Parsing WDT file %s", _name);

	unsigned int offset = 0;

	_version = (Version*) (_data);

	std::string magic(_version->magic, 4);

	if (magic != HEADER_MVER)
	{
		_logger.error("Expected header REVM not found");
		return false;
	}

	if (_version->version != 18)
	{
		_logger.error("Expected file version 18, got %i", _version->version);
		return false;
	}

	offset = 8 + _version->size;

	_header = (Header*)(_data + offset);

	magic = std::string(_header->magic, 4);

	if (magic != HEADER_MPHD)
	{
		_logger.error("Expected header DHPM not found");
		return false;
	}

	offset += 8 + _header->size;

	_main = (Main*)(_data + offset);
	
	magic = std::string(_main->magic, 4);

	if (magic != HEADER_MAIN)
	{
		_logger.error("Expected header NIAM not found");
		return false;
	}

	offset += 8 + _main->size;

	_wmo = (MWMO*)(_data + offset);

	magic = std::string(_wmo->magic, 4);

	if (magic != HEADER_MWMO) 
	{
		_logger.error("Expected header OMWM not found");
		return false;
	}

	if (_header->flags & USE_GLOBAL_MAP_OBJ)
	{
		offset += 8 + _wmo->size;
		_objDef = (MODF*)(_data + offset);
		magic = std::string(_objDef->magic, 4);
		if (magic != HEADER_MODF)
		{
			_logger.error("Expected header FDOM not found");
			return false;
		}
	}

	return true;
}

bool WDT::hasADT(int x, int y)
{
	return _main->areaInfo[x][y].flags & HAS_ADT;
}