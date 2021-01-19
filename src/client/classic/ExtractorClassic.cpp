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

#include "ExtractorClassic.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "../../maps/MapFile.h"
#include "../mpq/WDT.h"
#include "../mpq/ADTV1.h"

using Poco::Path;
using Poco::File;

void ExtractorClassic::init(std::string clientPath)
{
	_logger.information("Initializing ExtractorClassic..");
	Path client(clientPath);
    // This if the complete MPQ list but some of them have no interesting or recent data in them. Save time by not loading them.
	//std::string mpqs[] = { "base.MPQ", "dbc.MPQ", "misc.MPQ", "model.MPQ", "speech.MPQ", "terrain.MPQ", "texture.MPQ", "wmo.MPQ", "patch.MPQ", "patch-2.MPQ" };
    std::string mpqs[] = { "dbc.MPQ", "model.MPQ", "terrain.MPQ", "wmo.MPQ", "patch.MPQ", "patch-2.MPQ" };
	for (int i = 0; i < (sizeof(mpqs) / sizeof(mpqs[0])); i++)
	{
		Path path(client, mpqs[i]);
		_listMPQ.push_back(path.toString());
	}
	_version = Version::CLIENT_CLASSIC;
}

void ExtractorClassic::exportMaps(std::string outputPath)
{
	_logger.information("Extracting Maps..");
	Path path(outputPath + PATH_MAPS);
	File file(path);
	file.createDirectories();

	// For Classic, we only need Maps and AreaTable data.
	readMaps();
	readAreaTable();
	char wdtFile[1024];
	char adtFile[1024];

    bool allowHeightLimit = _config->getBool(PROP_ALLOW_HEIGHT_LIMIT);
    float useMinHeight = (float)_config->getDouble(PROP_USE_MIN_HEIGHT);
    bool allowFloatToInt = _config->getBool(PROP_ALLOW_FLOAT_TO_INT);
    float floatHeightDeltaTimit = (float)_config->getDouble(PROP_FLAT_HEIGHT_DELTA_LIMIT);
    float floatLiquidDeltaTimit = (float)_config->getDouble(PROP_FLAT_LIQUID_DELTA_LIMIT);
    float floatToByteLimit = (float)_config->getDouble(PROP_FLOAT_TO_BYTE_LIMIT);
    float floatToShortLimit = (float)_config->getDouble(PROP_FLOAT_TO_SHORT_LIMIT);
    unsigned int maxAreaId = _areas.rbegin()->first;

	for (auto it = _maps.begin(); it != _maps.end(); it++)
	{
		_logger.information("Loading WDT file %s.wdt (%u)", it->second, it->first);
		sprintf(wdtFile, PATTERN_WDT.c_str(), it->second.c_str(), it->second.c_str());
		WDT* wdt = (WDT*) _mpqManager->getFile(std::string(wdtFile), _version);
		if (!wdt)
		{
			_logger.warning("WDT file does not exist, warning can be safely ignored by not map info will be generated");
			delete wdt;
			continue;
		}
		if (!wdt->parse())
		{
			_logger.error("Error while parsing the WDT file");
			delete wdt;
			continue;
		}
		
		for (int x = 0; x < WDT::MAP_TILE_SIZE; x++)
		{
			for (int y = 0; y < WDT::MAP_TILE_SIZE; y++)
			{
				if (wdt->hasADT(x, y))
				{
					_logger.debug("Loading ADT file (X: %i, Y: %i)", x, y);
					sprintf(adtFile, PATTERN_ADT.c_str(), it->second.c_str(), it->second.c_str(), y, x); // Note how index are reversed.
					ADTV1* adt = (ADTV1*) _mpqManager->getFile(std::string(adtFile), _version);

					if (!adt)
					{
						_logger.error("ADT file does not exist");
						delete adt;
						continue;
					}

					if (!adt->parse())
					{
						_logger.error("Error while parsing the ADT file");
						delete adt;
						continue;
					}

					MapFile map(it->first, x, y);

					if (!convertADT(adt, &map, maxAreaId, allowHeightLimit, allowFloatToInt, floatHeightDeltaTimit, floatLiquidDeltaTimit, floatToByteLimit, floatToShortLimit, useMinHeight))
					{
						_logger.error("Error while generating the map file");
						delete adt;
						continue;
					}

					if (!map.save(path.toString()))
					{
						_logger.error("Error while saving the map file");
						delete adt;
						continue;
					}

					delete adt;
				}
			}
		}

		delete wdt;
	}
}

bool ExtractorClassic::convertADT(ADTV1* adt, MapFile* map, unsigned int maxAreaId, bool allowHeightLimit, bool allowFloatToInt, float floatHeightDeltaLimit, float floatLiquidDeltaLimit, float floatToByteLimit, float floatToShortLimit, float useMinHeight)
{
    map->mapHeightHeader.gridMaxHeight = -200000;
    map->mapHeightHeader.gridHeight = 20000;
    map->heightStep = 0.0f;
 
    for (int i = 0; i < ADTV1::SIZE_TILE_MAP; i++)
    {
        for (int j = 0; j < ADTV1::SIZE_TILE_MAP; j++)
        {
            MCNK* cell = adt->getCell(i, j);
            if (!cell)
            {
                _logger.error("Couldn't acquire a chunk!");
                continue;
            }

            handleAreas(map, cell, i, j, maxAreaId);
            handleHeight(map, adt, cell, i, j, allowHeightLimit, useMinHeight);
            handleLiquid(map, adt, cell, i, j);
            handleHoles(map, cell, i, j);
        }
    }

    packData(map, allowFloatToInt, floatHeightDeltaLimit, floatLiquidDeltaLimit, floatToByteLimit, floatToShortLimit, useMinHeight);

    return true;
}

void ExtractorClassic::handleAreas(MapFile* map, MCNK* cell, unsigned int i, unsigned int j, unsigned int maxAreaId)
{
    if (cell->areaId && cell->areaId <= maxAreaId)
    {
        unsigned int flag = _areas.find(cell->areaId)->second;
        if (flag == 0xFFFF)
        {
            _logger.warning("Can not find area flag for area %u [%u, %u]", cell->areaId, cell->indexX, cell->indexY);
        }
        map->areaFlags[i][j] = flag;
    }
    else {
        map->areaFlags[i][j] = 0xFFFF;
    }
}

void ExtractorClassic::handleHeight(MapFile* map, ADTV1* adt, MCNK* cell, unsigned int i, unsigned int j, bool allowHeightLimit, float useMinHeight)
{
    // Get custom height
    MCVT* v = adt->getVertices(cell);

    // Height values for triangles stored in order:
    // 1     2     3     4     5     6     7     8     9
    //    10    11    12    13    14    15    16    17
    // 18    19    20    21    22    23    24    25    26
    //    27    28    29    30    31    32    33    34
    // . . . . . . . .
    // For better get height values merge it to V9 and V8 map
    // V9 height map:
    // 1     2     3     4     5     6     7     8     9
    // 18    19    20    21    22    23    24    25    26
    // . . . . . . . .
    // V8 height map:
    //    10    11    12    13    14    15    16    17
    //    27    28    29    30    31    32    33    34
    // . . . . . . . .

    // Set map height as grid height
    for (int y = 0; y <= ADTV1::CHUNK_TILE_MAP_LENGTH; y++)
    {
        int cy = i * ADTV1::CHUNK_TILE_MAP_LENGTH + y;
        for (int x = 0; x <= ADTV1::CHUNK_TILE_MAP_LENGTH; x++)
        {
            int cx = j * ADTV1::CHUNK_TILE_MAP_LENGTH + x;
            if (y < ADTV1::CHUNK_TILE_MAP_LENGTH && x < ADTV1::CHUNK_TILE_MAP_LENGTH) // V8 has only 8*8 dimensions.
            {
                map->V8[cy][cx] = cell->position[2] + v->points[y * (ADTV1::CHUNK_TILE_MAP_LENGTH * 2 + 1) + ADTV1::CHUNK_TILE_MAP_LENGTH + 1 + x];

                // Check for allow limit minimum height (not store height in deep ochean - allow save some memory)
                if (allowHeightLimit)
                {
                    if (map->V8[cy][cx] < useMinHeight)
                    {
                        map->V8[cy][cx] = useMinHeight;
                    }
                }

                if (map->mapHeightHeader.gridMaxHeight < map->V8[cy][cx])
                {
                    map->mapHeightHeader.gridMaxHeight = map->V8[cy][cx];
                }
                if (map->mapHeightHeader.gridHeight > map->V8[cy][cx])
                {
                    map->mapHeightHeader.gridHeight = map->V8[cy][cx];
                }
            }

            map->V9[cy][cx] = cell->position[2] + v->points[y * (ADTV1::CHUNK_TILE_MAP_LENGTH * 2 + 1) + x];

            if (allowHeightLimit)
            {
                if (map->V9[cy][cx] < useMinHeight)
                {
                    map->V9[cy][cx] = useMinHeight;
                }
            }

            if (map->mapHeightHeader.gridMaxHeight < map->V9[cy][cx])
            {
                map->mapHeightHeader.gridMaxHeight = map->V9[cy][cx];
            }
            if (map->mapHeightHeader.gridHeight > map->V9[cy][cx])
            {
                map->mapHeightHeader.gridHeight = map->V9[cy][cx];
            }
        }
    }
}

void ExtractorClassic::handleLiquid(MapFile* map, ADTV1* adt, MCNK* cell, unsigned int i, unsigned int j)
{
    MCLQ* liquids = adt->getLiquid(cell);
    MCLQ::MCLQLayer* liquid;
    int count = 0;
    if (!liquids || cell->sizeLiquid <= 8)
    {
        return;
    }

    int layers = 0;

    // Detecting layers.
    if (adt->isRiver(cell))
    {
        map->liquidEntry[i][j] = 1;
        map->liquidFlags[i][j] |= MapFile::MAP_LIQUID_TYPE_WATER;            // water
        layers++;
    }
    if (adt->isOcean(cell))
    {
        map->liquidEntry[i][j] = 2;
        map->liquidFlags[i][j] |= MapFile::MAP_LIQUID_TYPE_OCEAN;            // ocean
        layers++;
    }
    if (adt->isMagma(cell) || adt->isSlime(cell))
    {
        map->liquidEntry[i][j] = 3;
        map->liquidFlags[i][j] |= MapFile::MAP_LIQUID_TYPE_MAGMA;            // magma/slime
        layers++;
    }

    if (layers > 1)
    {
        _logger.debug("Found %i layers of liquids in this chunk (ADT: %s, Cell X: %u, Cell Y: %u)", layers, adt->getName(), i, j);
    }

    for (int layer = 0; layer < layers; ++layer)
    {
        liquid = (MCLQ::MCLQLayer*) ((unsigned char*) liquids + 8 + (sizeof(MCLQ::MCLQLayer) * layer)); // We move the pointer by 8 (the magic + size of MCLQ) and the size of a layer.
        for (int y = 0; y < ADTV1::CHUNK_TILE_MAP_LENGTH; y++)
        {
            int cy = i * ADTV1::CHUNK_TILE_MAP_LENGTH + y;
            for (int x = 0; x < ADTV1::CHUNK_TILE_MAP_LENGTH; x++)
            {
                int cx = j * ADTV1::CHUNK_TILE_MAP_LENGTH + x;
                if (!adt->hasNoLiquid(liquid, y, x))
                {
                    map->liquidShow[cy][cx] = true;
                    map->liquidHeight[cy][cx] = liquid->data[y][x].height;
                    if (adt->isDarkWater(liquid, y, x))
                    {
                        map->liquidFlags[i][j] |= MapFile::MAP_LIQUID_TYPE_DARK_WATER;
                    }
                    ++count;
                }
            }
        }
    }
}

void ExtractorClassic::handleHoles(MapFile* map, MCNK* cell, unsigned int i, unsigned int j)
{
    map->holes[i][j] = cell->holes;
}

void ExtractorClassic::packAreaData(MapFile* map)
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

void ExtractorClassic::packHeight(MapFile* map, bool allowFloatToInt, float floatHeightDeltaLimit, float floatToByteLimit, float floatToShortLimit)
{
    map->header.heightMapOffset = map->header.areaMapOffset + map->header.areaMapSize;
    map->header.heightMapSize = sizeof(MapFile::MapHeightHeader);

    map->mapHeightHeader.flags = 0;
    float diff = map->mapHeightHeader.gridMaxHeight - map->mapHeightHeader.gridHeight;

    // Don't store if the surface is flat.
    if ((map->mapHeightHeader.gridHeight == map->mapHeightHeader.gridMaxHeight) || (allowFloatToInt && diff < floatHeightDeltaLimit))
    {
        map->mapHeightHeader.flags |= MapFile::MAP_HEIGHT_NO_HEIGHT;
    }
    else
    {
        if (allowFloatToInt)
        {
            if (diff < floatToByteLimit)
            {
                map->mapHeightHeader.flags |= MapFile::MAP_HEIGHT_AS_INT8;
                map->heightStep = 255 / diff;
                map->header.heightMapSize += 33025; // Size of V8 & V9 on a single byte.
            }
            else if (diff < floatToShortLimit)
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

void ExtractorClassic::packLiquid(MapFile* map, bool allowFloatToInt, float floatLiquidDeltaLimit, float useMinHeight)
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
                    map->liquidHeight[y][x] = useMinHeight;
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
        if (allowFloatToInt && (maxHeight - minHeight) < floatLiquidDeltaLimit)
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

void ExtractorClassic::packHoles(MapFile* map)
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

void ExtractorClassic::packData(MapFile* map, bool allowFloatToInt, float floatHeightDeltaLimit, float floatLiquidDeltaLimit, float floatToByteLimit, float floatToShortLimit, float useMinHeight)
{
    sprintf(map->header.versionMagic, "z1.5");
    map->header.buildMagic = 5875; // Classic build number, we only take 1.12 here.
    map->header.areaMapOffset = sizeof(map->header);
    map->header.areaMapSize = sizeof(MapFile::MapAreaHeader);

    // First, pack area data.
    packAreaData(map);

    // Then, pack height data.
    packHeight(map, allowFloatToInt, floatHeightDeltaLimit, floatToByteLimit, floatToShortLimit);

    // Then, pack liquid data.
    packLiquid(map, allowFloatToInt, floatLiquidDeltaLimit, useMinHeight);

    // Then, pack holes data.
    packHoles(map);
}