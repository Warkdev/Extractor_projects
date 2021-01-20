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

#ifndef WMOFILE_H
#define WMOFILE_H

#include "Poco/Logger.h"
#include "Poco/Util/LayeredConfiguration.h"
#include "../client/mpq/WMOV1.h"

using Poco::Logger;
using Poco::Util::LayeredConfiguration;

class WMOFile {
public:
	WMOFile(std::string directory, std::string hash, std::string filename);
	~WMOFile();

	bool save(std::string path);

	/** VMap File Header */
	struct VmapHeader {
		const char magic[4] = { 'V', 'M', 'A', 'P' };
		char versionMagic[4]; // "z070" for classic, "s070" for tbc, "t070" for wotlk, "c070" for cata, "p070" for mop, "w070" for wod, "l070" for legion.
		unsigned int nVectors = 0;
		unsigned int nGroups = 0;
		unsigned int rootWMOID = 0;
	} header;

	/** Vmap Group */
	struct VmapGroup {
		unsigned int flags = 0;
		unsigned int groupWMOID = 0;
		struct {
			struct {
				float x = 0;
				float y = 0;
				float z = 0;
			} min;
			struct {
				float x = 0;
				float y = 0;
				float z = 0;
			} max;
		} boundingBox;
		unsigned int liquidFlags = 0;
		struct {
			const char magic[4] = { 'G', 'R', 'P', ' ' };
			unsigned int mobaSize;		// MOBA size / 2
			unsigned int mobaBatch;		// mobaSize / 12. WTF are these dudes doing ?! 12 is the size of a MOBA record.
			unsigned int * mobaEx;		// Dynamic size, mobaSize but with a padding of 12 each time.. these guys.. Matches bx/by/startIdx from MOBA chunk.
		} header;
		struct {
			const char magic[4] = { 'I', 'N', 'D', 'X' };
			unsigned int size;			// sizeof(int) + (sizeof(short) * nIndices)
			unsigned int nIndices;		// nTriangles * 3. nTriangles being (MOPY size / 2)
			unsigned short * indices;	// Dynamic size - nIndices.
		} indices;
		struct {
			const char magic[4] = { 'V', 'E', 'R', 'T' };
			unsigned int size;			// sizeof(int) + (sizeof(float) * 3 * nVertices)
			unsigned int nVertices;		// nVertices. nVertices being (MOVT size / 12)
			struct {
				float x;
				float y;
				float z;
			} * vertices;				// Dynamic size - nVertices.
		} vertices;
		struct {
			const char magic[4] = { 'L', 'I', 'Q', 'U' };
			unsigned int size;			// sizeof(MLIQ::Header) + (sizeof(liquidVert) * xVerts * yVerts);
			unsigned int xVerts;
			unsigned int yVerts;
			unsigned int xTiles;
			unsigned int yTiles;
			struct {
				float x;
				float y;
				float z;
			} baseCoords;
			unsigned short type;
			float * height;			// Dynamic size (xVerts * yVerts)
			unsigned char * flags;	// Dynamic size (xTiles * yTiles)
		} liquid;
	} group;

private:
	const std::string PATTERN_WMO = "%s-%s"; // hash-filename (including .wmo)
	Logger& _logger = Logger::get("Extractor");

	const std::string _directory;
	const std::string _hash;
	const std::string _filename;

};
#endif