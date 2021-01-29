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

#include "VMTreeFile.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/BinaryWriter.h"
#include "Poco/FileStream.h"

using Poco::Path;
using Poco::File;
using Poco::BinaryWriter;
using Poco::FileOutputStream;

VMTreeFile::VMTreeFile(unsigned int mapId, BIH* tree, ModelInstance* globalWMO) : _mapId(mapId)
{
    _header.isTiled = (globalWMO ? false : true);
    _node.tree = tree;
    _globalWMO.model = globalWMO;
}

VMTreeFile::~VMTreeFile()
{
}

bool VMTreeFile::save(std::string path)
{
    char vmTreeFile[1024];
    sprintf(vmTreeFile, PATTERN_FILE.c_str(), _mapId);
    Path filePath(path + "/" + vmTreeFile);
    File file(filePath);
    file.createFile();

    FileOutputStream stream(filePath.toString());
    BinaryWriter writer(stream);

    // Writing the header.
    writer.writeRaw(_header.magic, 8);
    writer << _header.isTiled;

    // Writing the node data.
    writer.writeRaw(_node.magic, 4);
    AABox bounds = _node.tree->getBoundingBox();
    std::vector<unsigned int> tree = _node.tree->getTree();
    std::vector<unsigned int> objects = _node.tree->getObject();
    writer << bounds.low().x << bounds.low().y << bounds.low().z;
    writer << bounds.high().x << bounds.high().y << bounds.high().z;
    writer << tree << objects; // Poco vector << operator already feed the size.

    // Writing the global OBJ.
    writer.writeRaw(_globalWMO.magic, 4);
    if (_globalWMO.model)
    {
        writer << _globalWMO.model->model->flags;
        writer << _globalWMO.model->adtId << _globalWMO.model->id 
            << _globalWMO.model->position.x << _globalWMO.model->position.y << _globalWMO.model->position.z
            << _globalWMO.model->rotation.x << _globalWMO.model->rotation.y << _globalWMO.model->rotation.z
            << _globalWMO.model->scale;
        if (_globalWMO.model->model->flags & MOD_HAS_BOUND)
        {
            writer << _globalWMO.model->boundingBox.low().x << _globalWMO.model->boundingBox.low().y << _globalWMO.model->boundingBox.low().z;
            writer << _globalWMO.model->boundingBox.high().x << _globalWMO.model->boundingBox.high().y << _globalWMO.model->boundingBox.high().z;
        }
        writer << _globalWMO.model->model->name;
    }

    stream.close();

    return true;	
}
