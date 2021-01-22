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

#include <string>
#include "../Extractor.h"
#include "../mpq/ADTV1.h"
#include "../mpq/WMOV1.h"
#include "../mpq/M2V1.h"
#include "Poco/Util/LayeredConfiguration.h"

using Poco::Util::LayeredConfiguration;

class ExtractorClassic : public Extractor
{
	public:
		ExtractorClassic(LayeredConfiguration* config)
		{
			_config = config;
		}
		virtual void init(std::string clientPath);
		virtual void extract(std::string outputPath, bool exportMap, bool generateVmaps);
	protected:
		virtual void exportMaps(std::string outputPath);
		virtual void exportWMOs(std::string outputPath, bool cacheToDisk);
		virtual void exportModels(std::string outputPath, bool cacheToDisk);
		bool convertADT(ADTV1* adt, MapFile* map, unsigned int maxAreaId, bool allowHeightLimit, bool allowFloatToInt, float floatHeightDeltaLimit, float floatLiquidDeltaLimit, float floatToByteLimit, float floatToShortLimit, float useMinHeight);
		void handleAreas(MapFile* map, MCNK* cell, unsigned int i, unsigned int j, unsigned int maxAreaId);
		void handleHeight(MapFile* map, ADTV1* adt, MCNK* cell, unsigned int i, unsigned int j, bool allowHeightLimit, float useMinHeight);
		void handleLiquid(MapFile* map, ADTV1* adt, MCNK* cell, unsigned int i, unsigned int j);
		void handleHoles(MapFile* map, MCNK* cell, unsigned int i, unsigned int j);

		bool convertWMORoot(WMOV1* wmo, ModelFile* file);
		bool convertWMOGroup(WMOV1* root, WMOGroupV1* wmoGroup, ModelFile* file, unsigned int groupIdx, bool preciseVectorData);
		bool convertModel(M2V1* model, ModelFile* file, bool preciseVectorData);
		
	private:
		const std::string PATTERN_WDT = "World\\Maps\\%s\\%s.wdt";
		const std::string PATTERN_ADT = "World\\Maps\\%s\\%s_%i_%i.adt";
		//std::string _listMPQ[11] = { "base.MPQ", "dbc.MPQ", "misc.MPQ", "model.MPQ", "sound.MPQ", "speech.MPQ", "terrain.MPQ", "texture.MPQ", "wmo.MPQ", "patch.MPQ", "patch-2.MPQ" };
};