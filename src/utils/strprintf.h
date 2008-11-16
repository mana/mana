/*
 *  The Mana World
 *  Copyright 2007 The Mana World Development Team
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
 */

#ifndef _TMW_UTILS_STRPRINTF_H
#define _TMW_UTILS_STRPRINTF_H

#include <string>

std::string strprintf(char const *, ...)
#ifdef __GNUC__
    /* This attribute is nice: it even works through gettext invokation. For
       example, gcc will complain that strprintf(_("%s"), 42) is ill-formed. */
    __attribute__((format(printf, 1, 2)))
#endif
;

#endif
