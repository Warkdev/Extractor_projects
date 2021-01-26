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

#include "../client/mpq/M2V1.h"
#include "../client/mpq/WMOV1.h"
#include "G3D/AABox.h"
#include "G3D/Vector3.h"

using G3D::AABox;
using G3D::Vector3;

enum ModelFlags
{
	MOD_M2 = 0x01,
	MOD_WORLDSPAWN = 0x02,
	MOD_HAS_BOUND = 0x04
};

class Model
{
	public:
		unsigned int nVertices;
		Vector3* vertices;
		unsigned int nIndices;
		unsigned short* indices;
		unsigned int flags = MOD_M2;

		Model() : nVertices(0), vertices(NULL), nIndices(0), indices(NULL) {}
		Model(M2V1* m2);
		Model(WMOV1* wmo, WMOGroupV1** groups);
		~Model() { delete[] indices; delete[] vertices; }
};

class ModelInstance
{
	public:
		Model* model;
		// Model Instance data.
		unsigned short adtId = 0; // Not used for models
		unsigned int id;
		Vector3 position;
		Vector3 rotation;
		float scale;
		AABox boundingBox;
		std::string name;

		ModelInstance(Model* model) : model(model), adtId(0), id(0), position(Vector3(0, 0, 0)), rotation(Vector3(0, 0, 0)), scale(0.0f), boundingBox(Vector3(0, 0, 0), Vector3(0, 0, 0)), name("") {};
		~ModelInstance() {};

		bool operator==(const ModelInstance& other) const { return id == other.id; }
};