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

struct C2Vector
{
	float x;
	float y;
};

struct C2iVector
{
	int x;
	int y;
};

struct C3Vector
{
	float x;
	float y;
	float z;
};

struct C3iVector
{
	int x;
	int y;
	int z;
};

struct C4Vector
{
	float x;
	float y;
	float z;
	float w;
};

struct C4iVector
{
	int x;
	int y;
	int z;
	int w;
};

struct C4Quaternion
{
	float x;
	float y;
	float z;
	float w;
};

struct CAaBox
{
	C3Vector min;
	C3Vector max;
};

struct CAaSphere
{
	C3Vector position;
	float radius;
};

struct C33Matrix
{
	C3Vector columns[3];
};

struct C34Matrix
{
	C3Vector columns[4];
};

struct C44Matrix
{
	C4Vector columns[4];
};

struct C4Plane
{
	C3Vector normal;
	float distance;
};

struct CRange
{
	float min;
	float max;
};

struct CiRange
{
	int min;
	int max;
};

struct CArgb
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

struct CImVector
{
	unsigned char b;
	unsigned char g;
	unsigned char r;
	unsigned char a;
};

struct C3sVector
{
	unsigned short x;
	unsigned short y;
	unsigned short z;
};

struct C3Segment
{
	C3Vector start;
	C3Vector end;
};

struct CFacet
{
	C4Plane plane;
	C3Vector vertices[3];
};

struct C3Ray
{
	C3Vector origin;
	C3Vector dir;
};

struct CRect
{
	float top;
	float miny;
	float left;
	float minx;
	float bottom;
	float maxy;
	float right;
	float maxx;
};

struct CiRect
{
	int top;
	int miny;
	int left;
	int minx;
	int bottom;
	int maxy;
	int right;
	int maxx;
};