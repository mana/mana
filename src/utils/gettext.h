/*
 *  Aethyra
 *  Copyright (C) 2007  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef UTILS_GETTEXT_H
#define UTILS_GETTEXT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if ENABLE_NLS

#include <libintl.h>

#define _(s) ((char const *)gettext(s))
#define N_(s) ((char const *)s)

#else

#define gettext(s) ((char const *)s)
#define _(s) ((char const *)s)
#define N_(s) ((char const *)s)

#endif

#endif
