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

#include "CAaBspNode.h"

CAaBspNode::CAaBspNode() : flags(0), negChild(0), posChild(0), nFaces(0), faceStart(0), planeDist(0)
{
}

CAaBspNode::CAaBspNode(unsigned short flags, unsigned short negChild, unsigned short posChild, unsigned short nFaces, unsigned int faceStart, float planeDist) : 
	flags(flags), negChild(negChild), posChild(posChild), nFaces(nFaces), faceStart(faceStart), planeDist(planeDist)
{
}

std::ostream& operator<<(std::ostream& stream, const CAaBspNode& node)
{
	stream << node.flags << node.negChild << node.posChild << node.nFaces << node.faceStart << node.planeDist;
	return stream;
}

std::istream& operator>>(std::istream& stream, CAaBspNode& node)
{
	stream >> node.flags >> node.negChild >> node.posChild >> node.nFaces >> node.faceStart >> node.planeDist;
	return stream;
}
