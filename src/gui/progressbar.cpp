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
 */

#include "progressbar.h"
#include "gui.h"


ProgressBar::ProgressBar(float progress)
{
    setProgress(progress);
}

void ProgressBar::draw(gcn::Graphics *graphics)
{
    int x, y, w, h;
    getAbsolutePosition(x, y);
    w = getWidth();
    h = getHeight();

    if (progress != 0) {
        masked_blit(gui_skin.bar.bg.grid[3], gui_bitmap,
                0, 0, x, y, gui_bitmap->w, gui_bitmap->h);
    }
    else {
        masked_blit(gui_skin.bar.bg.grid[0], gui_bitmap,
                0, 0, x, y, gui_bitmap->w, gui_bitmap->h);
    }

    for (int i = 3; i < (w - 3); i++) {
        if (i < progress * w - 3) {
            masked_blit(gui_skin.bar.bg.grid[4], gui_bitmap,
                    0, 0, x + 1 * i, y, gui_bitmap->w, gui_bitmap->h);
        }
        else {
            masked_blit(gui_skin.bar.bg.grid[1], gui_bitmap,
                    0, 0, x + 1 * i, y, gui_bitmap->w, gui_bitmap->h);
        }
    }

    if (progress == 1) {
        masked_blit(gui_skin.bar.bg.grid[5], gui_bitmap,
                0, 0, x + w - 3, y, gui_bitmap->w, gui_bitmap->h);
    }
    else {
        masked_blit(gui_skin.bar.bg.grid[2], gui_bitmap,
                0, 0, x + w - 3, y, gui_bitmap->w, gui_bitmap->h);
    }
}

void ProgressBar::setProgress(float progress)
{
    this->progress = progress;
}

float ProgressBar::getProgress()
{
    return progress;
}
