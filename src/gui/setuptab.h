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

#ifndef GUI_SETUPTAB_H
#define GUI_SETUPTAB_H

#include "gui/widgets/container.h"

#include <string>

/**
 * A container for the contents of a tab in the setup window.
 */
class SetupTab : public Container
{
public:
    SetupTab();

    const std::string &getName() const
    { return mName; }

    /**
     * Called when the Apply button is pressed in the setup window.
     */
    virtual void apply() = 0;

    /**
     * Called when the Cancel button is pressed in the setup window.
     */
    virtual void cancel() = 0;

protected:
    /**
     * Sets the name displayed on the tab. Should be set in the
     * constructor of a subclass.
     */
    void setName(const std::string &name)
    { mName = name; }

private:
    std::string mName;
};

#endif
