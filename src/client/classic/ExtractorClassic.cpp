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
#include "../mpq/WDT.h"

using Poco::Path;
using Poco::File;

void ExtractorClassic::init()
{
	_logger.information("Initializing ExtractorClassic..");
    // This if the complete MPQ list but some of them have no interesting or recent data in them. Save time by not loading them.
	//std::string mpqs[] = { "base.MPQ", "dbc.MPQ", "misc.MPQ", "model.MPQ", "speech.MPQ", "terrain.MPQ", "texture.MPQ", "wmo.MPQ", "patch.MPQ", "patch-2.MPQ" };
    std::string mpqs[] = { "dbc.MPQ", "model.MPQ", "terrain.MPQ", "wmo.MPQ", "patch.MPQ", "patch-2.MPQ" };
	for (int i = 0; i < (sizeof(mpqs) / sizeof(mpqs[0])); i++)
	{
		Path path(_clientDataPath, mpqs[i]);
		_listMPQ.push_back(path.toString());
	}
	_version = Version::CLIENT_CLASSIC;
}

void ExtractorClassic::createDirectories()
{
    if (_exportMaps) {
        File file(_outputMapPath);
        file.createDirectories();
    }

    if (_generateVmaps) {
        File file(_outputVmapPath);
        file.createDirectories();
    }

    if (_generateMmaps) {
        File file(_outputMmapPath);
        file.createDirectories();
    }
}

bool ExtractorClassic::isContinent(unsigned int mapId)
{
    switch (mapId)
    {
        case 0:
        case 1:
            return true;
    }

    return false;
}

bool ExtractorClassic::isJunkMap(unsigned int mapId)
{
    switch (mapId)
    {
        case 13:
        case 25:
        case 29:
        case 35:
        case 37:
        case 42:
        case 44:
        case 169:
        case 451:
            return true;
    }

    return false;
}

bool ExtractorClassic::isBattleground(unsigned int mapId)
{
    switch (mapId)
    {
        case 30:    // AV
        case 489:   // WSG
        case 529:   // AB
            return true;
    }

    return false;
}

void ExtractorClassic::exportMaps()
{
    system("pause");
    _logger.information("Extracting Maps..");
    createDirectories();

	char wdtFile[1024];
	char adtFile[1024];

    readMaps();
    readAreaTable();

    unsigned int maxAreaId = _areas.rbegin()->first;

	for (auto it = _maps.begin(); it != _maps.end(); it++)
	{
        if ((_skipContinents && isContinent(it->first)) || (_skipJunkMaps && isJunkMap(it->first)) || (_skipBattlegrounds && isBattleground(it->first)))
        {
            _logger.information("Skipping WDT file %s.wdt (%u)", it->second, it->first);
            continue;
        }
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
		
        if (_generateVmaps)
        {
            spawnVMap(it->first, wdt);
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

                    if (_generateVmaps) 
                    {
                        // We read the models defined in the ADT. Chunk MDDX and load them as Model with their graphical data.
                        readModels(adt);

                        // Same for WMO's.
                        readWorldModels(adt);
                    }

                    if (!parseMap(adt, it->first, x, y, maxAreaId))
                    {
                        _logger.error("Error while generating the map file");
                        delete adt;
                        continue;
                    }

					delete adt;
				}
                // We use printf here because there's no known way to do it with Poco.
                printf(" Processing tile X: %u, Y: %u........................%d%%\r", x, y, (100 * (x + 1)) / WDT::MAP_TILE_SIZE);
			}
		}

        if (_generateVmaps)
        {
            saveVMap(it->first);
        }
		delete wdt;
	}
    _logger.information("");
    _logger.information("Asset extraction complete! Good game :-)");
}

bool ExtractorClassic::parseMap(ADTV1* adt, unsigned int mapId, unsigned int x, unsigned int y, unsigned int maxAreaId)
{
    MapFile* map;
    if (_exportMaps)
    {
        map = new MapFile(mapId, x, y);
        map->mapHeightHeader.gridMaxHeight = -200000;
        map->mapHeightHeader.gridHeight = 20000;
        map->heightStep = 0.0f;
    }
 
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

            if (_exportMaps)
            {
                handleAreas(map, cell, i, j, maxAreaId);
                handleHeight(map, adt, cell, i, j);
                handleLiquid(map, adt, cell, i, j);
                handleHoles(map, cell, i, j);
            }

            if (_generateVmaps)
            {
                // We spawn models and calculate their position.
                spawnModelInstances(adt, cell, x, y);
                // .. also for World Models.
                spawnWorldModelInstances(adt, cell, x, y);
            }
        }
    }

    if (_exportMaps)
    {
        packData(map);

        if (!map->save(_outputMapPath.toString()))
        {
            _logger.error("Error while saving the map file");
        }

        delete map;
    }

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

void ExtractorClassic::handleHeight(MapFile* map, ADTV1* adt, MCNK* cell, unsigned int i, unsigned int j)
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
                if (_allowHeightLimit)
                {
                    if (map->V8[cy][cx] < _useMinHeight)
                    {
                        map->V8[cy][cx] = _useMinHeight;
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

            if (_allowHeightLimit)
            {
                if (map->V9[cy][cx] < _useMinHeight)
                {
                    map->V9[cy][cx] = _useMinHeight;
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

void ExtractorClassic::spawnVMap(unsigned int mapId, MPQFile* wdt)
{
    Path tempPath;
    _logger.information("VMAP - Spawning map %u", mapId);
    char wmoGroupFile[1024];

    // We read first global WMO's.
    if (((WDT*)wdt)->hasGlobalWMO())
    {
        std::string wmoName = ((WDT*)wdt)->getGlobalWMOName();
        _logger.debug("Global WMO: %s", wmoName);

        WMOV1* wmo = (WMOV1*)_mpqManager->getFile(wmoName, _version);
        WMOGroupV1** groups;

        if (!wmo)
        {
            _logger.error("WMO file %s doesn't exist", wmoName);
            return;
        }

        if (!wmo->parse())
        {
            _logger.error("Error while parsing the wmo %s", wmoName);
            delete wmo;
            return;
        }

        groups = new WMOGroupV1 * [wmo->getNGroups()];

        for (int i = 0; i < wmo->getNGroups(); i++) {
            tempPath.assign(wmoName, Path::Style::PATH_WINDOWS); // Force path to interpret it with '\' as separator.
            sprintf(wmoGroupFile, "%s%s_%03d.wmo", tempPath.parent().toString().c_str(), tempPath.getBaseName().c_str(), i);
            WMOGroupV1* group = (WMOGroupV1*)_mpqManager->getFile(wmoGroupFile, _version);

            if (!group)
            {
                _logger.warning("WMO Group file (%s) does not exist, warning can be safely ignored but no vmap info will be generated", std::string(wmoGroupFile));
                continue;
            }

            if (!group->parse())
            {
                _logger.error("Error while parsing the WMO Group file %s", std::string(wmoGroupFile));
                delete group;
                continue;
            }

            groups[i] = group;
        }

        _worldModels[wmoName] = new Model(getUniformName(tempPath.parent().toString()) + '-' + tempPath.getFileName(), wmo, groups, _preciseVectorData);
        _worldModels[wmoName]->flags |= MOD_WORLDSPAWN;

        MODF::MapObjDef* placement = &((WDT*)wdt)->getGlobalWMOPlacement()->placements[0];

        ModelInstance* instance = new ModelInstance(_worldModels[wmoName], placement->uniqueId, placement->flags, 0, 0, placement->position, placement->orientation, 1.0f, placement->boundingBox);
        _worldModel = instance;
        saveVMapModels(_worldModels[wmoName]);

        delete[] groups;
    }
    else 
    {
        _worldModel = NULL;
    }
}

void ExtractorClassic::readModels(MPQFile* adt)
{
    Path tempPath;
    std::string m2Name;
    _modelsList = ((ADTV1*) adt)->getModels();
    unsigned int idx = 0;
    for (auto it = _modelsList.begin(); it < _modelsList.end(); it++)
    {
        m2Name = (*it).substr(0, (*it).find_last_of(".")) + ".m2";
        if (!_models.count(m2Name))
        {
            M2V1* m2 = (M2V1*)_mpqManager->getFile(m2Name, _version);
            tempPath.assign(m2Name, Path::Style::PATH_WINDOWS); // Force path to interpret it with '\' as separator.

            if (!m2)
            {
                _logger.error("Model file %s doesn't exist", m2Name);
                continue;
            }

            if (!m2->parse())
            {
                _logger.error("Error while parsing the model %s", m2Name);
                delete m2;
                continue;
            }
            if (!m2->getNCollisionVertices())
            {
                _logger.debug("Model has no collision vertices, skipping %s", m2Name);
                delete m2;
                continue;
            }
            _models[*it] = new Model(getUniformName(tempPath.parent().toString()) + '-' + tempPath.getFileName(), m2);
            saveVMapModels(_models[*it]);
        }
    }
}

void ExtractorClassic::readWorldModels(MPQFile* adt)
{
    Path tempPath;
    char wmoGroupFile[1024];
    _worldModelsList = ((ADTV1*) adt)->getWorldModels();
    for (auto itWMO = _worldModelsList.begin(); itWMO < _worldModelsList.end(); itWMO++)
    {
        if (!_worldModels.count(*itWMO))
        {
            WMOV1* wmo = (WMOV1*)_mpqManager->getFile(*itWMO, _version);
            WMOGroupV1** groups;

            if (!wmo)
            {
                _logger.error("WMO file %s doesn't exist", *itWMO);
                continue;
            }

            if (!wmo->parse())
            {
                _logger.error("Error while parsing the wmo %s", *itWMO);
                delete wmo;
                continue;
            }

            groups = new WMOGroupV1 * [wmo->getNGroups()];

            for (int i = 0; i < wmo->getNGroups(); i++) {
                tempPath.assign(*itWMO, Path::Style::PATH_WINDOWS); // Force path to interpret it with '\' as separator.
                sprintf(wmoGroupFile, "%s%s_%03d.wmo", tempPath.parent().toString().c_str(), tempPath.getBaseName().c_str(), i);
                WMOGroupV1* group = (WMOGroupV1*)_mpqManager->getFile(wmoGroupFile, _version);

                if (!group)
                {
                    _logger.warning("WMO Group file (%s) does not exist, warning can be safely ignored but no vmap info will be generated", std::string(wmoGroupFile));
                    continue;
                }

                if (!group->parse())
                {
                    _logger.error("Error while parsing the WMO Group file %s", std::string(wmoGroupFile));
                    delete group;
                    continue;
                }

                groups[i] = group;
            }

            _worldModels[*itWMO] = new Model(getUniformName(tempPath.parent().toString()) + '-' + tempPath.getFileName(), wmo, groups, _preciseVectorData);
            saveVMapModels(_worldModels[*itWMO]);
            delete[] groups;
        }
    }
}

void ExtractorClassic::spawnModelInstances(MPQFile* adt, MCNK* cell, unsigned int x, unsigned int y)
{
    MCRF* references = ((ADTV1*) adt)->getModelReferences(cell);
    if (cell->nDoodadRefs)
    {
        // Here, we get each instances of previously loaded models.
        unsigned int* modelsOffsets = (unsigned int*)((unsigned char*)references + sizeof(MCRF::magic) + sizeof(MCRF::size));
        MDDF* modelPlacement = ((ADTV1*)adt)->getModelsPlacement();
        for (int k = 0; k < cell->nDoodadRefs; k++)
        {
            MDDF::DoodadDef* placement = &modelPlacement->placements[modelsOffsets[k]];
            Model* m2;
            Path tempPath(_modelsList[placement->mmidEntry], Path::Style::PATH_WINDOWS);
            auto it = _models.find(_modelsList[placement->mmidEntry]);
            if (it != _models.end())
            {
                // Model found
                m2 = it->second;
                if(!_modelInstances.count(placement->uniqueId)) {
                    ModelInstance* instance = new ModelInstance(m2, placement->uniqueId, 0, x, y, placement->position, placement->orientation, placement->scale, m2->groups[0].boundingBox);
                    _modelInstances[instance->id] = instance;
                }
                _modelTileInstances[packTileId(x, y)][placement->uniqueId] = _modelInstances[placement->uniqueId];
            }
            else {
                // Can be a model without collision vertices.
                _logger.debug("Model not found %s", _modelsList[placement->mmidEntry]);
            }
        }
    }
}

void ExtractorClassic::spawnWorldModelInstances(MPQFile* adt, MCNK* cell, unsigned int x, unsigned int y)
{
    MCRF* references = ((ADTV1*)adt)->getModelReferences(cell);
    if (cell->nMapObjRefs)
    {
        unsigned int* wmoOffsets = (unsigned int*)((unsigned char*)references + sizeof(MCRF::magic) + sizeof(MCRF::size) + (cell->nDoodadRefs * sizeof(unsigned int)));
        MODF* wmoPlacement = ((ADTV1*) adt)->getWorldModelsPlacement();
        for (int k = 0; k < cell->nMapObjRefs; k++)
        {
            MODF::MapObjDef* placement = &wmoPlacement->placements[wmoOffsets[k]];
            Model* wmo;
            Path tempPath(_worldModelsList[placement->mwidEntry], Path::Style::PATH_WINDOWS);
            auto it = _worldModels.find(_worldModelsList[placement->mwidEntry]);
            if (it != _worldModels.end())
            {
                // Model found
                wmo = it->second;
                if (!_modelInstances.count(placement->uniqueId)) {
                    ModelInstance* instance = new ModelInstance(wmo, placement->uniqueId, placement->flags, x, y, placement->position, placement->orientation, 1.0f, placement->boundingBox);
                    _modelInstances[instance->id] = instance;
                }
                _modelTileInstances[packTileId(x, y)][placement->uniqueId] = _modelInstances[placement->uniqueId];
            }
            else {
                _logger.warning("WMO not found %s", _worldModelsList[placement->mwidEntry]);
            }
        }
    }
}