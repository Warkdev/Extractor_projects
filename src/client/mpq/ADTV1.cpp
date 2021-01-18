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
#include "Poco/BinaryReader.h"
#include "Poco/MemoryStream.h"

using Poco::BinaryReader;
using Poco::MemoryInputStream;

ADTV1::ADTV1(std::string name, char* data, long size)
{
	_name = name;
	_size = size;
	MemoryInputStream* stream = new MemoryInputStream(data, _size);
	_buffer = new BinaryReader(*stream);
	_data = data;
}

ADTV1::~ADTV1()
{
	delete _data;
	delete _chunkInfos;
	delete _buffer;
}

bool ADTV1::parse()
{
	_logger.debug("Parsing ADTv1 file %s", _name);

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

	if (_header.magic != HEADER_MHDR)
	{
		_logger.error("Expected header DHPM not found");
		return false;
	}

	*_buffer >> _header.size;
	*_buffer >> _header.flags;
	*_buffer >> _header.offsetMCIN;
	*_buffer >> _header.offsetMTEX;
	*_buffer >> _header.offsetMMDX;
	*_buffer >> _header.offsetMMID;
	*_buffer >> _header.offsetMWMO;
	*_buffer >> _header.offsetMWID;
	*_buffer >> _header.offsetMDDF;
	*_buffer >> _header.offsetMODF;

	if (!readMCIN()) 
	{
		_logger.error("Error while reading the MCIN structure");
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

	// Move the stream at the correct location.
	MCNK* chunk = new MCNK;
	std::string temp;
	unsigned int offset = _chunkInfos[x * SIZE_TILE_MAP + y].offsetMCNK;
	_buffer->stream().seekg(offset, std::ios::beg);
	_buffer->readRaw(4, chunk->magic);

	if (chunk->magic != HEADER_MCNK)
	{
		_logger.error("Expected header KNCM not found");
		return NULL;
	}

	*_buffer >> chunk->size;
	*_buffer >> chunk->flags >> chunk->indexX >> chunk->indexY >> chunk->nbLayers >> chunk->nDoodadRefs >> chunk->offsetMCVT;
	*_buffer >> chunk->offsetMCNR >> chunk->offsetMCLY >> chunk->offsetMCRF >> chunk->offsetMCAL >> chunk->sizeAlpha;
	*_buffer >> chunk->offsetMCSH >> chunk->sizeShadow >> chunk->areaId >> chunk->nMapObjRefs >> chunk->holes;
	for (int i = 0; i < (sizeof(chunk->lowQualityTextMap) / sizeof(chunk->lowQualityTextMap[0])); i++)
	{
		for (int j = 0; j < sizeof(chunk->lowQualityTextMap[i]); j++)
		{
			*_buffer >> chunk->lowQualityTextMap[i][j];
		}
	}
	*_buffer >> chunk->predTex >> chunk->noEffectDoodad >> chunk->offsetMCSE >> chunk->nSndEmitters >> chunk->offsetMCLQ >> chunk->sizeLiquid;
	*_buffer >> chunk->position >> chunk->offsetMCCV >> chunk->offsetMCLV;
	
	// We move at the MCVT offset.
	_buffer->stream().seekg(static_cast<unsigned __int64>(offset) + chunk->offsetMCVT, std::ios::beg);

	_buffer->readRaw(4, chunk->vertices.magic);

	if (chunk->vertices.magic != HEADER_MCVT)
	{
		_logger.error("Expected header TVCM");
		return NULL;
	}

	*_buffer >> chunk->vertices.size;
	for (int i = 0; i < 145; i++)
	{
		*_buffer >> chunk->vertices.points[i];
	}

	// We move at the MCNR offset.
	_buffer->stream().seekg(static_cast<unsigned __int64>(offset) + chunk->offsetMCNR, std::ios::beg);
	_buffer->readRaw(4, chunk->normals.magic);

	if (chunk->normals.magic != HEADER_MCNR)
	{
		_logger.error("Expected header RNCM");
		return NULL;
	}

	*_buffer >> chunk->normals.size;
	for (int i = 0; i < 145; i++)
	{
		*_buffer >> chunk->normals.points[i];
	}

	if (hasLiquid(chunk))
	{
		// Now, we move at the location of MCLQ chunk.
		_buffer->stream().seekg(static_cast<unsigned __int64>(offset) + chunk->offsetMCLQ, std::ios::beg);
		_buffer->readRaw(4, temp);

		if (temp != HEADER_MCLQ)
		{
			_logger.error("Expected header QLCM");
			return NULL;
		}
		// We skip the size, not always good.
		_buffer->stream().seekg(4, std::ios::cur);
		int liquidLayers = 0;
		if (chunk->flags & MCNK_IS_RIVER)
		{
			liquidLayers++;
		}
		if (chunk->flags & MCNK_IS_OCEAN)
		{
			liquidLayers++;
		}
		if (chunk->flags & MCNK_IS_MAGMA)
		{
			liquidLayers++;
		}
		if (chunk->flags & MCNK_IS_SLIME)
		{
			liquidLayers++;
		}

		chunk->listLiquids = new MCNK::MCLQ[liquidLayers];

		for (int i = 0; i < liquidLayers; i++)
		{
			*_buffer >> chunk->listLiquids[i].minHeight >> chunk->listLiquids[i].maxHeight;
			for (int j = 0; j < chunk->LIQUID_DATA_LENGTH; j++)
			{
				for (int k = 0; k < chunk->LIQUID_DATA_LENGTH; k++)
				{
					*_buffer >> chunk->listLiquids[i].lights[j * chunk->LIQUID_DATA_LENGTH + k] >> chunk->listLiquids[i].height[j * chunk->LIQUID_DATA_LENGTH + k];
				}
			}
			for (int j = 0; j < chunk->LIQUID_FLAG_LENGTH; j++)
			{
				for (int k = 0; k < chunk->LIQUID_FLAG_LENGTH; k++)
				{
					*_buffer >> chunk->listLiquids[i].flags[j * chunk->LIQUID_FLAG_LENGTH + k];
				}
			}

			// We pass on the SWFlowv, doesn't really matter for us.
		}
	}

	return chunk;
}

bool ADTV1::readMCIN()
{
	// Move the stream at the correct location.
	_buffer->stream().seekg(static_cast<unsigned __int64>(GLOBAL_OFFSET) + _header.offsetMCIN, std::ios::beg);
	std::string magic;
	unsigned int size;
	_buffer->readRaw(4, magic);

	if (magic != HEADER_MCIN)
	{
		_logger.error("Expected header NICM not found");
		return false;
	}

	*_buffer >> size;

	_cells = size / sizeof(MCIN);
	_chunkInfos = new MCIN[_cells];

	for (int i = 0; i < _cells; i++)
	{
		*_buffer >> _chunkInfos[i].offsetMCNK;
		*_buffer >> _chunkInfos[i].size;
		*_buffer >> _chunkInfos[i].flags;
		*_buffer >> _chunkInfos[i].asyncId;
	}

	return true;
}

unsigned int ADTV1::cellsSize()
{
	return _cells;
}

bool ADTV1::hasLiquid(MCNK* chunk)
{
	return chunk->flags & MCNK_IS_RIVER || chunk->flags & MCNK_IS_OCEAN || chunk->flags & MCNK_IS_MAGMA || chunk->flags & MCNK_IS_SLIME;
}

bool ADTV1::hasLiquid(MCNK::MCLQ* liquid, unsigned int x, unsigned int y)
{
	return liquid->flags[x * MCNK::LIQUID_FLAG_LENGTH + y] & MCLQ_HAS_LIQUID;
}

bool ADTV1::hasNoLiquid(MCNK::MCLQ* liquid, unsigned int x, unsigned int y)
{
	return (liquid->flags[x * MCNK::LIQUID_FLAG_LENGTH + y] & MCLQ_NO_LIQUID) == MCLQ_NO_LIQUID;
}

bool ADTV1::isDarkWater(MCNK::MCLQ* liquid, unsigned int x, unsigned int y)
{
	return liquid->flags[x * MCNK::LIQUID_FLAG_LENGTH + y] & MCLQ_IS_DARK;
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