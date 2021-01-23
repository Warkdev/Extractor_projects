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

#include "ADTV1.h"

ADTV1::ADTV1(std::string name, unsigned char* data, long size)
{
	_name = name;
	_size = size;
	_data = data;
}

ADTV1::~ADTV1()
{
	delete _data;
}

bool ADTV1::parse()
{
	_logger.debug("Parsing ADTv1 file %s", _name);

	std::string magic;

	_version = (MVER*) (_data);

	magic = std::string(_version->magic, 4);

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

	_header = (MHDR*) (_data + 8 + _version->size);

	magic = std::string(_header->magic, 4);

	if (magic != HEADER_MHDR)
	{
		_logger.error("Expected header DHPM not found");
		return false;
	}

	_chunkInfos = (MCIN*)(_data + GLOBAL_OFFSET + _header->offsetMCIN);

	magic = std::string(_chunkInfos->magic);

	if (magic != HEADER_MCIN)
	{
		_logger.error("Expected header NICM not found");
		return false;
	}

	return true;
}

MCNK* ADTV1::getCell(unsigned int x, unsigned int y)
{
	if (x < 0 || x >= SIZE_TILE_MAP || y < 0 || y >= SIZE_TILE_MAP)
	{
		return NULL;
	}

	return (MCNK*) (_data + _chunkInfos->cells[x][y].offsetMCNK);
}

MCVT* ADTV1::getVertices(MCNK* chunk)
{
	MCVT* v = (MCVT*)((unsigned char*)chunk + chunk->offsetMCVT);
	std::string magic(v->magic, 4);

	if (magic != HEADER_MCVT)
	{
		_logger.error("Expected header TVCM not found");
		return NULL;
	}

	return v;
}

MCLQ* ADTV1::getLiquid(MCNK* chunk)
{
	if (!hasLiquid(chunk))
	{
		_logger.debug("No liquid chunk for this cell");
		return NULL;
	}

	MCLQ* liq = (MCLQ*)((unsigned char*)chunk + chunk->offsetMCLQ);
	std::string magic(liq->magic, 4);

	if (magic != HEADER_MCLQ)
	{
		_logger.error("Expected header QLCM not found");
		return NULL;
	}

	return liq;
}

bool ADTV1::hasLiquid(MCNK* chunk)
{
	return chunk->flags & MCNK_IS_RIVER || chunk->flags & MCNK_IS_OCEAN || chunk->flags & MCNK_IS_MAGMA || chunk->flags & MCNK_IS_SLIME;
}

bool ADTV1::hasLiquid(MCLQ::MCLQLayer* liquid, unsigned int x, unsigned int y)
{
	return liquid->flags[x][y] & MCLQ_HAS_LIQUID;
}

bool ADTV1::hasNoLiquid(MCLQ::MCLQLayer* liquid, unsigned int x, unsigned int y)
{
	return (liquid->flags[x][y] & MCLQ_NO_LIQUID) == MCLQ_NO_LIQUID;
}

bool ADTV1::isDarkWater(MCLQ::MCLQLayer* liquid, unsigned int x, unsigned int y)
{
	return liquid->flags[x][y] & MCLQ_IS_DARK;
}

bool ADTV1::hasNoLiquid(MCNK* chunk)
{
	return !hasLiquid(chunk);
}

bool ADTV1::isRiver(MCNK* chunk)
{
	return chunk->flags & MCNK_IS_RIVER;
}

bool ADTV1::isOcean(MCNK* chunk)
{
	return chunk->flags & MCNK_IS_OCEAN;
}

bool ADTV1::isMagma(MCNK* chunk)
{
	return chunk->flags & MCNK_IS_MAGMA;
}

bool ADTV1::isSlime(MCNK* chunk)
{
	return chunk->flags & MCNK_IS_SLIME;
}

std::string ADTV1::getName()
{
	return std::string(_name);
}

std::vector<std::string> ADTV1::getModels()
{
	return readStringChunk(_data + GLOBAL_OFFSET + _header->offsetMMDX);
}

std::vector<std::string> ADTV1::getWorldModels() 
{
	return readStringChunk(_data + GLOBAL_OFFSET + _header->offsetMWMO);
}