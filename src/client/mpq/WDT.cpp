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

WDT::WDT(std::string name, char* data, long size)
{
	_name = name;
	_size = size;
	MemoryInputStream* stream = new MemoryInputStream(data, _size);
	_buffer = new BinaryReader(*stream);
	_main.areas = new AreaInfo[static_cast<unsigned __int64>(MAP_TILE_SIZE) * MAP_TILE_SIZE];
}

WDT::~WDT()
{
	delete _main.areas;
	delete _buffer;
}

bool WDT::parse()
{
	_logger.information("Parsing WDT file %s", _name);

	_buffer->readRaw(4, _version.magic);

	if (_version.magic != HEADER_MVER)
	{
		_logger.error("Expected header REVM not found");
		return false;
	}

	*_buffer >> _version.size;
	*_buffer >> _version.version;

	if (_version.version != 18)
	{
		_logger.error("Expected file version 18, got %i", _version.version);
		return false;
	}

	_buffer->readRaw(4, _header.magic);

	if (_header.magic != HEADER_MPHD)
	{
		_logger.error("Expected header DHPM not found");
		return false;
	}

	*_buffer >> _header.size;
	*_buffer >> _header.flags;
	// We skip the rest of the buffer section, should be always 0.
	_buffer->stream().seekg(_header.size - sizeof(_header.flags), std::ios::cur);

	_buffer->readRaw(4, _main.magic);

	if (_main.magic != HEADER_MAIN)
	{
		_logger.error("Expected header NIAM not found");
		return false;
	}

	*_buffer >> _main.size;
	for (int i = 0; i < MAP_TILE_SIZE * MAP_TILE_SIZE; i++)
	{
		*_buffer >> _main.areas[i].flags;
		*_buffer >> _main.areas[i].asyncId;
	}

	_buffer->readRaw(4, _wmo.magic);

	if (_wmo.magic != HEADER_MWMO) 
	{
		_logger.error("Expected header OMWM not found");
		return false;
	}

	*_buffer >> _wmo.size;

	if (_header.flags & USE_GLOBAL_MAP_OBJ)
	{
		_wmo.name = readString();
		_buffer->readRaw(4, _objDef.magic);
		if (_objDef.magic != HEADER_MODF)
		{
			_logger.error("Expected header FDOM not found");
			return false;
		}
		*_buffer >> _objDef.size;
		*_buffer >> _objDef.placement.mwidEntry;
		*_buffer >> _objDef.placement.uniqueId;
		*_buffer >> _objDef.placement.position[0]; // Position X
		*_buffer >> _objDef.placement.position[1]; // Position Y
		*_buffer >> _objDef.placement.position[2]; // Position Z
		*_buffer >> _objDef.placement.orientation[0]; // Orientation X
		*_buffer >> _objDef.placement.orientation[1]; // Orientation Y
		*_buffer >> _objDef.placement.orientation[2]; // Orientation Z
		*_buffer >> _objDef.placement.upperExtents[0];
		*_buffer >> _objDef.placement.upperExtents[1];
		*_buffer >> _objDef.placement.upperExtents[2];
		*_buffer >> _objDef.placement.lowerExtents[0];
		*_buffer >> _objDef.placement.lowerExtents[1];
		*_buffer >> _objDef.placement.lowerExtents[2];
		*_buffer >> _objDef.placement.flags;
		*_buffer >> _objDef.placement.doodadSet;
		*_buffer >> _objDef.placement.nameSet;
		*_buffer >> _objDef.placement.padding;
	}

	return true;
}

bool WDT::hasADT(int x, int y)
{
	return _main.areas[x * MAP_TILE_SIZE + y].flags & HAS_ADT;
}