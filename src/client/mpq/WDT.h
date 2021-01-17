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
#include "Poco/Logger.h"
#include "MPQFile.h"

using Poco::Logger;

class WDT : public MPQFile
{
	public:
		const static int MAP_TILE_SIZE = 64;
		WDT(std::string name, char* data, long size);
		~WDT();
		bool parse();
		bool hasADT(int x, int y);
		
	private:
		enum WDTHeaderFlags {
			USE_GLOBAL_MAP_OBJ = 0x0001,
			ADT_HAS_MCCV = 0x0002,
			ADT_HAS_BIG_ALPHA = 0x0004,
			ADT_HAS_DOODADREFS_SORTED_BY_SIZE_CAT = 0x0008,
			HAS_LIGHT_IN_GVERTICES = 0x0010,
			ADT_HAS_UPSIDE_DOWN_GROUND = 0x0020,
			//UNKNOWN_1 = 0x0040,
			ADT_HAS_HEIGHT_TEXTURING = 0x0080,
			//UNKNOWN_2 = 0x0100,
			WDT_HAS_MAID = 0x0200
		};
		enum WDTAreaInfoFlags {
			HAS_ADT = 0x01
		};
		/** AreaInfo Struct */
		struct AreaInfo {
			int flags;
			int asyncId;
		};

		/** MODF Chunk for WMO placement, if any. */
		struct MODF {
			unsigned int mwidEntry;
			unsigned int uniqueId;
			float position[3];
			float orientation[3];
			float upperExtents[3];
			float lowerExtents[3];
			unsigned short flags;
			unsigned short doodadSet;
			unsigned short nameSet;
			unsigned short padding;
		};
		/** File version - REVM chunk */
		const std::string HEADER_MVER = "REVM";
		struct {
			std::string magic;
			unsigned int size;
			unsigned int version;
		} _version;
		/** WDT Header - DHPM chunk */
		const std::string HEADER_MPHD = "DHPM";
		struct {
			std::string magic;
			unsigned int size;
			unsigned int flags;
		} _header;
		/** WDT Main - NIAM chunk */
		const std::string HEADER_MAIN = "NIAM";
		struct {
			std::string magic;
			unsigned int size;
			AreaInfo* areas;
		} _main;
		/** WDT World Model Object (if any) - OMWM chunk */
		const std::string HEADER_MWMO = "OMWM";
		struct {
			std::string magic;
			unsigned int size;
			std::string name;
		} _wmo;
		/** WDT WMO Placement - FDOM chunk */
		const std::string HEADER_MODF = "FDOM";
		struct {
			std::string magic;
			unsigned int size;
			MODF placement;
		} _objDef;
};