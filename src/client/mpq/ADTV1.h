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
#ifndef ADTV1_H
#define ADTV1_H

#include <string>
#include "Poco/Logger.h"
#include "MPQFile.h"
#include "../../geometry/CAaSphere.h"
#include "../../geometry/Point3D.h"

using Poco::Logger;

struct MCNK {
	struct MCVT {
		std::string magic;
		unsigned int size;
		float points[145];
	};

	struct MCNR {
		std::string magic;
		unsigned int size;
		Point3D points[145];
	};

	/** Liquid type information*/
	const static unsigned int LIQUID_DATA_LENGTH = 9;
	const static unsigned int LIQUID_FLAG_LENGTH = 8;

	struct MCLQ {
		struct SWFlowv {
			CAaSphere sphere;
			Point3D direction;
			float velocity;
			float amplitude;
			float frequency;
		};

		float minHeight;
		float maxHeight;
		unsigned int lights[LIQUID_DATA_LENGTH * LIQUID_DATA_LENGTH];
		float height[LIQUID_DATA_LENGTH * LIQUID_DATA_LENGTH];
		unsigned short flags[LIQUID_FLAG_LENGTH * LIQUID_FLAG_LENGTH];

		unsigned int nFlowvs;
		SWFlowv* flowvs;
	};

	struct MCLY {
		unsigned int textureId;
		unsigned int flags;
		unsigned int offsetinMCAL;
		unsigned int effectId;
	};

	std::string magic;
	unsigned int size;
	unsigned int flags;
	unsigned int indexX;
	unsigned int indexY;
	unsigned int nbLayers;
	unsigned int nDoodadRefs;
	unsigned int offsetMCVT;
	unsigned int offsetMCNR;
	unsigned int offsetMCLY;
	unsigned int offsetMCRF;
	unsigned int offsetMCAL;
	unsigned int sizeAlpha;
	unsigned int offsetMCSH;
	unsigned int sizeShadow;
	unsigned int areaId;
	unsigned int nMapObjRefs;
	unsigned int holes;
	unsigned char lowQualityTextMap[8][2];
	unsigned int predTex;
	unsigned int noEffectDoodad;
	unsigned int offsetMCSE;
	unsigned int nSndEmitters;
	unsigned int offsetMCLQ;
	unsigned int sizeLiquid;
	Point3D position;
	unsigned int offsetMCCV;
	unsigned int offsetMCLV;
	MCVT vertices;
	MCNR normals;
	MCLQ* listLiquids;
	//MCLY textureLayers[4];
	unsigned int* refList;
};

/**
* This file represent an ADT file from a WoW Vanilla client.
*/
class ADTV1 : public MPQFile
{
	public:
		static const unsigned int SIZE_TILE_MAP = 16;
		static const unsigned int SIZE_TILE_HEIGHTMAP = 144;
		static const unsigned int CHUNK_TILE_MAP_LENGTH = 8;
		static const unsigned int SIZE_ADT_GRID = 128;
		ADTV1(std::string name, char* data, long size);
		~ADTV1();
		bool parse();
		MCNK* getCell(unsigned int x, unsigned int y);
		unsigned int cellsSize();
		bool hasLiquid(MCNK* chunk);
		static bool hasLiquid(MCNK::MCLQ* liquid, unsigned int y, unsigned int x);
		static bool hasNoLiquid(MCNK::MCLQ* liquid, unsigned int y, unsigned int x);
		static bool isDarkWater(MCNK::MCLQ* liquid, unsigned int y, unsigned int x);
		static bool isRiver(MCNK* chunk);
		static bool isOcean(MCNK* chunk);
		static bool isMagma(MCNK* chunk);
		static bool isSlime(MCNK* chunk);
		bool hasNoLiquid(MCNK* chunk);
	private:
		bool readMCIN();

		static const unsigned int GLOBAL_OFFSET = 20;
		static const unsigned int CHUNK_TILE_HEIGHTMAP_LENGTH = 9;
		/** MCIN Chunk for MCNK information in the ADT File. */
		const std::string HEADER_MCIN = "NICM";
		struct MCIN {
			unsigned int offsetMCNK;
			unsigned int size;
			unsigned int flags;
			unsigned int asyncId;
		};
		/** MCNK Chunk with detailed chunk information in the ADT File. */
		const std::string HEADER_MCNK = "KNCM";
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
		/** ADT Header - RDHM chunk */
		const std::string HEADER_MHDR = "RDHM";
		struct {
			std::string magic;
			unsigned int size;
			unsigned long flags;
			unsigned int offsetMCIN;
			unsigned int offsetMTEX;
			unsigned int offsetMMDX;
			unsigned int offsetMMID;
			unsigned int offsetMWMO;
			unsigned int offsetMWID;
			unsigned int offsetMDDF;
			unsigned int offsetMODF;
		} _header;
		/**	ADT - NICM chunk */
		unsigned int _cells;
		MCIN* _chunkInfos;

		/** MCNK headers. */
		const std::string HEADER_MCVT = "TVCM";
		const std::string HEADER_MCNR = "RNCM";
		const std::string HEADER_MCLQ = "QLCM";
		const std::string HEADER_MCLY = "YLCM";

		enum ADTMCNKFlag {
			MCNK_HAS_MCSH = 0x01,
			MCNK_IS_IMPASS = 0x02,
			MCNK_IS_RIVER = 0x04,
			MCNK_IS_OCEAN = 0x08,
			MCNK_IS_MAGMA = 0x10,
			MCNK_IS_SLIME = 0x20,
			MCNK_HAS_MCCV = 0x40,
			//UNKNOWN_1 = 0x80,
			//UNKNOWN_2 = 0x100,
			//UNKNOWN_3 = 0x200,
			//UNKNOWN_4 = 0x400,
			//UNKNOWN_5 = 0x800,
			//UNKNOWN_6 = 0x1000,
			//UNKNOWN_7 = 0x2000,
			//UNKNOWN_8 = 0x4000,
			//UNKNOWN_9 = 0x8000,
			MCNK_IS_HIGH_RES_HOLE = 0x10000
			// More unknown flags..
		};

		enum ADTMCLQFlags {
			MCLQ_IS_WATER = 0x00,
			MCLQ_IS_OCEAN = 0x01,
			MCLQ_IS_MAGMA = 0x02,
			MCLQ_IS_SLIME = 0x03,
			MCLQ_IS_ANIMATED = 0x04,
			MCLQ_IS_E = 0x08,
			MCLQ_IS_C = 0x10,
			MCLQ_IS_D = 0x20,
			MCLQ_IS_FISHABLE = 0x40,
			MCLQ_IS_DARK = 0x80
		};

		enum ADTMCLQMask {
			MCLQ_HAS_LIQUID = 0x03,
			MCLQ_NO_LIQUID = 0x0F
		};
};

#endif