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

#include "M2V1.h"

M2V1::M2V1(std::string name, unsigned char* data, long size) {
	_name = name;
	_data = data;
	size = size;
}
M2V1::~M2V1()
{
	delete _data;
}
bool M2V1::parse()
{
	_logger.debug("Parsing M2 file %s", _name);

	_header = (Header*)(_data);

	if (!checkHeader(_header->magic, HEADER_MD20))
	{
		return false;
	}

	bool supported = false;

	for (int i = 0; i < sizeof(SUPPORTED_VERSIONS) / sizeof(SUPPORTED_VERSIONS[0]); i++)
	{
		if (_header->version == SUPPORTED_VERSIONS[i])
		{
			supported = true;
		}
	}

	return supported;
}

unsigned int M2V1::getNCollisionVertices()
{
	return _header->collisionVertices.size;
}

C3Vector* M2V1::getCollisionVertices()
{
	if (_header->collisionVertices.size)
	{
		return (C3Vector*)(_data + _header->collisionVertices.offset);
	}

	return NULL;
}

unsigned int M2V1::getNCollisionTriangles()
{
	return _header->collisionTriangles.size;
}

unsigned short* M2V1::getCollisionTriangles()
{
	if (_header->collisionTriangles.size)
	{
		return (unsigned short*)(_data + _header->collisionTriangles.offset);
	}
	return NULL;
}

unsigned int M2V1::getNVertices()
{
	return _header->vertices.size;
}

M2Vertex* M2V1::getVertices()
{
	if (_header->vertices.size)
	{
		return (M2Vertex*)(_data + _header->vertices.offset);
	}

	return NULL;
}

M2SkinProfile* M2V1::getSkins()
{
	if (_header->skinProfiles.size)
	{
		return (M2SkinProfile*)(_data + _header->skinProfiles.offset);
	}
	return NULL;
}

M2SkinSection* M2V1::getSubmeshes(unsigned int view)
{
	M2SkinProfile* skins = getSkins();
	return (M2SkinSection*) ((unsigned char*) &skins[view]) + skins[view].subMeshes.offset;
}

unsigned short* M2V1::getIndices(unsigned int view)
{
	M2SkinProfile* skins = getSkins();
	return (unsigned short*)((unsigned char*)&skins[view]) + skins[view].indices.offset;
}