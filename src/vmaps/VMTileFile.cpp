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

#include "VMTileFile.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/BinaryWriter.h"
#include "Poco/FileStream.h"

using Poco::Path;
using Poco::File;
using Poco::BinaryWriter;
using Poco::FileOutputStream;

VMTileFile::VMTileFile(unsigned int mapId, unsigned int tileX, unsigned int tileY, std::vector<ModelInstance*> instances) : _mapId(mapId), _tileX(tileX), _tileY(tileY)
{
    _data.instances = instances;
}

VMTileFile::~VMTileFile()
{
}

bool VMTileFile::save(std::string path)
{
    char vmTileFile[1024];
    sprintf(vmTileFile, PATTERN_FILE.c_str(), _mapId, _tileY, _tileX); // Tile indices are reversed.
    Path filePath(path + "/" + vmTileFile);
    File file(filePath);
    file.createFile();

    FileOutputStream stream(filePath.toString());
    BinaryWriter writer(stream);

    // Writing the header.
    writer.writeRaw(_data.magic, 8);

    // Writing the model instance data.
    unsigned int nInstances = _data.instances.size();
    writer << nInstances;

    for (int i = 0; i < nInstances; i++)
    {
        writer << _data.instances[i]->model->flags;
        writer << _data.instances[i]->adtId << _data.instances[i]->id
            << _data.instances[i]->position.x << _data.instances[i]->position.y << _data.instances[i]->position.z
            << _data.instances[i]->rotation.x << _data.instances[i]->rotation.y << _data.instances[i]->rotation.z
            << _data.instances[i]->scale;
        if (_data.instances[i]->model->flags & MOD_HAS_BOUND)
        {
            writer << _data.instances[i]->boundingBox.low().x << _data.instances[i]->boundingBox.low().y << _data.instances[i]->boundingBox.low().z;
            writer << _data.instances[i]->boundingBox.high().x << _data.instances[i]->boundingBox.high().y << _data.instances[i]->boundingBox.high().z;
        }
        writer << (unsigned int) _data.instances[i]->model->name.length();
        writer.writeRaw(_data.instances[i]->model->name.c_str(), _data.instances[i]->model->name.length());

        // MapTree nodes to update when loading tile.
        writer << _data.instances[i]->nodeIdx;
    }

    stream.close();

    return true;
}
