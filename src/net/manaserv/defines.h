/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *
 *  This file is part of The Mana Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MANASERV_DEFINES_H
#define MANASERV_DEFINES_H

/**
 * Attributes used during combat. Available to all the beings.
 */
enum
{
BASE_ATTR_BEGIN = 0,
    BASE_ATTR_PHY_ATK_MIN = BASE_ATTR_BEGIN,
    BASE_ATTR_PHY_ATK_DELTA,
                       /**< Physical attack power. */
    BASE_ATTR_MAG_ATK, /**< Magical attack power. */
    BASE_ATTR_PHY_RES, /**< Resistance to physical damage. */
    BASE_ATTR_MAG_RES, /**< Resistance to magical damage. */
    BASE_ATTR_EVADE,   /**< Ability to avoid hits. */
    BASE_ATTR_HIT,     /**< Ability to hit stuff. */
    BASE_ATTR_HP,      /**< Hit Points (Base value: maximum, Modded value: current) */
    BASE_ATTR_HP_REGEN,/**< number of HP regenerated every 10 game ticks */
    BASE_ATTR_END,
    BASE_ATTR_NB = BASE_ATTR_END - BASE_ATTR_BEGIN,

    BASE_ELEM_BEGIN = BASE_ATTR_END,
    BASE_ELEM_NEUTRAL = BASE_ELEM_BEGIN,
    BASE_ELEM_FIRE,
    BASE_ELEM_WATER,
    BASE_ELEM_EARTH,
    BASE_ELEM_AIR,
    BASE_ELEM_SACRED,
    BASE_ELEM_DEATH,
    BASE_ELEM_END,
    BASE_ELEM_NB = BASE_ELEM_END - BASE_ELEM_BEGIN,

    NB_BEING_ATTRIBUTES = BASE_ELEM_END
};

/**
 * Attributes of characters. Used to derive being attributes.
 */
enum
{
    CHAR_ATTR_BEGIN = NB_BEING_ATTRIBUTES,
    CHAR_ATTR_STRENGTH = CHAR_ATTR_BEGIN,
    CHAR_ATTR_AGILITY,
    CHAR_ATTR_DEXTERITY,
    CHAR_ATTR_VITALITY,
    CHAR_ATTR_INTELLIGENCE,
    CHAR_ATTR_WILLPOWER,
    CHAR_ATTR_END,
    CHAR_ATTR_NB = CHAR_ATTR_END - CHAR_ATTR_BEGIN,

    NB_CHARACTER_ATTRIBUTES = CHAR_ATTR_END
};

#endif // MANASERV_DEFINES_H
