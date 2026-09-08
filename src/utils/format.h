/*
 *  The Mana Client
 *  Copyright (C) 2026  The Mana Developers
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

#include <string>
#include <string_view>

#ifdef USE_FMT
#include <fmt/format.h>
#else
#include <format>
// Makes the std::format facilities available under the same names as those of
// the fmt library, so that either implementation can be used.
namespace fmt = std;
#endif

/**
 * Formats a string using a format string that is only known at runtime, like
 * a translated one.
 *
 * Use fmt::format directly when the format string is a literal, since it is
 * then checked at compile time.
 */
template<typename... Args>
std::string strformat(std::string_view format, Args &&...args)
{
    return fmt::vformat(format, fmt::make_format_args(args...));
}
