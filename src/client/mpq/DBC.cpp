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
#include "Poco/MemoryStream.h"

using Poco::BinaryReader;
using Poco::MemoryInputStream;

DBC::DBC(std::string name, char* data, long size)
{
	_name = name;
	_size = size;
	MemoryInputStream* stream = new MemoryInputStream(data, _size);
	_data = data;
	_buffer = new BinaryReader(*stream);
	_offsetString = 0;
}

DBC::~DBC()
{
	for (auto it = _records.begin(); it < _records.end(); ++it)
	{
		delete *it;
	}
	delete _data;
	delete _buffer;
}

bool DBC::parse()
{
	_logger.information("Parsing DBC file %s", _name);

	_buffer->readRaw(4, _header.magic);

	if (_header.magic != HEADER_WDBC)
	{
		_logger.error("The provided file is not a DBC file");
		return false;
	}
	
	*_buffer >> _header.recordCount >> _header.fieldCount >> _header.recordSize >> _header.stringBlockSize;

	_logger.debug("DBC record_count: %u", _header.recordCount);
	_logger.debug("DBC field_count: %u", _header.fieldCount);
	_logger.debug("DBC record_size: %u", _header.recordSize);
	_logger.debug("DBC string_block_size: %u", _header.stringBlockSize);

	if (_header.fieldCount * 4 != _header.recordSize)
	{
		_logger.error("Field count and record size in DBC %s do not match.", _name);
		return false;
	}

	_offsetString = 20 + (_header.recordCount * static_cast<unsigned __int64>(_header.recordSize));

	return true;
}

int DBC::getRecordCount()
{
	return _header.recordCount;
}

BinaryReader* DBC::getRecord(int idx)
{
	if (idx < 0 || idx > _header.recordCount)
	{
		_logger.error("Trying to retrieve a record which does not exist");
		return NULL;
	}

	// Let's move to the beginning of the record.
	char* record = new char[_header.recordSize];
	_records.push_back(record);
	_buffer->stream().seekg(20 + (idx * static_cast<unsigned __int64>(_header.recordSize)), std::ios::beg);

	_buffer->readRaw(record, _header.recordSize);
	MemoryInputStream* stream = new MemoryInputStream(record, _header.recordSize);
	return new BinaryReader(*stream);
}

std::string DBC::getString(int offset)
{
	std::string str;
	char c;
	bool complete = false;

	// Let's move to the String Table.
	_buffer->stream().seekg(_offsetString + static_cast<unsigned __int64>(offset), std::ios::beg);

	// Then we read what's on there until we find a '0' terminating the string.
	while (!complete && (_buffer->stream().read(&c,1)))
	{
		if (c == '\0')
		{
			complete = true;
		} else {
			str.append(&c, 1);
		}
	}
	return str;
}