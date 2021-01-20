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

#include "WMOFile.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/BinaryWriter.h"
#include "Poco/FileStream.h"

using Poco::Path;
using Poco::File;
using Poco::BinaryWriter;
using Poco::FileOutputStream;

WMOFile::WMOFile(std::string directory, std::string hash, std::string filename) : _directory(directory), _hash(hash), _filename(filename)
{
}

WMOFile::~WMOFile()
{
}

bool WMOFile::save(std::string path)
{
    char wmoFile[1024];
    sprintf(wmoFile, PATTERN_WMO.c_str(), _hash.c_str(), _filename.c_str());
    Path wmoPath(path + "/" + wmoFile);
    File file(wmoPath);
    file.createFile();

    FileOutputStream stream(wmoPath.toString());
    BinaryWriter writer(stream);

    // Writing the header.
    writer.writeRaw(header.magic, 4);
    writer.writeRaw(header.versionMagic, 4);
    writer << header.nVectors << header.nGroups << header.rootWMOID;

    stream.close();

    return true;
}
