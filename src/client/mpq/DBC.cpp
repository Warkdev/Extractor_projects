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

#include "DBC.h"
#include "Poco/BinaryReader.h"

using Poco::BinaryReader;

DBC::DBC(std::string name, unsigned char* data, long size)
{
	_name = name;
	_size = size;
	_data = data;
	_offsetString = 0;
}

DBC::~DBC()
{
	delete _data;
}

bool DBC::parse()
{
	_logger.information("Parsing DBC file %s", _name);

	_header = reinterpret_cast<Header*>(_data);

	std::string magic(_header->magic, 4);

	if (magic != HEADER_WDBC)
	{
		_logger.error("The provided file is not a DBC file");
		return false;
	}

	_logger.debug("DBC record_count: %u", _header->recordCount);
	_logger.debug("DBC field_count: %u", _header->fieldCount);
	_logger.debug("DBC record_size: %u", _header->recordSize);
	_logger.debug("DBC string_block_size: %u", _header->stringBlockSize);

	if (_header->fieldCount * 4 != _header->recordSize)
	{
		_logger.error("Field count and record size in DBC %s do not match.", _name);
		return false;
	}

	_offsetString = 20 + (_header->recordCount * static_cast<unsigned __int64>(_header->recordSize));

	return true;
}

int DBC::getRecordCount()
{
	return _header->recordCount;
}

unsigned char* DBC::getRecord(int idx)
{
	if (idx < 0 || idx > _header->recordCount)
	{
		_logger.error("Trying to retrieve a record which does not exist");
		return NULL;
	}

	return &_data[20 + (idx * _header->recordSize)];
}

std::string DBC::getString(int offset)
{
	std::string str;
	unsigned int pos = _offsetString + offset;
	bool complete = false;

	// Then we read what's on there until we find a '0' terminating the string.
	while (!complete)
	{
		if (_data[pos] == '\0')
		{
			complete = true;
		} else {
			str.push_back(_data[pos]);
		}
		pos++;
	}
	return str;
}