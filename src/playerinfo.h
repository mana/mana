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

#ifndef _TMW_PLAYERINFO_
#define _TMW_PLAYERINFO_

#include <string>

struct PLAYER_INFO
{
    int id;
    float lastAttackTime;      /**< Used to synchronize the charge dialog */
    std::string name;          /**< Player name */
    short hp, maxHp, mp, maxMp, lvl;
    short statsPointsToAttribute;
    int xp, xpForNextLevel, gp, jobXp, jobXpForNextLevel, jobLvl;
    short statPoint, skillPoint, hairColor, hairStyle;
    char STR, AGI, VIT, INT, DEX, LUK;
    char STRUp, AGIUp, VITUp, INTUp, DEXUp, LUKUp;
    char ATK, ATKBonus, MATK, MATKBonus, DEF, DEFBonus, MDEF;
    char MDEFBonus, HIT, FLEE, FLEEBonus;
    char characterNumber;
    int totalWeight, maxWeight;
    short weapon;
};

extern PLAYER_INFO **char_info;
extern PLAYER_INFO *player_info;

#endif
