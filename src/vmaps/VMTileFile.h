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

#ifndef VMTILEFILE_H
#define VMTILEILE_H

#include "Poco/Logger.h"
#include "Model.h"

using Poco::Logger;

class VMTileFile {
public:
	VMTileFile(unsigned int mapId, unsigned int tileX, unsigned int tileY, std::vector<ModelInstance*> instances);
	~VMTileFile();

	bool save(std::string path);

private:
	const std::string PATTERN_FILE = "%03u_%02u_%02u.vmtile";
	Logger& _logger = Logger::get("Extractor");

	const unsigned int _mapId;
	const unsigned int _tileX;
	const unsigned int _tileY;
	struct {
		const char magic[8] = { 'V', 'M', 'A', 'P', '_', '4', '.', '0' };
		std::vector<ModelInstance*> instances;
	} _data;
};

#endif