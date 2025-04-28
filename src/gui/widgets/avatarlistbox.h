/*
 *  The Mana Client
 *  Copyright (C) 2010-2012  The Mana Developers
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

#include "avatar.h"

#include "gui/widgets/listbox.h"

#include <string>

class Image;

class AvatarListModel : public gcn::ListModel
{
public:
    virtual Avatar *getAvatarAt(int i) = 0;

    std::string getElementAt(int i) override
    { return getAvatarAt(i)->getName(); }
};

class AvatarListBox : public ListBox
{
public:
    AvatarListBox(AvatarListModel *model);

    unsigned int getRowHeight() const override;

    /**
     * Draws the list box.
     */
    void draw(gcn::Graphics *gcnGraphics) override;

    void mousePressed(gcn::MouseEvent &event) override;
};
