/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#pragma once

#include <string>

class Avatar
{
public:
    Avatar(const std::string &name = std::string());

    /**
     * Returns the avatar's name.
     */
    const std::string &getName() const { return mName; }

    /**
     * Set the avatar's name.
     */
    void setName(const std::string &name) { mName = name; }

    /**
     * Returns the avatar's online status.
     */
    bool getOnline() const { return mOnline; }

    /**
     * Set the avatar's online status.
     */
    void setOnline(bool online) { mOnline = online; }

    int getHp() const { return mHp; }

    void setHp(int hp) { mHp = hp; }

    int getMaxHp() const { return mMaxHp; }

    void setMaxHp(int maxHp) { mMaxHp = maxHp; }

    bool getDisplayBold() const { return mDisplayBold; }

    void setDisplayBold(bool displayBold) { mDisplayBold = displayBold; }

private:
    std::string mName;
    int mHp;
    int mMaxHp;
    bool mOnline;
    bool mDisplayBold;
};
