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
#include "Poco/String.h"
#include "Poco/MD5Engine.h"
#include "Poco/DigestStream.h"
#include "../mpq/WDT.h"

using Poco::Path;
using Poco::File;
using Poco::MD5Engine;
using Poco::DigestOutputStream;
using Poco::DigestEngine;

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

void ExtractorClassic::extract(std::string outputPath, bool exportMap, bool generateVmaps)
{

    // For Classic, we only need Maps and AreaTable data.
    readMaps();
    readAreaTable();

    if (exportMap)
    {
        exportMaps(outputPath);
    }

    if (generateVmaps)
    {
        //readLiquidType();
        bool cacheToDisk = _config->getBool(PROP_VMAP_CACHE_TO_DISK);
        Path path(outputPath + PATH_MODELS);
        File file(path);

        if (cacheToDisk)
        {
            file.createDirectories();
        }

        exportWMOs(path.toString(), cacheToDisk);
        exportModels(path.toString(), cacheToDisk);

        _logger.information("Extraction complete");

        if (cacheToDisk)
        {
            //file.remove(true);
        }
    }
}

void ExtractorClassic::exportMaps(std::string outputPath)
{
	_logger.information("Extracting Maps..");
	Path path(outputPath + PATH_MAPS);
	File file(path);
	file.createDirectories();

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
			_logger.warning("WDT file does not exist, warning can be safely ignored but no map info will be generated");
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
                // We use printf here because there's no known way to do it with Poco.
                printf(" Processing........................%d%%\r", (100 * (x + 1)) / WDT::MAP_TILE_SIZE);
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

    packData("z1.5", 5875, map, allowFloatToInt, floatHeightDeltaLimit, floatLiquidDeltaLimit, floatToByteLimit, floatToShortLimit, useMinHeight);

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

void ExtractorClassic::exportWMOs(std::string outputPath, bool cacheToDisk)
{
    _logger.information("Extracting WMOs..");
    bool preciseVectorData = _config->getBool(PROP_VMAP_WMO_PRECISE_VECTOR_DATA);
    Path path(outputPath);

    std::vector<std::string> wmos = _mpqManager->getWMOList();
    Path temp;
    std::string flatName;
    std::string hash;
    MD5Engine engine;
    DigestOutputStream ds(engine);
    char groupFile[1024];
    unsigned int count = 0;
    unsigned int total = (unsigned int) wmos.size();

    for (auto it = wmos.begin(); it < wmos.end(); ++it)
    {
        _logger.debug("Extracting wmo %s", *it);
        temp.assign(*it, Path::Style::PATH_WINDOWS); // Force path to interpret it with '\' as separator.
        ds << temp.parent().toString();
        hash = DigestEngine::digestToHex(engine.digest());
        ds.clear();
        WMOV1* wmo = (WMOV1*)_mpqManager->getFile(*it, _version);

        if (!wmo)
        {
            _logger.warning("WMO file does not exist, warning can be safely ignored but no vmap info will be generated");
            continue;
        }

        if (!wmo->parse())
        {
            _logger.error("Error while parsing the WMO file %s ", *it);
            delete wmo;
            continue;
        }

        ModelFile* wmoFile = new ModelFile(temp.parent().toString(), hash, temp.getFileName());

        convertWMORoot(wmo, wmoFile);

        for (int i = 0; i < wmo->getNGroups(); i++) {
            sprintf(groupFile, "%s%s_%03d.wmo", temp.parent().toString().c_str(), temp.getBaseName().c_str(), i);
            WMOGroupV1* group = (WMOGroupV1*)_mpqManager->getFile(groupFile, _version);

            if (!group)
            {
                _logger.warning("WMO Group file (%s) does not exist, warning can be safely ignored but no vmap info will be generated", std::string(groupFile));
                continue;
            }

            if (!group->parse())
            {
                _logger.error("Error while parsing the WMO Group file %s", std::string(groupFile));
                delete group;
                continue;
            }

            convertWMOGroup(wmo, group, wmoFile, i, preciseVectorData);

            delete group;
        }

        if (cacheToDisk)
        {
            wmoFile->save(path.toString());
            delete wmoFile;
        }

        // We use printf here because there's no known way to do it with Poco.
        count++;
        printf(" Processing........................%d%%\r", (100 * (count + 1)) / total);

        delete wmo;
    }

    ds.close();
}

bool ExtractorClassic::convertWMORoot(WMOV1* wmo, ModelFile* file)
{
    strncpy(file->header.versionMagic, "z07\0", 4);
    file->header.nGroups = wmo->getNGroups();
    file->header.rootWMOID = wmo->getWMOID();
    file->groups = new ModelFile::VmapGroup[wmo->getNGroups()];
    return true;
}

bool ExtractorClassic::convertWMOGroup(WMOV1* root, WMOGroupV1* wmoGroup, ModelFile* file, unsigned int groupIdx, bool preciseVectorData)
{
    MOGP::GroupInfo* info = wmoGroup->getGroupInfo();
    MOBA* batchInfo = wmoGroup->getBatchInfo();
    MOPY* polyInfo = wmoGroup->getPolyInfo();
    MOVI* vertexIndices = wmoGroup->getVertexIndices();
    MOVT* vertexInfo = wmoGroup->getVertexInfo();
    ModelFile::VmapGroup* group = &(file->groups[groupIdx]);

    // Source:: MOGP
    group->flags = info->flags;
    group->groupWMOID = info->wmoAreaTableRecId;
    group->boundingBox.min.x = info->boundingBox.min.x;
    group->boundingBox.min.y = info->boundingBox.min.y;
    group->boundingBox.min.z = info->boundingBox.min.z;
    group->boundingBox.max.x = info->boundingBox.max.x;
    group->boundingBox.max.y = info->boundingBox.max.y;
    group->boundingBox.max.z = info->boundingBox.max.z;
    group->liquidFlags = 0;

    // Source: MOBA
    group->header.mobaBatch = batchInfo->size / sizeof(MOBA::Batch);
    group->header.mobaSize = (group->header.mobaBatch * 4) + 4;
    group->header.mobaEx = new unsigned int[group->header.mobaBatch];

    unsigned int k = 0;
    for (int i = 0; i < group->header.mobaBatch; i++)
    {
        group->header.mobaEx[k++] = batchInfo->batches[i].count;
    }

    if (preciseVectorData)
    {
        file->header.nVectors += polyInfo->size / sizeof(unsigned short);
        // Source: MOPY
        group->indices.nIndices = polyInfo->size / sizeof(unsigned short) * 3;
        group->indices.size = sizeof(unsigned int) + (sizeof(unsigned short) * group->indices.nIndices);
        group->indices.indices = new unsigned short[group->indices.nIndices];
        memcpy(group->indices.indices, vertexIndices->indexes, group->indices.nIndices * sizeof(unsigned short));

        // Source: MOVT
        group->vertices.nVertices = vertexInfo->size / sizeof(MOVT::Vertex);
        group->vertices.size = sizeof(unsigned int) + vertexInfo->size;
        group->vertices.vertices = new ModelFile::VmapGroup::Vertices::Vertex[group->vertices.nVertices];
        memcpy(group->vertices.vertices, vertexInfo->vertices, group->vertices.nVertices * sizeof(ModelFile::VmapGroup::Vertices::Vertex));
    }
    else {
        unsigned int nColTriangles = 0;
        unsigned int nColVertices = 0;
        unsigned int nVectors = polyInfo->size / sizeof(unsigned short);
        unsigned int nVertices = vertexInfo->size / sizeof(MOVT::Vertex);
        unsigned short* moviEx = new unsigned short[nVectors * 3]; // Worst case, all triangles are collisions ones.
        int* newIndex = new int[nVertices];
        memset(newIndex, 0xFF, nVertices * sizeof(int));
        unsigned short tempIdx = 0;

        for (int i = 0; i < nVectors; i++)
        {
            if (wmoGroup->isCollidable(i))
            {
                // Use this triangle.
                for (int j = 0; j < 3; j++)
                {
                    tempIdx = vertexIndices->indexes[3 * i + j];
                    newIndex[tempIdx] = 1;
                    moviEx[3 * nColTriangles + j] = tempIdx;
                }
                nColTriangles++;
            }
        }

        // Assign new vertex index numbers
        for (int i = 0; i < nVertices; i++)
        {
            if (newIndex[i] == 1)
            {
                newIndex[i] = nColVertices;
                nColVertices++;
            }
        }

        file->header.nVectors += nColTriangles;

        group->indices.nIndices = nColTriangles * 3;
        group->indices.size = sizeof(unsigned int) + (sizeof(unsigned short) * group->indices.nIndices);
        group->indices.indices = new unsigned short[group->indices.nIndices];

        // Translate triangle indices to new numbers
        for (int i = 0; i < group->indices.nIndices; i++)
        {
            group->indices.indices[i] = (unsigned short) newIndex[moviEx[i]];
        }

        group->vertices.nVertices = nColVertices;
        group->vertices.size = sizeof(unsigned int) + (sizeof(ModelFile::VmapGroup::Vertices::Vertex) * nColVertices);
        group->vertices.vertices = new ModelFile::VmapGroup::Vertices::Vertex[group->vertices.nVertices];
        int k = 0;

        for (int i = 0; i < nVertices; i++)
        {
            if (newIndex[i] >= 0)
            {
                group->vertices.vertices[k].x = vertexInfo->vertices[i].x;
                group->vertices.vertices[k].y = vertexInfo->vertices[i].y;
                group->vertices.vertices[k].z = vertexInfo->vertices[i].z;
                k++;
            }
        }

        if (k != nColVertices)
        {
            _logger.error("Bad collision vertex count for model %s", file->getFilename());
        }

        delete[] newIndex;
        delete[] moviEx;
    }

    if (wmoGroup->hasLiquid())
    {
        MLIQ* liquidInfo = wmoGroup->getLiquidInfo();
        LiquidVert* liquidVertices = wmoGroup->getLiquidVertices();
        group->liquidFlags |= 1;
        group->liquid.size = sizeof(MLIQ::Header) - 2 + (sizeof(float) * liquidInfo->header.xVerts * liquidInfo->header.yVerts) + (liquidInfo->header.xTiles * liquidInfo->header.yTiles);
        group->liquid.xVerts = liquidInfo->header.xVerts;
        group->liquid.yVerts = liquidInfo->header.yVerts;
        group->liquid.xTiles = liquidInfo->header.xTiles;
        group->liquid.yTiles = liquidInfo->header.yTiles;
        group->liquid.baseCoords.x = liquidInfo->header.baseCoords.x;
        group->liquid.baseCoords.y = liquidInfo->header.baseCoords.y;
        group->liquid.baseCoords.z = liquidInfo->header.baseCoords.z;

        unsigned int liquidVertexSize = group->liquid.xVerts * group->liquid.yVerts;
        unsigned int liquidFlagsSize = group->liquid.xTiles * group->liquid.yTiles;

        if (root->useLiquidTypeFromDBC())
        {
            group->liquid.type = info->groupLiquid;
        }
        else {
            // Trying to determine the liquid type by parsing the flag tiles. The first one having a type wins.
            for (int i = 0; i < liquidFlagsSize; i++)
            {
                if (wmoGroup->tileIsWater(i))
                {
                    group->liquid.type = IS_WATER + 1;
                    break;
                }
                else if (wmoGroup->tileIsOcean(i))
                {
                    group->liquid.type = IS_OCEAN + 1;
                    break;
                }
                else if (wmoGroup->tileIsMagma(i))
                {
                    group->liquid.type = IS_MAGMA + 1;
                    break;
                }
                else if (wmoGroup->tileIsSlime(i))
                {
                    if (root->getWMOID() == 4489) // Naxxramas
                    {
                        group->liquid.type = 21;
                    }
                    else 
                    {
                        group->liquid.type = IS_SLIME + 1;
                    }
                    break;
                }
            }
        }


        group->liquid.height = new float[liquidVertexSize];
        for (int i = 0; i < liquidVertexSize; i++)
        {
            group->liquid.height[i] = liquidVertices[i].height;
        }
        group->liquid.flags = new unsigned char[liquidFlagsSize];
        memcpy(group->liquid.flags, wmoGroup->getLiquidFlags(), liquidFlagsSize);
    }

    return true;
}

void ExtractorClassic::exportModels(std::string outputPath, bool cacheToDisk)
{
    _logger.information("Extracting Models...");

    Path path(outputPath);

    std::vector<std::string> models = _mpqManager->getModelsList();
    Path temp;
    std::string flatName;
    std::string hash;
    MD5Engine engine;
    DigestOutputStream ds(engine);
    unsigned int count = 0;
    unsigned int total = (unsigned int) models.size();

    for (auto it = models.begin(); it < models.end(); ++it)
    {
        _logger.debug("Extracting model %s", *it);
        temp.assign(*it, Path::Style::PATH_WINDOWS); // Force path to interpret it with '\' as separator.
        ds << temp.parent().toString();
        hash = DigestEngine::digestToHex(engine.digest());
        ds.clear();
        M2V1* model = (M2V1*)_mpqManager->getFile(*it, _version);

        if (!model)
        {
            _logger.warning("Model file does not exist %s, warning can be safely ignored but no vmap info will be generated", *it);
            continue;
        }

        if (!model->parse())
        {
            _logger.error("Error while parsing the Model file %s ", *it);
            delete model;
            continue;
        }

        ModelFile* modelFile = new ModelFile(temp.parent().toString(), hash, temp.getFileName());

        convertModel(model, modelFile);

        if (cacheToDisk)
        {
            modelFile->save(path.toString());
            delete modelFile;
        }

        // We use printf here because there's no known way to do it with Poco.
        count++;
        printf(" Processing........................%d%%\r", (100 * (count + 1)) / total);

        delete model;
    }

    ds.close();
}

bool ExtractorClassic::convertModel(M2V1* model, ModelFile* file)
{
    strncpy(file->header.versionMagic, "z07\0", 4);
    file->header.nGroups = 1;
    file->header.rootWMOID = 0;
    file->groups = new ModelFile::VmapGroup[1];
    ModelFile::VmapGroup* group = &(file->groups[0]);
    unsigned int tmp;

    file->header.nVectors = model->getNCollisionVertices();
    group->flags = 0;
    group->groupWMOID = 0;
    group->boundingBox.min.x = 0;
    group->boundingBox.min.y = 0;
    group->boundingBox.min.z = 0;
    group->boundingBox.max.x = 0;
    group->boundingBox.max.y = 0;
    group->boundingBox.max.z = 0;
    group->liquidFlags = 0;

    // Header
    group->header.mobaSize = sizeof(unsigned int) + sizeof(unsigned int);
    group->header.mobaBatch = 1;
    group->header.mobaEx = new unsigned int[group->header.mobaBatch];
    group->header.mobaEx[0] = model->getNCollisionTriangles();

    // Use collision triangles.

    // Vertices
    group->vertices.nVertices = model->getNCollisionVertices();
    group->vertices.size = sizeof(unsigned int) + (sizeof(ModelFile::VmapGroup::Vertices::Vertex) * group->vertices.nVertices);
    group->vertices.vertices = new ModelFile::VmapGroup::Vertices::Vertex[group->vertices.nVertices];
    memcpy(group->vertices.vertices, model->getCollisionVertices(), group->vertices.nVertices);

    // Indices
    group->indices.nIndices = model->getNCollisionTriangles();
    group->indices.size = sizeof(unsigned int) + sizeof(unsigned short) * group->indices.nIndices;
    group->indices.indices = new unsigned short[group->indices.nIndices];
    memcpy(group->indices.indices, model->getCollisionTriangles(), group->indices.nIndices);

    for (int i = 0; i < group->indices.nIndices; i++)
    {
        if (!((i % 3) - 1))
        {
            tmp = group->indices.indices[i];
            group->indices.indices[i] = group->indices.indices[i + 1];
            group->indices.indices[i + 1] = tmp;
        }
    }

    return true;
}