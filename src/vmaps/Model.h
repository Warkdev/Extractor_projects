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

#ifndef MODEL_H
#define MODEL_H

#include "../client/mpq/M2V1.h"
#include "../client/mpq/WMOV1.h"
#include "G3D/AABox.h"
#include "G3D/Vector3.h"
#include "BIH.h"

using G3D::AABox;
using G3D::Vector3;

enum ModelFlags
{
	MOD_M2 = 0x01,
	MOD_WORLDSPAWN = 0x02,
	MOD_HAS_BOUND = 0x04
};

class MeshTriangle
{
public:
	/**
	 * @brief
	 *
	 */
	MeshTriangle() {};
	/**
	 * @brief
	 *
	 * @param na
	 * @param nb
	 * @param nc
	 */
	MeshTriangle(unsigned int na, unsigned int nb, unsigned int nc) : idx0(na), idx1(nb), idx2(nc) {};

	unsigned int idx0; /**< TODO */
	unsigned int idx1; /**< TODO */
	unsigned int idx2; /**< TODO */
};

class TriBoundFunc
{
public:
	TriBoundFunc(std::vector<Vector3>& vert) : vertices(vert.begin()) {}
	void operator()(const MeshTriangle& tri, G3D::AABox& out) const
	{
		G3D::Vector3 lo = vertices[tri.idx0];
		G3D::Vector3 hi = lo;

		lo = (lo.min(vertices[tri.idx1])).min(vertices[tri.idx2]);
		hi = (hi.max(vertices[tri.idx1])).max(vertices[tri.idx2]);

		out = G3D::AABox(lo, hi);
	}
protected:
	const std::vector<Vector3>::const_iterator vertices;
};

struct ModelLiquid
{
	unsigned int tilesX;
	unsigned int tilesY;
	Vector3 baseCoords;
	unsigned int type;
	float* height;
	unsigned char* flags;
};

class ModelGroup
{
	public:
		AABox boundingBox;
		unsigned int groupFlags;
		unsigned int groupWMOID;
		unsigned int nVertices;
		std::vector<Vector3> vertices;
		unsigned int nTriangles;
		unsigned int nIndices;
		std::vector<MeshTriangle> mesh;
		BIH tree;
		ModelLiquid* liquid;

		const G3D::AABox& GetBound() const { return boundingBox; }
};

class Model
{
	public:
		unsigned int rootWMOID;
		unsigned int flags;
		unsigned int nGroups;
		std::vector<ModelGroup> groups;
		std::string name;
		BIH groupTree;

		Model();
		Model(std::string name, M2V1* m2);
		Model(std::string name, WMOV1* wmo, WMOGroupV1** wmoGroups, bool preciseVectorData);
		~Model() 
		{ 
			if (nGroups)
			{
				for (int i = 0; i < nGroups; i++)
				{
					if (groups[i].liquid)
					{
						delete groups[i].liquid;
					}
				}
			}
		}
};

class ModelInstance
{
	public:
		Model* model;
		// Model Instance data.
		unsigned short adtId = 0; // Not used for models
		unsigned int nodeIdx = 0;
		unsigned int id;
		unsigned int tileX;
		unsigned int tileY;
		Vector3 position;
		Vector3 rotation;
		float scale;
		AABox boundingBox;

		ModelInstance(Model* model, unsigned int uniqueId, unsigned short adtId, unsigned int nodeIdx, unsigned int tileX, unsigned int tileY, Vector3 position, Vector3 rotation, float scale, AABox boundingBox);
		~ModelInstance() {};

		bool operator==(const ModelInstance& other) const { return id == other.id; }
};

#endif