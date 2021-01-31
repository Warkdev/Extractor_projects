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

#include "VMOFile.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/BinaryWriter.h"
#include "Poco/FileStream.h"

using Poco::Path;
using Poco::File;
using Poco::BinaryWriter;
using Poco::FileOutputStream;

VMOFile::VMOFile(Model* model) : _model(model)
{
}

VMOFile::~VMOFile()
{
}

bool VMOFile::save(std::string path)
{
    char vmoFile[1024];
    sprintf(vmoFile, PATTERN_FILE.c_str(), _model->name.c_str());
    Path filePath(path + "/" + vmoFile);
    File file(filePath);
    file.createFile();

    FileOutputStream stream(filePath.toString());
    BinaryWriter writer(stream);

    // Writing the header.
    writer.writeRaw(_header.magic, 8);

    writer.writeRaw(_root.magic, 4);

    // Writing the header size.
    writer << 8;
    writer << _model->rootWMOID;

    if (_model->nGroups) {
        writer.writeRaw(_group.header.magic, 4);
        writer << _model->nGroups;

        for (int i = 0; i < _model->nGroups; i++)
        {
            ModelGroup* gmod = &_model->groups[i];
            writer << gmod->boundingBox.low().x << gmod->boundingBox.low().y << gmod->boundingBox.low().z;
            writer << gmod->boundingBox.high().x << gmod->boundingBox.high().y << gmod->boundingBox.high().z;
            writer << gmod->groupFlags << gmod->groupWMOID;

            // Vertices chunk
            writer.writeRaw(_group.vertex.magic, 4);

            unsigned int chunkSize = sizeof(unsigned int) + sizeof(Vector3) * gmod->nVertices;
            writer << chunkSize; // Chunk size
            writer << gmod->nVertices;
            for (int j = 0; j < gmod->nVertices; j++)
            {
                writer << gmod->vertices[j].x << gmod->vertices[j].y << gmod->vertices[j].z;
            }
            
            // Triangle mesh
            writer.writeRaw(_group.mesh.magic, 4);
            chunkSize = sizeof(unsigned int) + sizeof(unsigned int) * gmod->nIndices;
            writer << chunkSize; // chunk size
            writer << gmod->nTriangles; // Amount of triangles

            for (int j = 0; j < gmod->nTriangles; j++)
            {
                writer << gmod->mesh[j].idx0 << gmod->mesh[j].idx1 << gmod->mesh[j].idx2;
            }

            // BIH
            AABox bounds = gmod->tree.getBoundingBox();
            std::vector<unsigned int> tree = gmod->tree.getTree();
            std::vector<unsigned int> objects = gmod->tree.getObject();
            writer.writeRaw(_group.bih.magic, 4);
            writer << bounds.low().x << bounds.low().y << bounds.low().z;
            writer << bounds.high().x << bounds.high().y << bounds.high().z;
            writer << tree << objects; // Poco vector << operator already feed the size.
            
            // Liquid
            writer.writeRaw(_group.liquid.magic, 4);
            if (gmod->liquid)
            {
                unsigned int heightSize = (gmod->liquid->tilesX + 1) * (gmod->liquid->tilesY + 1);
                unsigned int flagSize = gmod->liquid->tilesX * gmod->liquid->tilesY;
                chunkSize = 2 * sizeof(unsigned int) + sizeof(Vector3) + heightSize * sizeof(float) + flagSize;
                writer << chunkSize; // Liquid size
                writer << gmod->liquid->tilesX << gmod->liquid->tilesY;
                writer << gmod->liquid->baseCoords.x << gmod->liquid->baseCoords.y << gmod->liquid->baseCoords.z;
                writer << gmod->liquid->type;
                for (int j = 0; j < heightSize; j++)
                {
                    writer << gmod->liquid->height[j];
                }
                for (int j = 0; j < flagSize; j++)
                {
                    writer << gmod->liquid->flags[j];
                }
            }
            else 
            {
                writer << 0; // Liquid size.
            }
        }

        // Group BIH
        writer.writeRaw(_bih.bih.magic, 4);
        AABox bounds = _model->groupTree.getBoundingBox();
        std::vector<unsigned int> tree = _model->groupTree.getTree();
        std::vector<unsigned int> objects = _model->groupTree.getObject();
        writer << bounds.low().x << bounds.low().y << bounds.low().z;
        writer << bounds.high().x << bounds.high().y << bounds.high().z;
        writer << tree << objects; // Poco vector << operator already feed the size.
    }

    stream.close();

    return true;
}
