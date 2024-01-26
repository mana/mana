/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#ifndef UTILS_DTOR_H
#define UTILS_DTOR_H

#include <algorithm>
#include <utility>

template<typename T>
struct dtor
{
    void operator()(T &ptr) { delete ptr; }
};

template<typename T1, typename T2>
struct dtor<std::pair<T1, T2>>
{
    void operator()(std::pair<T1, T2> &pair) { delete pair.second; }
};

template<class Cont>
inline dtor<typename Cont::value_type> make_dtor(Cont const&)
{
    return dtor<typename Cont::value_type>();
}

template<typename Container>
inline void delete_all(Container &c)
{
    std::for_each(c.begin(), c.end(), make_dtor(c));
}

#endif
