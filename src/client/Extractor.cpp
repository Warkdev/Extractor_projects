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
#include "Poco/MD5Engine.h"
#include "Poco/DigestStream.h"

using Poco::File;
using Poco::MD5Engine;
using Poco::DigestOutputStream;
using Poco::DigestEngine;

void Extractor::loadMPQs()
{
	_mpqManager->load(_listMPQ);
}

void Extractor::exportDBC()
{
	Path path(_outputDBCPath);
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

void Extractor::packAreaData(MapFile* map)
{
    map->fullArea = false;
    map->header.areaMapOffset = sizeof(map->header);
    map->header.areaMapSize = sizeof(MapFile::MapAreaHeader);
    map->areaHeader.flags = 0;

    unsigned int areaflag = map->areaFlags[0][0];
    for (int y = 0; y < ADTV1::SIZE_TILE_MAP; y++)
    {
        for (int x = 0; x < ADTV1::SIZE_TILE_MAP; x++)
        {
            if (map->areaFlags[y][x] != areaflag)
            {
                map->fullArea = true;
                y = ADTV1::SIZE_TILE_MAP;
                break;
            }
        }
    }

    if (map->fullArea)
    {
        map->areaHeader.gridArea = 0;
        map->header.areaMapSize += sizeof(map->areaFlags);
    }
    else {
        map->areaHeader.flags |= MapFile::MAP_AREA_NO_AREA;
        map->areaHeader.gridArea = areaflag;
    }
}

void Extractor::packHeight(MapFile* map)
{
    map->header.heightMapOffset = map->header.areaMapOffset + map->header.areaMapSize;
    map->header.heightMapSize = sizeof(MapFile::MapHeightHeader);

    map->mapHeightHeader.flags = 0;
    float diff = map->mapHeightHeader.gridMaxHeight - map->mapHeightHeader.gridHeight;

    // Don't store if the surface is flat.
    if ((map->mapHeightHeader.gridHeight == map->mapHeightHeader.gridMaxHeight) || (_allowFloatToInt && diff < _flatHeightDeltaLimit))
    {
        map->mapHeightHeader.flags |= MapFile::MAP_HEIGHT_NO_HEIGHT;
    }
    else
    {
        if (_allowFloatToInt)
        {
            if (diff < _floatToByteLimit)
            {
                map->mapHeightHeader.flags |= MapFile::MAP_HEIGHT_AS_INT8;
                map->heightStep = 255 / diff;
                map->header.heightMapSize += 33025; // Size of V8 & V9 on a single byte.
            }
            else if (diff < _floatToShortLimit)
            {
                map->mapHeightHeader.flags |= MapFile::MAP_HEIGHT_AS_INT16;
                map->heightStep = 65535 / diff;
                map->header.heightMapSize += 66050; // Size of V8 & V9 on 2-bytes.
            }
            else
            {
                map->header.heightMapSize += 132100; // Size of V8 & V9 on 4-bytes.
            }
        }
        else
        {
            map->header.heightMapSize += 132100; // Size of V8 & V9 on 4-bytes.
        }
    }
}

void Extractor::packLiquid(MapFile* map)
{
    map->fullLiquidType = false;
    unsigned char liquidType = map->liquidFlags[0][0];

    for (int y = 0; y < ADTV1::SIZE_TILE_MAP; y++)
    {
        for (int x = 0; x < ADTV1::SIZE_TILE_MAP; x++)
        {
            if (map->liquidFlags[y][x] != liquidType)
            {
                map->fullLiquidType = true;
                y = ADTV1::SIZE_TILE_MAP;
                break;
            }
        }
    }

    // No water data, the whole grid has '0' as liquid type.
    if (!liquidType && !map->fullLiquidType)
    {
        map->header.liquidMapOffset = 0;
        map->header.liquidMapSize = 0;
    }
    else
    {
        int minX = 255, minY = 255;
        int maxX = 0, maxY = 0;
        float maxHeight = -20000;
        float minHeight = 20000;
        for (int y = 0; y < ADTV1::SIZE_ADT_GRID; y++)
        {
            for (int x = 0; x < ADTV1::SIZE_ADT_GRID; x++)
            {
                if (map->liquidShow[y][x])
                {
                    if (minX > x)
                    {
                        minX = x;
                    }
                    if (maxX < x)
                    {
                        maxX = x;
                    }
                    if (minY > y)
                    {
                        minY = y;
                    }
                    if (maxY < y)
                    {
                        maxY = y;
                    }
                    float h = map->liquidHeight[y][x];
                    if (maxHeight < h)
                    {
                        maxHeight = h;
                    }
                    if (minHeight > h)
                    {
                        minHeight = h;
                    }
                }
                else
                {
                    map->liquidHeight[y][x] = _useMinHeight;
                }
            }
        }
        map->header.liquidMapOffset = map->header.heightMapOffset + map->header.heightMapSize;
        map->header.liquidMapSize = sizeof(MapFile::MapLiquidHeader);
        map->mapLiquidHeader.flags = 0;
        map->mapLiquidHeader.liquidType = 0;
        map->mapLiquidHeader.offsetX = minX;
        map->mapLiquidHeader.offsetY = minY;
        map->mapLiquidHeader.width = maxX - minX + 1 + 1;
        map->mapLiquidHeader.height = maxY - minY + 1 + 1;
        map->mapLiquidHeader.liquidLevel = minHeight;

        if (maxHeight == minHeight)
        {
            map->mapLiquidHeader.flags |= MapFile::MAP_LIQUID_NO_HEIGHT;
        }

        // Not need store if flat surface
        if (_allowFloatToInt && (maxHeight - minHeight) < _flatLiquidDeltaLimit)
        {
            map->mapLiquidHeader.flags |= MapFile::MAP_LIQUID_NO_HEIGHT;
        }

        if (!map->fullLiquidType)
        {
            map->mapLiquidHeader.flags |= MapFile::MAP_LIQUID_NO_TYPE;
        }

        if (map->mapLiquidHeader.flags & MapFile::MAP_LIQUID_NO_TYPE)
        {
            map->mapLiquidHeader.liquidType = liquidType;
        }
        else
        {
            map->header.liquidMapSize += sizeof(map->liquidEntry) + sizeof(map->liquidFlags);
        }

        if (!(map->mapLiquidHeader.flags & MapFile::MAP_LIQUID_NO_HEIGHT))
        {
            map->header.liquidMapSize += sizeof(float) * map->mapLiquidHeader.width * map->mapLiquidHeader.height;
        }
    }
}

void Extractor::packHoles(MapFile* map)
{
    if (map->header.liquidMapOffset)
    {
        map->header.holesOffset = map->header.liquidMapOffset + map->header.liquidMapSize;
    }
    else
    {
        map->header.holesOffset = map->header.heightMapOffset + map->header.heightMapSize;
    }

    map->header.holesSize = sizeof(map->holes);
}

void Extractor::packData(MapFile* map)
{
    sprintf(map->header.versionMagic, _mapVersion.c_str());
    map->header.buildMagic = _buildVersion;
    map->header.areaMapOffset = sizeof(map->header);
    map->header.areaMapSize = sizeof(MapFile::MapAreaHeader);

    // First, pack area data.
    packAreaData(map);

    // Then, pack height data.
    packHeight(map);

    // Then, pack liquid data.
    packLiquid(map);

    // Then, pack holes data.
    packHoles(map);
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

std::string Extractor::getUniformName(std::string filename)
{
    MD5Engine engine;
    DigestOutputStream ds(engine);
    std::string hash;

    ds << filename;
    hash = DigestEngine::digestToHex(engine.digest());
    ds.clear();
    ds.close();

    return hash;
}