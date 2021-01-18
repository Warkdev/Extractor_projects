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
#include "Poco/MemoryStream.h"

using Poco::MemoryInputStream;

MPQFile::MPQFile()
{
	_name = "";
	_buffer = NULL;
}

MPQFile::MPQFile(std::string name, char* data, long size) : _name(name),  _size(size)
{
	MemoryInputStream* stream = new MemoryInputStream(data, _size);
	_buffer = new BinaryReader(*stream);
	_data = data;
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

std::string MPQFile::readString()
{
	std::string str;
	char c;
	bool complete = false;

	// We read what's on the buffer until we find a '0' terminating the string.
	while (!complete && (_buffer->stream().read(&c, 1)))
	{
		if (c == '\0')
		{
			complete = true;
		}
		else {
			str.append(&c, 1);
		}
	}
	return str;
}