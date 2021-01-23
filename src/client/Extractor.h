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
#include "Poco/Logger.h"
#include "Poco/Util/LayeredConfiguration.h"
#include "MPQManager.h"
#include "../maps/MapFile.h"
#include "../vmaps/ModelFile.h"

using Poco::Logger;
using Poco::Util::LayeredConfiguration;

class Extractor 
{
	public:
		Extractor()
		{
			_mpqManager = new MPQManager();
			_version = Version::CLIENT_UNKNOWN;
			_config = NULL;
		}
		Extractor(LayeredConfiguration* configuration)
		{
			_mpqManager = new MPQManager();
			_version = Version::CLIENT_UNKNOWN;
			_config = configuration;
		}

		~Extractor()
		{
			delete _mpqManager;
		}

		virtual void init(std::string clientPath);
		void loadMPQs();
		void exportDBC(std::string outputPath);
		virtual void extract(std::string outputPath, bool exportMap, bool generateVmaps);
	protected:
		void readMaps();
		void readAreaTable();
		void readLiquidType();
		virtual void exportMaps(std::string outputPath);
		virtual void exportWMOs(std::string outputPath, bool cacheToDisk);
		virtual void exportModels(std::string outputPath, bool cacheToDisk);

		void packAreaData(MapFile* map);
		void packHeight(MapFile* map, bool allowFloatToInt, float floatHeightDeltaLimit, float floatToByteLimit, float floatToShortLimit);
		void packLiquid(MapFile* map, bool allowFloatToInt, float floatLiquidDeltaLimit, float useMinHeight);
		void packHoles(MapFile* map);
		void packData(char* version, unsigned int build, MapFile* map, bool allowFloatToInt, float floatHeightDeltaLimit, float floatLiquidDeltaLimit, float floatToByteLimit, float floatToShortLimit, float useMinHeight);

		// Map Extractor Configuration keys.
		const std::string PROP_ALLOW_HEIGHT_LIMIT = "map.allowHeightLimit";
		const std::string PROP_USE_MIN_HEIGHT = "map.useMinHeight";
		const std::string PROP_ALLOW_FLOAT_TO_INT = "map.allowFloatToInt";
		const std::string PROP_FLOAT_TO_BYTE_LIMIT = "map.floatToByteLimit";
		const std::string PROP_FLOAT_TO_SHORT_LIMIT = "map.floatToShortLimit";
		const std::string PROP_FLAT_HEIGHT_DELTA_LIMIT = "map.flatHeightDeltaLimit";
		const std::string PROP_FLAT_LIQUID_DELTA_LIMIT = "map.flatLiquidDeltaLimit";

		// Vmap Generator Configuration keys.
		const std::string PROP_VMAP_CACHE_TO_DISK = "vmap.cache.disk";
		const std::string PROP_VMAP_WMO_PRECISE_VECTOR_DATA = "vmap.wmo.preciseVectorData";

		Logger& _logger = Logger::get("Extractor");
		LayeredConfiguration* _config;
		Version _version;
		const std::string PATH_DBC = "/dbc";
		const std::string PATH_MAPS = "/maps";
		const std::string PATH_MODELS = "/models";
		const std::string DBC_MAPS = "DBFilesClient\\Map.dbc";
		const std::string DBC_AREATABLE = "DBFilesClient\\AreaTable.dbc";
		const std::string DBC_LIQUIDTYPE = "DBFilesClient\\LiquidType.dbc";
		std::vector<std::string> _listMPQ;
		MPQManager* _mpqManager;

		std::vector<std::string> _worldModels;
		std::vector<std::string> _models;

		std::map<unsigned int, std::string> _maps;
		std::map<unsigned int, unsigned int> _areas;
		std::map<unsigned int, unsigned int> _liquids;
};

#endif // !EXTRACTOR_H