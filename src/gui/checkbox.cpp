/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
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
 *
 *  $Id$
 */

#include "checkbox.h"

CheckBox::CheckBox(const std::string& caption, bool marked):
    gcn::CheckBox(caption, marked)
{
}

void CheckBox::drawBox(gcn::Graphics* graphics) {
    BITMAP *box = NULL;
    int x, y;

    getAbsolutePosition(x, y);

    if (mMarked) {
        if (false /*disabled*/) {
            box = gui_skin.checkbox.disabled_checked;
        } else {
            box = gui_skin.checkbox.checked;
        }
    } else if (false /*disabled*/) {
        box = gui_skin.checkbox.disabled;
    } else {
        box = gui_skin.checkbox.normal;
    }

    x += 2;
    y += 2;

    masked_blit(box, gui_bitmap, 0, 0, x, y, box->w, box->h);
}
