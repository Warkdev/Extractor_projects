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
#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <string>
#include <set>
#include "Poco/Logger.h"
#include "Poco/Util/LayeredConfiguration.h"
#include "MPQManager.h"
#include "../maps/MapFile.h"
#include "../vmaps/ModelFile.h"
#include "../vmaps/Model.h"
#include "../vmaps/VMTreeFile.h"
#include "../vmaps/VMTileFile.h"
#include "../vmaps/VMOFile.h"

using Poco::Logger;
using Poco::Util::LayeredConfiguration;

// General extractor flags
static const std::string FLAG_EXPORT_PATH = "extractor.output.path";
static const std::string FLAG_CLIENT_PATH = "extractor.client.path";
static const std::string FLAG_EXTRACT_DBCS = "extractor.extract.dbcs";
static const std::string FLAG_EXPORT_MAPS = "extractor.export.map";
static const std::string FLAG_EXPORT_VMAPS = "extractor.generate.vmap";
static const std::string FLAG_EXPORT_MMAPS = "extractor.generate.mmap";
static const std::string FLAG_SKIP_CONTINENTS = "extractor.skip.continents";
static const std::string FLAG_SKIP_JUNK_MAPS = "extractor.skip.junkmaps";
static const std::string FLAG_SKIP_BATTLEGROUNDS = "extractor.skip.battlegrounds";

// Map files versions.
static const std::string MAP_FILE_VERSION_CLASSIC = "z1.5";
static const std::string MAP_FILE_VERSION_TBC = "s1.5";
static const std::string MAP_FILE_VERSION_WOLK = "v1.5";
static const std::string MAP_FILE_VERSION_CATA = "c1.5";
static const std::string MAP_FILE_VERSION_MOP = "p1.5";
static const std::string MAP_FILE_VERSION_WOD = "w1.5";
static const std::string MAP_FILE_VERSION_LEGION = "l1.5";

// Client build numbers
static const unsigned int CLIENT_BUILD_CLASSIC = 5875;

// Mmap files versions.
static const std::string MMAP_FILE_VERSION_CLASSIC = "z1.5";
static const std::string MMAP_FILE_VERSION_TBC = "s1.5";
static const std::string MMAP_FILE_VERSION_WOLK = "v1.5";
static const std::string MMAP_FILE_VERSION_CATA = "c1.5";
static const std::string MMAP_FILE_VERSION_MOP = "p1.5";
static const std::string MMAP_FILE_VERSION_WOD = "w1.5";
static const std::string MMAP_FILE_VERSION_LEGION = "l1.5";

template<> struct BoundsTrait<ModelInstance*>
{
	static void getBounds(const ModelInstance* const& model, AABox& out) { out = model->boundingBox; }
};

class Extractor 
{
	public:
		
		Extractor(LayeredConfiguration* config)
		{
			_mpqManager = new MPQManager();
			_version = Version::CLIENT_UNKNOWN;
			_extractDbcs = config->getBool(FLAG_EXTRACT_DBCS);
			_exportMaps = config->getBool(FLAG_EXPORT_MAPS);
			_generateVmaps = config->getBool(FLAG_EXPORT_VMAPS);
			_generateMmaps = config->getBool(FLAG_EXPORT_MMAPS);
			try {
				_skipContinents = config->getBool(FLAG_SKIP_CONTINENTS);
			}
			catch (...) {
				_skipContinents = false;
			}
			try {
				_skipJunkMaps = config->getBool(FLAG_SKIP_JUNK_MAPS);
			}
			catch (...) {
				_skipJunkMaps = true;
			}
			try {
				_skipBattlegrounds = config->getBool(FLAG_SKIP_BATTLEGROUNDS);
			}
			catch (...) {
				_skipBattlegrounds = false;
			}
			_outputDBCPath = config->getString(FLAG_EXPORT_PATH) + PATH_DBC;
			_outputMapPath = config->getString(FLAG_EXPORT_PATH) + PATH_MAPS;
			_outputVmapPath = config->getString(FLAG_EXPORT_PATH) + PATH_VMAPS;
			_outputMmapPath = config->getString(FLAG_EXPORT_PATH) + PATH_MMAPS;
			_clientDataPath = config->getString(FLAG_CLIENT_PATH) + PATH_DATA;
			_allowHeightLimit = config->getBool(PROP_ALLOW_HEIGHT_LIMIT);
			_useMinHeight = (float)config->getDouble(PROP_USE_MIN_HEIGHT);
			_allowFloatToInt = config->getBool(PROP_ALLOW_FLOAT_TO_INT);
			_flatHeightDeltaLimit = (float)config->getDouble(PROP_FLAT_HEIGHT_DELTA_LIMIT);
			_flatLiquidDeltaLimit = (float)config->getDouble(PROP_FLAT_LIQUID_DELTA_LIMIT);
			_floatToByteLimit = (float)config->getDouble(PROP_FLOAT_TO_BYTE_LIMIT);
			_floatToShortLimit = (float)config->getDouble(PROP_FLOAT_TO_SHORT_LIMIT);
			_preciseVectorData = config->getBool(PROP_VMAP_WMO_PRECISE_VECTOR_DATA);
		}

		~Extractor()
		{
			delete _mpqManager;
			for (auto it = _models.begin(); it != _models.end(); it++)
			{
				delete it->second;
			}

			for (auto it = _worldModels.begin(); it != _worldModels.end(); it++)
			{
				delete it->second;
			}
			for (auto it = _modelInstances.begin(); it != _modelInstances.end(); it++)
			{
				delete it->second;
			}
			for (auto it = _modelTileInstances.begin(); it != _modelTileInstances.end(); it++)
			{
				delete it->second;
			}
		}

		virtual void init() = 0;
		void loadMPQs();
		void exportDBC();
		virtual void exportMaps() = 0;
	protected:
		virtual void createDirectories() = 0;

		void readMaps();
		void readAreaTable();
		void readLiquidType();

		// Tools function
		std::string getUniformName(std::string filename);
		unsigned int packTileId(unsigned int x, unsigned int y);
		virtual bool isContinent(unsigned int mapId) = 0;
		virtual bool isJunkMap(unsigned int mapId) = 0;
		virtual bool isBattleground(unsigned int mapId) = 0;

		// Map Functions
		void packAreaData(MapFile* map);
		void packHeight(MapFile* map);
		void packLiquid(MapFile* map);
		void packHoles(MapFile* map);
		void packData(MapFile* map);

		// VMap Functions
		virtual void spawnVMap(unsigned int mapId, MPQFile* wdt) = 0;
		virtual void readModels(MPQFile* adt) = 0;
		virtual void readWorldModels(MPQFile* adt) = 0;
		virtual void spawnModelInstances(MPQFile* adt, MCNK* cell, unsigned int x, unsigned int y) = 0;
		virtual void spawnWorldModelInstances(MPQFile* adt, MCNK* cell, unsigned int x, unsigned int y) = 0;
		virtual void saveVMap(unsigned int mapId);
		virtual void saveVMapTile(unsigned int mapId, unsigned int x, unsigned int y);
		virtual void saveVMapModels(Model* model);

		// Extractor flags.
		bool _extractDbcs;
		bool _exportMaps;
		bool _generateVmaps;
		bool _generateMmaps;
		bool _skipContinents;
		bool _skipJunkMaps;
		bool _skipBattlegrounds;

		// Client Path.
		Path _clientDataPath;
		const std::string PATH_DATA = "/Data";

		// DBC Extractor properties.
		Path _outputDBCPath;
		const std::string PATH_DBC = "/dbc";
		const std::string DBC_MAPS = "DBFilesClient\\Map.dbc";
		const std::string DBC_AREATABLE = "DBFilesClient\\AreaTable.dbc";
		const std::string DBC_LIQUIDTYPE = "DBFilesClient\\LiquidType.dbc";

		// Map Extractor Configuration keys.
		const std::string PROP_ALLOW_HEIGHT_LIMIT = "map.allowHeightLimit";
		const std::string PROP_USE_MIN_HEIGHT = "map.useMinHeight";
		const std::string PROP_ALLOW_FLOAT_TO_INT = "map.allowFloatToInt";
		const std::string PROP_FLOAT_TO_BYTE_LIMIT = "map.floatToByteLimit";
		const std::string PROP_FLOAT_TO_SHORT_LIMIT = "map.floatToShortLimit";
		const std::string PROP_FLAT_HEIGHT_DELTA_LIMIT = "map.flatHeightDeltaLimit";
		const std::string PROP_FLAT_LIQUID_DELTA_LIMIT = "map.flatLiquidDeltaLimit";
		// Map Extractor properties.
		const std::string PATH_MAPS = "/maps";
		Path _outputMapPath;
		bool _allowHeightLimit;
		float _useMinHeight;
		bool _allowFloatToInt;
		float _floatToByteLimit;
		float _floatToShortLimit;
		float _flatHeightDeltaLimit;
		float _flatLiquidDeltaLimit;
		// Map file info
		std::string _mapVersion;
		unsigned int _buildVersion;

		// Vmap Generator Configuration keys.
		const std::string PROP_VMAP_WMO_PRECISE_VECTOR_DATA = "vmap.wmo.preciseVectorData";
		bool _preciseVectorData;
		const std::string PATH_VMAPS = "/vmaps";
		Path _outputVmapPath;

		// Mmap Generator Configuration keys.
		const std::string PATH_MMAPS = "/mmaps";
		Path _outputMmapPath;

		Logger& _logger = Logger::get("Extractor");
		Version _version;

		// MPQ Archive data
		std::vector<std::string> _listMPQ;
		MPQManager* _mpqManager;

		// Maps data
		std::map<unsigned int, std::string> _maps;
		std::map<unsigned int, unsigned int> _areas;
		std::map<unsigned int, unsigned int> _liquids;

		// VMaps data
		std::map<std::string, Model*> _models; // Hold the models info for the current WDT.
		std::map<std::string, Model*> _worldModels; // Hold the world models info for the current WDT.
		ModelInstance* _worldModel; // Hold a global world model for the current WDT.
		std::vector<std::string> _modelsList; // Hold the list of models for the current ADT.
		std::vector<std::string> _worldModelsList; // Hold the list of world models for the current ADT.
		std::map<unsigned int, ModelInstance*> _modelInstances; // Hold the list of spawned models for the current WDT.
		std::map<unsigned int, ModelInstance*> _modelTileInstances; // Hold the list of spawned models for the current Tile.

		// MMaps file info;
		std::string _mmapVersion;
};

#endif // !EXTRACTOR_H