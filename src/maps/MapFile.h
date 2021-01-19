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

#ifndef MAPFILE_H
#define MAPFILE_H

#include "Poco/Logger.h"
#include "Poco/Util/LayeredConfiguration.h"
#include "../client/mpq/ADTV1.h"

using Poco::Logger;
using Poco::Util::LayeredConfiguration;

class MapFile {
	public:
		MapFile(unsigned int mapId, unsigned int x, unsigned int y);
		~MapFile();

		bool save(std::string path);

		unsigned short areaFlags[ADTV1::SIZE_TILE_MAP][ADTV1::SIZE_TILE_MAP];
		float V8[ADTV1::SIZE_ADT_GRID][ADTV1::SIZE_ADT_GRID];
		float V9[ADTV1::SIZE_ADT_GRID + 1][ADTV1::SIZE_ADT_GRID + 1];
		float heightStep;

		/** Map File Header */
		struct MapHeader {
			const char magic[4] = { 'M', 'A', 'P', 'S' };
			char versionMagic[4]; // "z1.5" for classic, "s1.5" for tbc, "v1.5" for wotlk, "c1.5" for cata, "p1.5" for mop, "w1.5" for wod, "l1.5" for legion.
			unsigned int buildMagic;
			unsigned int areaMapOffset;
			unsigned int areaMapSize;
			unsigned int heightMapOffset;
			unsigned int heightMapSize;
			unsigned int liquidMapOffset;
			unsigned int liquidMapSize;
			unsigned int holesOffset;
			unsigned int holesSize;
		} header;
		// Map Area Flags
		static const unsigned int MAP_AREA_NO_AREA = 0x0001;
		/** Map Area Header */
		struct MapAreaHeader {
			const char magic[4] = { 'A', 'R', 'E', 'A' };
			unsigned short flags;
			unsigned short gridArea;
		} areaHeader;
		bool fullArea;
		// Map Height Flags
		static const unsigned int MAP_HEIGHT_NO_HEIGHT = 0x0001;
		static const unsigned int MAP_HEIGHT_AS_INT16 = 0x0002;
		static const unsigned int MAP_HEIGHT_AS_INT8 = 0x0004;
		/** Map Height Header */
		struct MapHeightHeader {
			const char magic[4] = { 'M', 'H', 'G', 'T' };
			unsigned int flags;
			float gridHeight;
			float gridMaxHeight;
		} mapHeightHeader;
		// Map Liquid Flags
		static const unsigned int MAP_LIQUID_NO_TYPE = 0x0001;
		static const unsigned int MAP_LIQUID_NO_HEIGHT = 0x0002;

		static const unsigned int MAP_LIQUID_TYPE_NO_WATER = 0x00;
		static const unsigned int MAP_LIQUID_TYPE_MAGMA = 0x01;
		static const unsigned int MAP_LIQUID_TYPE_OCEAN = 0x02;
		static const unsigned int MAP_LIQUID_TYPE_SLIME = 0x04;
		static const unsigned int MAP_LIQUID_TYPE_WATER = 0x08;
		static const unsigned int MAP_LIQUID_TYPE_DARK_WATER = 0x10;
		static const unsigned int MAP_LIQUID_TYPE_WMO_WATER = 0x20;
		struct MapLiquidHeader {
			const char magic[4] = { 'M', 'L', 'I', 'Q' };
			unsigned short flags;
			unsigned short liquidType;
			unsigned char offsetX;
			unsigned char offsetY;
			unsigned char width;
			unsigned char height;
			float liquidLevel;
		} mapLiquidHeader;
		bool liquidShow[ADTV1::SIZE_ADT_GRID][ADTV1::SIZE_ADT_GRID];
		unsigned char liquidFlags[ADTV1::SIZE_TILE_MAP][ADTV1::SIZE_TILE_MAP];
		unsigned short liquidEntry[ADTV1::SIZE_TILE_MAP][ADTV1::SIZE_TILE_MAP];
		float liquidHeight[ADTV1::SIZE_ADT_GRID + 1][ADTV1::SIZE_ADT_GRID + 1];
		bool fullLiquidType;
		/** Holes flags */
		unsigned short holes[ADTV1::SIZE_TILE_MAP][ADTV1::SIZE_TILE_MAP];

	private:
		const std::string PATTERN_MAP = "%03u%02u%02u.map";
		Logger& _logger = Logger::get("Extractor");

		const unsigned int _mapId;
		const unsigned int _x;
		const unsigned int _y;

};
#endif