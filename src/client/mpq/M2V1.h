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

template<typename T>
struct M2Array
{
	unsigned int size;
	unsigned int offset;
};

struct M2TrackBase
{
	unsigned short interpolationType;
	unsigned short globalSequence;
	M2Array<std::pair<unsigned int, unsigned int>> interpolationRanges;
	M2Array<unsigned int> timestamps;
};

template<typename T>
struct M2Track : M2TrackBase
{
	M2Array<T> values;
};

struct M2Loop
{

};

struct M2Sequence
{

};

struct M2CompBone
{

};

struct M2Vertex
{

};

struct M2SkinProfile
{

};

struct M2Color
{

};

struct M2Texture
{

};

struct M2TextureWeight
{

};

struct M2TextureTransform
{

};

struct M2Material
{

};

struct M2Attachment
{
	unsigned int id;
	unsigned short bone;
	unsigned short unknown;
	struct {
		float x;
		float y;
		float z;
	} position;
};

struct M2Event
{

};

struct M2Light
{

};

struct M2Camera
{

};

struct M2Ribbon
{

};

struct M2Particle
{

};

struct M2SkinProfile
{

};

struct C3Vector
{

};

class M2V1 : public MPQFile
{
public:
	M2V1(std::string name, unsigned char* data, long size);
	~M2V1();
	bool parse();

private:
	/** File version - 02DM chunk */
	const std::string HEADER_MD20 = "20DM";
	static const unsigned int SUPPORTED_VERSION = 256;

	struct Header {
		char magic[4];
		unsigned int size;
		unsigned int version;
	} * _header;
	
};