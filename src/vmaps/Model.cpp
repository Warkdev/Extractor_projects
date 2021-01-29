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

#include "Model.h"
#include "../MapUnit.h"
#include "G3D/g3dmath.h"
#include "G3D/Matrix3.h"

using G3D::Matrix3;

template<> struct BoundsTrait<ModelGroup>
{
    static void getBounds(const ModelGroup& group, G3D::AABox& out) { out = group.GetBound(); }
};

Model::Model() : rootWMOID(0), flags(0), nGroups(0), groups(NULL)
{

}

Model::Model(std::string name, M2V1* m2)
{
	flags = MOD_M2;
    this->name = name;
    this->rootWMOID = 0;
	nGroups = 1;
    groups.push_back(ModelGroup());
	groups[0].nVertices = m2->getNCollisionVertices();
	groups[0].nIndices = m2->getNCollisionTriangles();
    groups[0].nTriangles = groups[0].nIndices / 3;
	groups[0].liquid = NULL;
	groups[0].boundingBox = AABox::zero();
	groups[0].groupFlags = 0;
	groups[0].groupWMOID = 0;

    const Vector3* vertices = m2->getCollisionVertices();
    for (int i = 0; i < groups[0].nVertices; i++)
    {
        groups[0].vertices.push_back(vertices[i]);
    }

    const unsigned short* indices = m2->getCollisionTriangles();
    for (int i = 0; i < groups[0].nIndices; i += 3)
    {
        groups[0].mesh.push_back(MeshTriangle(indices[i], indices[i+1], indices[i+2]));
    }

    TriBoundFunc bFunc(groups[0].vertices);
    groups[0].tree.build(groups[0].mesh, bFunc);
    groupTree.build(groups, BoundsTrait<ModelGroup>::getBounds, 1);
}

Model::Model(std::string name, WMOV1* wmo, WMOGroupV1** wmoGroups, bool preciseVectorData)
{
	flags = MOD_HAS_BOUND;
    this->name = name;
    this->rootWMOID = wmo->getWMOID();
	this->nGroups = wmo->getNGroups();

	for (unsigned int i = 0; i < this->nGroups; i++)
	{
        MOGP::GroupInfo* info = wmoGroups[i]->getGroupInfo();
        MOBA* batchInfo = wmoGroups[i]->getBatchInfo();
        MOPY* polyInfo = wmoGroups[i]->getPolyInfo();
        MOVI* vertexIndices = wmoGroups[i]->getVertexIndices();
        MOVT* vertexInfo = wmoGroups[i]->getVertexInfo();
        groups.push_back(ModelGroup());

        groups[i].boundingBox.set(info->boundingBox.low(), info->boundingBox.high());
		groups[i].groupFlags = info->flags;
		groups[i].groupWMOID = info->wmoAreaTableRecId;

        if (preciseVectorData)
        {
            // Source: MOPY
            groups[i].nTriangles = polyInfo->size / sizeof(unsigned short);
            groups[i].nIndices = groups[i].nTriangles * 3;
            short* indices = vertexIndices->indexes;
            for (int j = 0; j < groups[i].nIndices; j += 3)
            {
                groups[i].mesh.push_back(MeshTriangle(indices[j], indices[j + 1], indices[j + 2]));
            }

            // Source: MOVT
            groups[i].nVertices = vertexInfo->size / sizeof(Vector3);
            const Vector3* vertices = vertexInfo->vertices;
            for (int j = 0; j < groups[i].nVertices; j++)
            {
                groups[i].vertices.push_back(vertices[j]);
            }
        }
        else 
        {
            unsigned int nColTriangles = 0;
            unsigned int nColVertices = 0;
            unsigned int nVectors = polyInfo->size / sizeof(unsigned short);
            unsigned int nVertices = vertexInfo->size / sizeof(Vector3);
            unsigned short* moviEx = new unsigned short[nVectors * 3]; // Worst case, all triangles are collisions ones.
            int* newIndex = new int[nVertices];
            memset(newIndex, 0xFF, nVertices * sizeof(int));
            unsigned short tempIdx = 0;

            for (int j = 0; j < nVectors; j++)
            {
                if (wmoGroups[i]->isCollidable(j))
                {
                    // Use this triangle.
                    for (int k = 0; k < 3; k++)
                    {
                        tempIdx = vertexIndices->indexes[3 * j + k];
                        newIndex[tempIdx] = 1;
                        moviEx[3 * nColTriangles + k] = tempIdx;
                    }
                    nColTriangles++;
                }
            }

            // Assign new vertex index numbers
            for (int j = 0; j < nVertices; j++)
            {
                if (newIndex[j] == 1)
                {
                    newIndex[j] = nColVertices;
                    nColVertices++;
                }
            }

            groups[i].nTriangles = nColTriangles;
            groups[i].nIndices = nColTriangles * 3;

            // Translate triangle indices to new numbers
            for (int j = 0; j < groups[i].nIndices; j += 3)
            {
                groups[i].mesh.push_back(MeshTriangle((unsigned short)newIndex[moviEx[j]], (unsigned short)newIndex[moviEx[j+1]], (unsigned short)newIndex[moviEx[j+2]]));
            }

            groups[i].nVertices = nColVertices;

            for (int j = 0; j < nVertices; j++)
            {
                if (newIndex[j] >= 0)
                {
                    groups[i].vertices.push_back(Vector3(vertexInfo->vertices[j].x, vertexInfo->vertices[j].y, vertexInfo->vertices[j].z));
                }
            }

            delete[] newIndex;
            delete[] moviEx;
        }

        // Build tree now.
        TriBoundFunc bFunc(groups[i].vertices);
        groups[i].tree.build(groups[i].mesh, bFunc);

        if (wmoGroups[i]->hasLiquid())
        {
            MLIQ* liquidInfo = wmoGroups[i]->getLiquidInfo();
            LiquidVert* liquidVertices = wmoGroups[i]->getLiquidVertices();
            unsigned int liquidVertexSize = liquidInfo->header.xVerts * liquidInfo->header.yVerts;
            unsigned int liquidFlagsSize = liquidInfo->header.xTiles * liquidInfo->header.yTiles;
            groups[i].liquid = new ModelLiquid();
            groups[i].liquid->tilesX = liquidInfo->header.xTiles;
            groups[i].liquid->tilesY = liquidInfo->header.yTiles;
            groups[i].liquid->baseCoords = liquidInfo->header.baseCoords;
            groups[i].liquid->height = new float[liquidVertexSize];
            groups[i].liquid->flags = new unsigned char[liquidFlagsSize];

            for (int j = 0; j < liquidVertexSize; j++)
            {
                groups[i].liquid->height[j] = liquidVertices[j].height;
            }
            memcpy(groups[i].liquid->flags, wmoGroups[i]->getLiquidFlags(), liquidFlagsSize);

            if (wmo->useLiquidTypeFromDBC())
            {
                groups[i].liquid->type = info->groupLiquid;
            }
            else {
                // Trying to determine the liquid type by parsing the flag tiles. The first one having a type wins.
                for (int j = 0; j < liquidFlagsSize; j++)
                {
                    if (wmoGroups[i]->tileIsWater(j))
                    {
                        groups[i].liquid->type = IS_WATER + 1;
                        break;
                    }
                    else if (wmoGroups[i]->tileIsOcean(j))
                    {
                        groups[i].liquid->type = IS_OCEAN + 1;
                        break;
                    }
                    else if (wmoGroups[i]->tileIsMagma(j))
                    {
                        groups[i].liquid->type = IS_MAGMA + 1;
                        break;
                    }
                    else if (wmoGroups[i]->tileIsSlime(j))
                    {
                        if (wmo->getWMOID() == 4489) // Naxxramas
                        {
                            groups[i].liquid->type = 21;
                        }
                        else
                        {
                            groups[i].liquid->type = IS_SLIME + 1;
                        }
                        break;
                    }
                }
            }
        }
        else
        {
            groups[i].liquid = NULL;
        }
	}

    groupTree.build(groups, BoundsTrait<ModelGroup>::getBounds, 1);
}

// Constructor is switching positions as the WoW Map system uses Y-up as origin.
ModelInstance::ModelInstance(Model* model, unsigned int uniqueId, unsigned short adtId, unsigned int nodeIdx, unsigned int tileX, unsigned int tileY, Vector3 position, Vector3 rotation, float scale, AABox boundingBox) :
	model(model), 
	adtId(adtId), nodeIdx(nodeIdx), tileX(tileX), tileY(tileY), 
	id(uniqueId), 
	position(Vector3(position.z, position.x, position.y)), 
	rotation(rotation), 
	scale(scale), 
	boundingBox(AABox
		(Vector3(boundingBox.low().z, boundingBox.low().x, boundingBox.low().y), 
			Vector3(boundingBox.high().z, boundingBox.high().x, boundingBox.high().y)))
{
	Matrix3 matrix;
	if (this->model->flags & MOD_M2) 
	{
		this->scale /= 1024.0f;
		// Calculate Transformed bound.
		matrix = Matrix3::fromEulerAnglesZYX(G3D::pi() * this->rotation.y / 180.0f, G3D::pi() * this->rotation.x / 180.f, G3D::pi() * this->rotation.z / 180.0f);
		AABox zero = AABox::zero();
		for (int i = 0; i < this->model->groups[0].nVertices; i++)
		{
			Vector3 v = matrix * (this->model->groups[0].vertices[i] * this->scale);
			if (i == 0)
			{
				this->boundingBox.set(v, v);
			}
			else 
			{
				this->boundingBox.merge(v);
			}	
		}
		this->boundingBox = this->boundingBox + this->position;
		this->model->flags |= MOD_HAS_BOUND;
	}
	else if (this->model->flags & MOD_WORLDSPAWN) // Worldspawn (WDT content) has different origin than WMO contained in ADT.
	{
		// WMO has wrong origin
		Vector3 v = Vector3(MAP_SIZE * 32, MAP_SIZE * 32, 0.f);
		this->position = position + v;
		this->boundingBox = this->boundingBox + v;
	}
};