/*
 *  The Mana Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// The fallback macros below would mangle the declarations in <libintl.h>, so
// the header has to come first wherever it exists. Platforms without it (a
// gettext-less toolchain) fall through to the macros.
#if __has_include(<libintl.h>)
#include <libintl.h>
#endif

#if ENABLE_NLS

#define _(s) ((char const *)gettext(s))
#define N_(s) ((char const *)s)

#else

#define gettext(s) ((char const *)s)
#define ngettext(s1, s2, n) ((char const *)((n) == 1 ? (s1) : (s2)))
#define _(s) ((char const *)s)
#define N_(s) ((char const *)s)

#endif
