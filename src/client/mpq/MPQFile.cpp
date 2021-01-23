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

#include "MPQFile.h"
#include "Poco/String.h"

MPQFile::MPQFile()
{
	_name = "";
}

MPQFile::MPQFile(std::string name, unsigned char* data, long size) : _name(name),  _size(size)
{
	_data = data;
	_size = size;
}

MPQFile::~MPQFile()
{
}

bool MPQFile::parse()
{
	// This method is virtual and shall never parse anything
	_logger.information("Parsing MPQFile");
	return false;
}

bool MPQFile::checkHeader(char magic[4], std::string expected)
{
	std::string temp(magic, 4);

	if (temp != expected)
	{
		_logger.error("Expected header %s not found", expected);
		return false;
	}

	return true;
}

bool MPQFile::checkOptionalHeader(char magic[4], std::string expected)
{
	std::string temp(magic, 4);

	if (temp != expected)
	{
		return false;
	}

	return true;
}

std::vector<std::string> MPQFile::readStringChunk(unsigned char* offset)
{
	std::vector<std::string> ret;

	GenericStringChunk* chunk = (GenericStringChunk*) offset;

	if (!chunk->size)
	{
		return ret;
	}

	unsigned char* start = offset + 8;
	unsigned char* current = start;
	std::string temp;

	while (current - start < chunk->size)
	{
		temp.assign((char *)current);
		ret.push_back(Poco::toLower(temp));
		current += temp.size() + 1;
	}

	return ret;
}