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

#include "ModelFile.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/BinaryWriter.h"
#include "Poco/FileStream.h"

using Poco::Path;
using Poco::File;
using Poco::BinaryWriter;
using Poco::FileOutputStream;

ModelFile::ModelFile(std::string directory, std::string hash, std::string filename) : _directory(directory), _hash(hash), _filename(filename)
{
}

ModelFile::~ModelFile()
{
    for (int i = 0; i < header.nGroups; i++)
    {
        if (groups[i].header.mobaBatch)
        {
            delete groups[i].header.mobaEx;
        }
        if (groups[i].vertices.nVertices)
        {
            delete groups[i].vertices.vertices;
        }
        if (groups[i].indices.nIndices)
        {
            delete groups[i].indices.indices;
        }
        if(groups[i].liquid.height)
        {
            delete groups[i].liquid.height;
        }
        if(groups[i].liquid.flags)
        {
            delete groups[i].liquid.flags;
        }
    }
    delete groups;
}

bool ModelFile::save(std::string path)
{
    char ModelFile[1024];
    sprintf(ModelFile, PATTERN_FILE.c_str(), _hash.c_str(), _filename.c_str());
    Path wmoPath(path + "/" + ModelFile);
    File file(wmoPath);
    file.createFile();

    FileOutputStream stream(wmoPath.toString());
    BinaryWriter writer(stream);

    // Writing the header.
    writer.writeRaw(header.magic, 4);
    writer.writeRaw(header.versionMagic, 4);
    writer << header.nVectors << header.nGroups << header.rootWMOID;

    // Writing the groups.
    for (int i = 0; i < header.nGroups; i++)
    {
        writer << groups[i].flags << groups[i].groupWMOID
            << groups[i].boundingBox.min.x << groups[i].boundingBox.min.y << groups[i].boundingBox.min.z
            << groups[i].boundingBox.max.x << groups[i].boundingBox.max.y << groups[i].boundingBox.max.z
            << groups[i].liquidFlags;

        // Group Header.
        writer.writeRaw(groups[i].header.magic, 4);
        writer << groups[i].header.mobaSize << groups[i].header.mobaBatch;
        for(int j = 0; j < (groups[i].header.mobaBatch); j++)
        {
            writer << groups[i].header.mobaEx[j];
        }

        // Indices.
        writer.writeRaw(groups[i].indices.magic, 4);
        writer << groups[i].indices.size << groups[i].indices.nIndices;
        for (int j = 0; j < groups[i].indices.nIndices; j++)
        {
            writer << groups[i].indices.indices[j];
        }

        // Vertices.
        writer.writeRaw(groups[i].vertices.magic, 4);
        writer << groups[i].vertices.size << groups[i].vertices.nVertices;
        for (int j = 0; j < groups[i].vertices.nVertices; j++)
        {
            writer << groups[i].vertices.vertices[j].x << groups[i].vertices.vertices[j].y << groups[i].vertices.vertices[j].z;
        }

        // Liquid.
        if (groups[i].liquidFlags)
        {
            writer.writeRaw(groups[i].liquid.magic, 4);
            writer << groups[i].liquid.size;

            writer << groups[i].liquid.xVerts << groups[i].liquid.yVerts
                << groups[i].liquid.xTiles << groups[i].liquid.yTiles
                << groups[i].liquid.baseCoords.x << groups[i].liquid.baseCoords.y << groups[i].liquid.baseCoords.z
                << groups[i].liquid.type;

            // Liquid vertices
            for (int j = 0; j < (groups[i].liquid.xVerts * groups[i].liquid.yVerts); j++)
            {
                writer << groups[i].liquid.height[j];
            }

            // Liquid flags for tiles
            for (int j = 0; j < (groups[i].liquid.xTiles * groups[i].liquid.yTiles); j++)
            {
                writer << groups[i].liquid.flags[j];
            }
        }
    }

    stream.close();

    return true;
}

std::string ModelFile::getFilename()
{
    return _filename;
}