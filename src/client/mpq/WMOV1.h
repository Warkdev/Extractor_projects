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

#ifndef WMOV1_H
#define WMOV1_H

#include <string>
#include "Poco/Logger.h"
#include "MPQFile.h"
#include "G3D/AABox.h"
#include "G3D/Vector2.h"
#include "G3D/Vector3.h"
#include "G3D/Plane.h"
#include "G3D/Quat.h"

#pragma warning(disable : 4200) // Disable MS-specific warnings

using Poco::Logger;
using G3D::AABox;
using G3D::Vector2;
using G3D::Vector3;
using G3D::Plane;
using G3D::Quat;

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
		AABox boundinbBox;;
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
	Vector3 portalVertexList[];
};

struct MOPT {
	char magic[4];
	unsigned int size;
	struct Portal {
		unsigned short startVertex;
		unsigned short count;
		Plane plane;
	} portalList[];
};

struct MOVV {
	char magic[4];
	unsigned int size;
	Vector3 visibleBlocksVertices[];
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
		Vector3 position;
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
		Vector3 position;
		Quat orientation;
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
		Vector3 position;
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
		Plane plane;
	} convexVolumePlanes[];
};

class WMOV1 : public MPQFile
{
public:
	WMOV1(std::string name, unsigned char* data, long size);
	~WMOV1();
	bool parse();
	unsigned int getNGroups();
	unsigned int getWMOID();
	bool useLiquidTypeFromDBC();

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
		unsigned int wmoID;
		AABox boundingBox;
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

// Here starts WMOGroup content

struct MOGP {
	char magic[4];
	unsigned int size;
	struct GroupInfo {
		unsigned int groupName;
		unsigned int descriptiveGroupName;
		unsigned int flags;
		AABox boundingBox;
		unsigned short portalStart;
		unsigned short portalCount;
		unsigned short transBatchCount;
		unsigned short intBatchCount;
		unsigned short extBatchCount;
		unsigned short padding;
		unsigned char fogsIds[4];
		unsigned int groupLiquid;
		unsigned int wmoAreaTableRecId;
		unsigned int flags2;
		unsigned int unused;
	} info;
};

enum PolyFlags {
	UNK1 = 0x1,
	NO_CAM_COLLIDE = 0x2,
	DETAIL = 0x4,
	COLLISION = 0x8,
	HINT = 0x10,
	RENDER = 0x20,
	UNK2 = 0x40,
	COLLIDE_HIT = 0x80
};

struct MOPY {
	char magic[4];
	unsigned int size;
	struct Material {
		unsigned char flags;
		unsigned char materialId;
	} materials[];
};

struct MOVI {
	char magic[4];
	unsigned int size;
	short indexes[];
};

struct MOVT {
	char magic[4];
	unsigned int size;
	Vector3 vertices[];
};

struct MONR {
	char magic[4];
	unsigned int size;
	Vector3 normals[];
};

struct MOTV {
	char magic[4];
	unsigned int size;
	Vector2 textVertex[];
};

struct MOBA {
	char magic[4];
	unsigned int size;
	struct Batch {
		unsigned short bx;
		unsigned short by;
		unsigned short bz;
		unsigned short tx;
		unsigned short ty;
		unsigned short tz;
		unsigned int startIdx;
		unsigned short count;
		unsigned short minIdx;
		unsigned short maxIdx;
		unsigned char flag;
		unsigned char padding;
	} batches[];
};

struct MOLR {
	char magic[4];
	unsigned int size;
	unsigned short lights[];
};

struct MODR {
	char magic[4];
	unsigned int size;
	unsigned short doodads[];
};

struct MOBN {
	char magic[4];
	unsigned int size;
	struct BspNode {
		unsigned short flags;
		unsigned short negChild;
		unsigned short posChild;
		unsigned short nFaces;
		unsigned int faceStart;
		float planeDist;
	} bspTree[];
};

struct MOBR {
	char magic[4];
	unsigned size;
	unsigned short nodeFacesIndices[];
};

struct MOCV {
	char magic[4];
	unsigned size;
	struct Vector {
		unsigned char b;
		unsigned char g;
		unsigned char r;
		unsigned char a;
	} colors[];
};

struct MLIQ {
	char magic[4];
	unsigned size;
	struct Header {
		unsigned int xVerts;
		unsigned int yVerts;
		unsigned int xTiles;
		unsigned int yTiles;
		Vector3 baseCoords;
		unsigned short materialId;
	} header;
};

// Liquid vertices comes at the end of MLIQ chunk
struct LiquidVert {
	unsigned char flow1;
	unsigned char flow2;
	unsigned char flow1Pct;
	unsigned char filler;
	float height;
};

// And after that, we get the liquid tiles list.
enum LiquidTileFlags {
	IS_WATER = 0x00,
	IS_OCEAN = 0x01,
	IS_MAGMA = 0x02,
	IS_SLIME = 0x03,
	IS_ANIMATED = 0x04,
	IS_E = 0x10,
	IS_F = 0x20,
	IS_FISHABLE = 0x40,
	HAS_OVERLAP = 0x80
};

enum LiquidTileMasks {
	MASK_LIQUID = 0x03,
	MASK_NO_LIQUID = 0x0F
};

struct LiquidTile {
	unsigned char flag;
};

struct MORI {
	char magic[4];
	unsigned int size;
	unsigned short triangleStripIndeces[];
};

class WMOGroupV1 : public MPQFile
{
	public:
		WMOGroupV1();
		WMOGroupV1(std::string name, unsigned char* data, long size);
		~WMOGroupV1();
		bool parse();
		bool hasLiquid();

		unsigned int getAreaTableId();

		// Poly
		bool isCollidable(unsigned int index);

		// Liquid
		bool tileHasNoLiquid(unsigned int idx);
		bool tileIsWater(unsigned int idx);
		bool tileIsOcean(unsigned int idx);
		bool tileIsMagma(unsigned int idx);
		bool tileIsSlime(unsigned int idx);

		MOGP::GroupInfo * getGroupInfo();
		MOBA* getBatchInfo();
		MOPY* getPolyInfo();
		MOVI* getVertexIndices();
		MOVT* getVertexInfo();
		MLIQ* getLiquidInfo();
		LiquidVert* getLiquidVertices();
		LiquidTile* getLiquidFlags();
	private:
		LiquidTile* getLiquidTile(unsigned int idx);

		/** File version - REVM chunk */
		const std::string HEADER_MVER = "REVM";
		static const unsigned int SUPPORTED_VERSION = 17;
		struct Version {
			char magic[4];
			unsigned int size;
			unsigned int version;
		} *_version;
		/** MOGP chunk */
		const std::string HEADER_MOGP = "PGOM";

		enum GroupFlags
		{
			HAS_BSP_TREE =			0x00000001,
			HAS_LIGHT_MAP =			0x00000002,
			HAS_VERTEX_COLORS =		0x00000004,
			IS_EXTERIOR =			0x00000008,
			UNK1 =					0x00000010,
			UNK2 =					0x00000020,
			EXTERIOR_LIT =			0x00000040,
			UNREACHABLE =			0x00000080,
			UNK3 =					0x00000100,
			HAS_LIGHT =				0x00000200,
			UNK4 =					0x00000400,
			HAS_DOODADS =			0x00000800,
			HAS_LIQUID =			0x00001000,
			IS_INTERIOR =			0x00002000,
			UNK5 =					0x00004000,
			UNK6 =					0x00008000,
			ALWAYS_DRAW =			0x00010000,
			HAS_TRIANGLESTRIP =		0x00020000,
			SHOW_SKYBOX =			0x00040000,
			IS_OCEAN =				0x00080000,
			UNK7 =					0x00100000,
			IS_MOUNT_ALLOWED =		0x00200000,
			UNK8 =					0x00400000,
			UNK9 =					0x00800000,
			HAS_2_MOCV =			0x01000000,
			HAS_2_MOTV =			0x02000000,
			ANTIPORTAL =			0x04000000,
			UNK10 =					0x08000000,
			UNK11 =					0x10000000,
			EXTERIOR_CULL =			0x20000000,
			HAS_3_MOTV =			0x40000000,
			UNK12 =					0x80000000
		};

		enum GroupLiquidMask
		{
			MASK_HAS_LIQUID = 0x0F
		};

		MOGP * _group;
		
		/** MOPY chunk */
		const std::string HEADER_MOPY = "YPOM";
		unsigned int _offsetMOPY = 0;
		/** MOVI chunk */
		const std::string HEADER_MOVI = "IVOM";
		unsigned int _offsetMOVI = 0;
		/** MOVT chunk */
		const std::string HEADER_MOVT = "TVOM";
		unsigned int _offsetMOVT = 0;
		/** MONR chunk */
		const std::string HEADER_MONR = "RNOM";
		unsigned int _offsetMONR = 0;
		/** MOTV chunk */
		const std::string HEADER_MOTV = "VTOM";
		unsigned int _offsetMOTV = 0;
		/** MOBA chunk */
		const std::string HEADER_MOBA = "ABOM";
		unsigned int _offsetMOBA = 0;
		/** MOLR chunk */
		const std::string HEADER_MOLR = "RLOM";
		unsigned int _offsetMOLR = 0;
		/** MODR chunk */
		const std::string HEADER_MODR = "RDOM";
		unsigned int _offsetMODR = 0;
		/** MOBN chunk */
		const std::string HEADER_MOBN = "NBOM";
		unsigned int _offsetMOBN = 0;
		/** MOBR chunk */
		const std::string HEADER_MOBR = "RBOM";
		unsigned int _offsetMOBR = 0;
		/** MOCV chunk */
		const std::string HEADER_MOCV = "VCOM";
		unsigned int _offsetMOCV = 0;
		/** MLIQ chunk */
		const std::string HEADER_MLIQ = "QILM";
		unsigned int _offsetMLIQ = 0;
		/** MORI chunk */
		const std::string HEADER_MORI = "IROM";
		unsigned int _offsetMORI = 0;
};

#endif