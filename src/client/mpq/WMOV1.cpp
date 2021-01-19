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

#include "WMOV1.h"

WMOV1::WMOV1(std::string name, unsigned char* data, long size)
{
	_name = name;
	_size = size;
	_data = data;
}

WMOV1::~WMOV1()
{
	delete _data;
}

bool WMOV1::parse()
{
	_logger.information("Parsing WMO file %s", _name);

	unsigned int offset = 0;

	_version = (Version*)(_data);

	if (!checkHeader(_version->magic, HEADER_MVER))
	{
		return false;
	}

	if (_version->version != SUPPORTED_VERSION)
	{
		_logger.error("Expected file version %u, got %i", SUPPORTED_VERSION, _version->version);
		return false;
	}

	offset = 8 + _version->size;

	_header = (Header*)(_data + offset);

	if (!checkHeader(_header->magic, HEADER_MOHD))
	{
		return false;
	}

	offset += 8 + _header->size;

	MOTX* motx = (MOTX*)(_data + offset);

	if (!checkHeader(motx->magic, HEADER_MOTX))
	{
		return false;
	}

	_offsetMOTX = offset;

	offset += 8 + motx->size;

	MOMT* momt = (MOMT*)(_data + offset);

	if (!checkHeader(momt->magic, HEADER_MOMT))
	{
		return false;
	}

	_offsetMOMT = offset;

	offset += 8 + momt->size;

	MOGN* mogn = (MOGN*)(_data + offset);

	if (!checkHeader(mogn->magic, HEADER_MOGN))
	{
		return false;
	}

	_offsetMOGN = offset;

	offset += 8 + mogn->size;

	MOGI* mogi = (MOGI*)(_data + offset);

	if (!checkHeader(mogi->magic, HEADER_MOGI))
	{
		return false;
	}

	_offsetMOGI = offset;

	offset += 8 + mogi->size;

	MOSB* mosb = (MOSB*)(_data + offset);

	if (!checkHeader(mosb->magic, HEADER_MOSB))
	{
		return false;
	}

	_offsetMOSB = offset;

	offset += 8 + mosb->size;

	MOPV* mopv = (MOPV*)(_data + offset);

	if (!checkHeader(mopv->magic, HEADER_MOPV))
	{
		return false;
	}

	_offsetMOPV = offset;

	offset += 8 + mopv->size;

	MOPT* mopt = (MOPT*)(_data + offset);

	if (!checkHeader(mopt->magic, HEADER_MOPT))
	{
		return false;
	}

	_offsetMOPT = offset;

	offset += 8 + mopt->size;

	MOPR* mopr = (MOPR*)(_data + offset);

	if (!checkHeader(mopr->magic, HEADER_MOPR))
	{
		return false;
	}

	_offsetMOPR = offset;

	offset += 8 + mopr->size;

	MOVV* movv = (MOVV*)(_data + offset);

	if (!checkHeader(movv->magic, HEADER_MOVV))
	{
		return false;
	}

	_offsetMOVV = offset;

	offset += 8 + movv->size;

	MOVB* movb = (MOVB*)(_data + offset);

	if (!checkHeader(movb->magic, HEADER_MOVB))
	{
		return false;
	}

	_offsetMOVB = offset;

	offset += 8 + movb->size;

	MOLT* molt = (MOLT*)(_data + offset);

	if (!checkHeader(molt->magic, HEADER_MOLT))
	{
		return false;
	}

	_offsetMOLT = offset;

	offset += 8 + molt->size;

	MODS* mods = (MODS*)(_data + offset);

	if (!checkHeader(mods->magic, HEADER_MODS))
	{
		return false;
	}

	_offsetMODS = offset;

	offset += 8 + mods->size;

	MODN* modn = (MODN*)(_data + offset);

	if (!checkHeader(modn->magic, HEADER_MODN))
	{
		return false;
	}

	_offsetMODN = offset;

	offset += 8 + modn->size;

	MODD* modd = (MODD*)(_data + offset);

	if (!checkHeader(modd->magic, HEADER_MODD))
	{
		return false;
	}

	_offsetMODD = offset;

	offset += 8 + modd->size;

	MFOG* mfog = (MFOG*)(_data + offset);

	if (!checkHeader(mfog->magic, HEADER_MFOG))
	{
		return false;
	}

	_offsetMFOG = offset;

	offset += 8 + mfog->size;

	MCVP* mcvp = (MCVP*)(_data + offset);

	if (!checkOptionalHeader(mcvp->magic, HEADER_MCVP))
	{
		_offsetMCVP = 0;
	}
	else {
		_offsetMCVP = offset;
	}

	return true;
}