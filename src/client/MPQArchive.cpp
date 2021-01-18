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

#include "MPQArchive.h"
#include "Poco/String.h"

#include "StormLib.h"
#include "mpq/DBC.h"
#include "mpq/WDT.h"
#include "mpq/ADTV1.h"

void MPQArchive::open()
{
	_logger.debug("Opening archive %s", _path.toString());
	if (SFileOpenArchive((TCHAR*) _path.toString().c_str(), 0, STREAM_FLAG_READ_ONLY, &_mpqHandle))
	{
		_logger.debug("Archive opened!");
	}
	else {
		_logger.error("Error loading the archive: err=%s", GetLastError());
	}
}

void MPQArchive::close()
{
	_logger.debug("Closing archive %s", _path.toString());
	SFileCloseArchive(_mpqHandle);
}

std::string MPQArchive::getName()
{
	return _path.toString();
}

std::vector<std::string> MPQArchive::getFilesList()
{
	SFILE_FIND_DATA findFiles;
	std::vector<std::string> ret;

	HANDLE search = SFileFindFirstFile(_mpqHandle, "*", &findFiles, NULL);

	if (!search)
	{
		_logger.debug("MPQ Archive contains not file");
		return ret;
	}

    int count = 0;
	ret.push_back(findFiles.cFileName);
	while (SFileFindNextFile(search, &findFiles))
	{
        std::string file(findFiles.cFileName);
        if (isAllowedExt(file))
        {
		    ret.push_back(findFiles.cFileName);
            count++;
        }
	}

    if (!count)
    {
        _logger.information("No file loaded from the archive %s", _path.toString());
    }

	SFileFindClose(search);

	return ret;
}

bool MPQArchive::extractFile(std::string file, std::string path)
{
	_logger.debug("Archive: %s, File: %s", _path.toString(), file);
	return SFileExtractFile(_mpqHandle, file.c_str(), (TCHAR*) path.c_str(), SFILE_OPEN_FROM_MPQ);
}

MPQFile* MPQArchive::getFile(std::string file, Version version)
{
	HANDLE handle;

    if (!SFileOpenFileEx(_mpqHandle, file.c_str(), SFILE_OPEN_FROM_MPQ, &handle))
    {
        int error = GetLastError();
        if (error != ERROR_FILE_NOT_FOUND)
        {
            _logger.error("Can't open %s, err=%u!", file, GetLastError());
        }
        return NULL;
    }

    long hi = 0;
    int size = SFileGetFileSize(handle, (DWORD*) &hi);

    if (hi)
    {
        _logger.error("Can't open %s, size[hi] = %u!", file, hi);
        SFileCloseFile(handle);
        return NULL;
    }

    if (size <= 1)
    {
        _logger.error("Can't open %s, size[hi] = %u!", file, size);
        SFileCloseFile(handle);
        return NULL;
    }

    long read = 0;
    char* buffer = new char[size];
    if (!SFileReadFile(handle, buffer, size, (DWORD*) &read, NULL) || size != read)
    {
        _logger.error("Can't read %s, size=%u read=%u!", file, size);
        SFileCloseFile(handle);
        return NULL;
    }

    SFileCloseFile(handle);
    
    if (Poco::endsWith(file, EXT_DBC))
    {
        return new DBC(file, buffer, size);
    }
    else if (Poco::endsWith(file, EXT_WDT))
    {
        return new WDT(file, buffer, size);
    }
    else if (Poco::endsWith(file, EXT_ADT))
    {
        switch (version)
        {
            case Version::CLIENT_CLASSIC:
                return new ADTV1(file, buffer, size);
                break;
        }
    }

    return new MPQFile(file, buffer, size);
}

bool MPQArchive::isAllowedExt(std::string file)
{
    return (Poco::endsWith(file, EXT_DBC) 
        || Poco::endsWith(file, EXT_WDT) 
        || Poco::endsWith(file, EXT_ADT)
        || Poco::endsWith(file, EXT_WMO)
        || Poco::endsWith(file, EXT_M2)
        || Poco::endsWith(file, EXT_MDX)
        );
}