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

#include "MapFile.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/BinaryWriter.h"
#include "Poco/FileStream.h"

using Poco::Path;
using Poco::File;
using Poco::BinaryWriter;
using Poco::FileOutputStream;

MapFile::MapFile(unsigned int mapId, unsigned int x, unsigned int y) : _mapId(mapId), _x(x), _y(y)
{
    memset(liquidShow, 0, sizeof(liquidShow));
    memset(liquidFlags, 0, sizeof(liquidFlags));
    memset(liquidEntry, 0, sizeof(liquidEntry));
    memset(holes, 0, sizeof(holes));
}

MapFile::~MapFile()
{

}

bool MapFile::save(std::string path)
{
    char mapFile[1024];
    sprintf(mapFile, PATTERN_MAP.c_str(), _mapId, _x, _y);
    Path mapPath(path + "/" + mapFile);
    File file(mapPath);
    file.createFile();

    FileOutputStream stream(mapPath.toString());
    BinaryWriter writer(stream);

    // Writing the header.
    writer.writeRaw(header.magic, 4);
    writer.writeRaw(header.versionMagic, 4);
    writer << header.buildMagic << header.areaMapOffset << header.areaMapSize << header.heightMapOffset << header.heightMapSize;
    writer << header.liquidMapOffset << header.liquidMapSize << header.holesOffset << header.holesSize;

    // Writing the area data.
    writer.writeRaw(areaHeader.magic, 4);
    writer << areaHeader.flags << areaHeader.gridArea;
    if (!(areaHeader.flags & MAP_AREA_NO_AREA))
    {
        for (int x = 0; x < ADTV1::SIZE_TILE_MAP; x++)
        {
            for (int y = 0; y < ADTV1::SIZE_TILE_MAP; y++)
            {
                writer << areaFlags[x][y];
            }
        }
    }

    // Writing the height data.
    writer.writeRaw(mapHeightHeader.magic, 4);
    writer << mapHeightHeader.flags << mapHeightHeader.gridHeight << mapHeightHeader.gridMaxHeight;
    if (!(mapHeightHeader.flags & MAP_HEIGHT_NO_HEIGHT))
    {
        if (mapHeightHeader.flags & MAP_HEIGHT_AS_INT16)
        {
            for (int y = 0; y <= ADTV1::SIZE_ADT_GRID; y++)
            {
                for (int x = 0; x <= ADTV1::SIZE_ADT_GRID; x++)
                {
                    writer << (unsigned short) ((V9[y][x] - mapHeightHeader.gridHeight) * heightStep + 0.5f);
                }
            }
            for (int y = 0; y < ADTV1::SIZE_ADT_GRID; y++)
            {
                for (int x = 0; x < ADTV1::SIZE_ADT_GRID; x++)
                {
                    writer << (unsigned short)((V8[y][x] - mapHeightHeader.gridHeight) * heightStep + 0.5f);
                }
            }
        }
        else if (mapHeightHeader.flags & MAP_HEIGHT_AS_INT8)
        {
            for (int y = 0; y <= ADTV1::SIZE_ADT_GRID; y++)
            {
                for (int x = 0; x <= ADTV1::SIZE_ADT_GRID; x++)
                {
                    writer << (unsigned char)((V9[y][x] - mapHeightHeader.gridHeight) * heightStep + 0.5f);

                }
            }
            for (int y = 0; y < ADTV1::SIZE_ADT_GRID; y++)
            {
                for (int x = 0; x < ADTV1::SIZE_ADT_GRID; x++)
                {
                    writer << (unsigned char)((V8[y][x] - mapHeightHeader.gridHeight) * heightStep + 0.5f);

                }
            }
        }
        else
        {
            for (int x = 0; x <= ADTV1::SIZE_ADT_GRID; x++)
            {
                for (int y = 0; y <= ADTV1::SIZE_ADT_GRID; y++)
                {
                    writer << V9[x][y];
                }
            }
            for (int x = 0; x < ADTV1::SIZE_ADT_GRID; x++)
            {
                for (int y = 0; y < ADTV1::SIZE_ADT_GRID; y++)
                {
                    writer << V8[x][y];
                }
            }
        }
    }

    // Store liquid data if needed
    if (header.liquidMapOffset)
    {
        writer.writeRaw(mapLiquidHeader.magic, 4);
        writer << mapLiquidHeader.flags << mapLiquidHeader.liquidType << mapLiquidHeader.offsetX << mapLiquidHeader.offsetY 
            << mapLiquidHeader.width << mapLiquidHeader.height << mapLiquidHeader.liquidLevel;
        if (!(mapLiquidHeader.flags & MAP_LIQUID_NO_TYPE))
        {
            for (int x = 0; x < ADTV1::SIZE_TILE_MAP; x++)
            {
                for (int y = 0; y < ADTV1::SIZE_TILE_MAP; y++)
                {
                    writer << liquidEntry[x][y];
                }
            }
            for (int x = 0; x < ADTV1::SIZE_TILE_MAP; x++)
            {
                for (int y = 0; y < ADTV1::SIZE_TILE_MAP; y++)
                {
                    writer << liquidFlags[x][y];
                }
            }
        }
        if (!(mapLiquidHeader.flags & MAP_LIQUID_NO_HEIGHT))
        {
            for (int y = 0; y < mapLiquidHeader.height; y++)
            {
                for (int i = 0; i < mapLiquidHeader.width; i++)
                {
                    writer << liquidHeight[y + mapLiquidHeader.offsetY][mapLiquidHeader.offsetX + i];
                }
            }
        }
    }

    // Store holes data
    for (int x = 0; x < ADTV1::SIZE_TILE_MAP; x++)
    {
        for (int y = 0; y < ADTV1::SIZE_TILE_MAP; y++)
        {
            writer << holes[x][y];
        }
    }

    stream.close();

    return true;
}