/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef XEEN_MAZE_MONSTERDATA_H
#define XEEN_MAZE_MONSTERDATA_H

#ifndef XEEN_MAZE_SOURCE
# error "Private header included"
#endif

#include "xeen/utility.h"

namespace XEEN
{
    namespace Maze
    {
        class Manager;
        class Map;

        // Only accessible by Map and Manager
        class MonsterData : public Validateable, public Common::NonCopyable
        {
            friend class Manager;
            friend class Map;

            static const uint32 MAX_MONSTERS = 90;
            static const uint32 DAT_SIZE = MAX_MONSTERS * 60;
        
            private:
                MonsterData(Valid<Manager> parent);
                
                NonNull<const char> getName(uint32 id) const;

            private:
                Valid<Manager> _parent;

                byte _xeenMON[DAT_SIZE];
                byte _darkMON[DAT_SIZE];
        };
    }
}

#endif // XEEN_MAZE_MONSTERDATA_H
