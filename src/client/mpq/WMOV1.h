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

#pragma warning(disable : 4200) // Disable MS-specific warnings

using Poco::Logger;

struct MOTX {
	char magic[4];
	unsigned int size;
	char textureNameList[];
};

struct MOMT {
	char magic[4];
	unsigned int size;
	unsigned int flags;
	unsigned int shader;
	unsigned int diffuseNameIdx;
	struct {
		unsigned char b;
		unsigned char g;
		unsigned char r;
		unsigned char a;
	} sidnColor;
	struct {
		unsigned char b;
		unsigned char g;
		unsigned char r;
		unsigned char a;
	} frameSidnColor;
	unsigned int envNameIdx;
	unsigned int diffColor;
	unsigned int idTerrainTypeRec; // Foreign key to TerrainTypeRec.dbc
	unsigned int texture;
	unsigned int color;
	unsigned int flags2;
	unsigned int runtimeData[4];
};

struct MOGN {
	char magic[4];
	unsigned int size;
	char textureNameList[];
};

struct MOGI {
	char magic[4];
	unsigned int size;
	struct GroupInfo {
		unsigned int flags;
		struct {
			struct {
				float x;
				float y;
				float z;
			} min;
			struct {
				float x;
				float y;
				float z;
			} max;
		} boundinbBox;;
	} info[];
};

struct MOSB {
	char magic[4];
	unsigned int size;
	char skyboxName[];
};

struct MOPV {
	char magic[4];
	unsigned int size;
	struct Vertex {
		float x;
		float y;
		float z;
	} portalVertexList[];
};

struct MOPT {
	char magic[4];
	unsigned int size;
	struct Portal {
		unsigned short startVertex;
		unsigned short count;
		struct {
			struct {
				float x;
				float y;
				float z;
			} normal;
			float distance;
		} plane;
	} portalList[];
};

struct MOVV {
	char magic[4];
	unsigned int size;
	struct {
		float x;
		float y;
		float z;
	} visibleBlocksVertices[];
};

struct MOVB {
	char magic[4];
	unsigned int size;
	struct {
		unsigned short firstVertex;
		unsigned short count;
	} visibleBlocks[];
};

struct MOPR {
	char magic[4];
	unsigned int size;
	struct PortalRef {
		unsigned short portalIdx;
		unsigned short groupIdx;
		short side;
		unsigned short filler;
	};
};

enum LightType {
	OMNI_LGT = 0,
	SPOT_LGT = 1,
	DIRECT_LGT = 2,
	AMBIENT_LGT = 3
};

struct MOLT {
	char magic[4];
	unsigned size;
	struct Light {
		unsigned char type;
		unsigned char useAtten;
		unsigned char pad[2];
		struct {
			unsigned char b;
			unsigned char g;
			unsigned char r;
			unsigned char a;
		} color;
		struct {
			float x;
			float y;
			float z;
		} position;
		float intensity;
		float unknown[4];
		float attenStart;
		float attenEnd;
	} lightList[];
};

struct MODS {
	char magic[4];
	unsigned size;
	struct DoodadSet {
		char name[20];
		unsigned int startIdx;
		unsigned int count;
		char pad[4];
	} doodadSetList[];
};

struct MODN {
	char magic[4];
	unsigned size;
	char doodadNameList[];
};

struct MODD {
	char magic[4];
	unsigned size;
	struct DoodadDef {
		unsigned int nameIdx;
		unsigned int flag;
		struct {
			float x;
			float y;
			float z;
		} position;
		struct {
			float x;
			float y;
			float z;
			float w;
		} orientation;
		float scale;
		struct {
			unsigned char b;
			unsigned char g;
			unsigned char r;
			unsigned char a;
		} color;
	} doodadDefList[];
};

enum EFogs {
	FOG = 0,
	UW_FOG = 1, // under water
	NUM_FOGS = 2
};

struct MFOG {
	char magic[4];
	unsigned int size;
	struct Fog {
		unsigned int flag;
		struct {
			float x;
			float y;
			float z;
		} position;
		float smallerRadius;
		float largerRadius;
		struct iFog {
			float end;
			float startScalar;
			struct {
				unsigned char b;
				unsigned char g;
				unsigned char r;
				unsigned char a;
			} color;
		} fogs[2];
	} fogList[];
};

struct MCVP {
	char magic[4];
	unsigned int size;
	struct {
		struct {
			struct {
				float x;
				float y;
				float z;
			} normal;
			float distance;
		} plane;
	} convexVolumePlanes[];
};

class WMOV1 : public MPQFile
{
public:
	WMOV1(std::string name, unsigned char* data, long size);
	~WMOV1();
	bool parse();

private:
	enum WMOHeaderFlags {
		DO_NOT_ATTENUATE_VERTICES = 0x1,
		USE_UNIFIED_RENDER_PATH = 0x2,
		USE_LIQUID_FROM_DBC = 0x4,
		DO_NOT_FIX_VERTEX_COLOR_ALPHA = 0x8
	};

	/** File version - REVM chunk */
	const std::string HEADER_MVER = "REVM";
	static const unsigned int SUPPORTED_VERSION = 17;
	struct Version {
		char magic[4];
		unsigned int size;
		unsigned int version;
	} * _version;
	/** WMO Header - DHOM chunk */
	const std::string HEADER_MOHD = "DHOM";
	struct Header {
		char magic[4];
		unsigned int size;
		unsigned int nMaterials;
		unsigned int nGroups;
		unsigned int nPortals;
		unsigned int nLights;
		unsigned int nModels;
		unsigned int nDoodads;
		unsigned int nDoodadSets;
		struct {
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		} ambColor;
		struct {
			struct {
				float x;
				float y;
				float z;
			} min;
			struct {
				float x;
				float y;
				float z;
			} max;
		} boundingBox;
		unsigned short flags;
		unsigned short numLod;
	} * _header;

	/** WMO Texture List - XTOM chunk */
	const std::string HEADER_MOTX = "XTOM";
	unsigned int _offsetMOTX = 0;
	/** WMO Materials - TMOM chunk */
	const std::string HEADER_MOMT = "TMOM";
	unsigned int _offsetMOMT = 0;
	/** WMO Groups Names - NGOM chunk */
	const std::string HEADER_MOGN = "NGOM";
	unsigned int _offsetMOGN = 0;
	/** WMO Groups Info - IGOM chunk */
	const std::string HEADER_MOGI = "IGOM";
	unsigned int _offsetMOGI = 0;
	/** WMO Skybox Info - BSOM chunk */
	const std::string HEADER_MOSB = "BSOM";
	unsigned int _offsetMOSB = 0;
	/** WMO - VPOM chunk */
	const std::string HEADER_MOPV = "VPOM";
	unsigned int _offsetMOPV = 0;
	/** WMO - TPOM chunk */
	const std::string HEADER_MOPT = "TPOM";
	unsigned int _offsetMOPT = 0;
	/** WMO - RPOM chunk */
	const std::string HEADER_MOPR = "RPOM";
	unsigned int _offsetMOPR = 0;
	/** WMO - VVOM chunk */
	const std::string HEADER_MOVV = "VVOM";
	unsigned int _offsetMOVV = 0;
	/** WMO - BVOM chunk */
	const std::string HEADER_MOVB = "BVOM";
	unsigned int _offsetMOVB = 0;
	/** WMO - TLOM chunk */
	const std::string HEADER_MOLT = "TLOM";
	unsigned int _offsetMOLT = 0;
	/** WMO - SDOM chunk */
	const std::string HEADER_MODS = "SDOM";
	unsigned int _offsetMODS = 0;
	/** WMO - NDOM chunk */
	const std::string HEADER_MODN = "NDOM";
	unsigned int _offsetMODN = 0;
	/** WMO - DDOM chunk */
	const std::string HEADER_MODD = "DDOM";
	unsigned int _offsetMODD = 0;
	enum WMODoodadDefFlag {
		ACCEPT_PROJ_TEXT = 0x1,
		UNK_2 = 0x2,
		UNK_3 = 0x4,
		UNK_4 = 0x8
	};
	/** WMO - GOFM chunk */
	const std::string HEADER_MFOG = "GOFM";
	unsigned int _offsetMFOG = 0;
	enum WMOFogFlag {
		INFINITE_RADIUS = 0x1
	};
	/** WMO - PVCM chunk */
	const std::string HEADER_MCVP = "PVCM";
	unsigned int _offsetMCVP = 0;
};