/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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

#ifndef UTILS_DTOR_H
#define UTILS_DTOR_H

#include <algorithm>
#include <functional>
#include <utility>

template<typename T>
struct dtor : public std::unary_function <T, void>
{
    void operator()(T &ptr) { delete ptr; }
};

template<typename T1, typename T2>
struct dtor<std::pair<T1, T2> > :
public std::unary_function <std::pair<T1, T2>, void>
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
