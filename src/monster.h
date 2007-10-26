/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#ifndef _TMW_MONSTER_H
#define _TMW_MONSTER_H

#include "being.h"

class MonsterInfo;

class Monster : public Being
{
    public:
        Monster(Uint32 id, Uint16 job, Map *map);

        virtual void logic();

        virtual void setAction(Uint8 action);

        virtual Type getType() const;

        virtual TargetCursorSize
        getTargetCursorSize() const;

        /**
         * Handles an attack of another being by this monster. Plays a hit or
         * miss sound when appropriate.
         *
         * @param victim The attacked being.
         * @param damage The amount of damage dealt (0 means miss).
         */
        virtual void handleAttack(Being *victim, int damage);

        /**
         * Puts a damage bubble above this monster and plays the hurt sound
         *
         * @param amount The amount of damage.
         */
        virtual void takeDamage(int amount);

        /**
         * Returns the MonsterInfo, with static data about this monster.
         */
        const MonsterInfo&
        getInfo() const;
};

#endif
