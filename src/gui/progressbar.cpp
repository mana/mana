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

#include "progressbar.h"
#include "gui.h"


ProgressBar::ProgressBar(float progress, int x, int y, int width, unsigned char red, unsigned green, unsigned char blue)
{
    setProgress(progress);
    Red = red;
    Green = green;
    Blue = blue;
    X = x;
    Y = y;
    Width = width;
}

void ProgressBar::draw(gcn::Graphics *graphics)
{
    int absx, absy;
    getAbsolutePosition(absx, absy);
    
    // outer bar
    int MyColor = makecol(abs(Red-70), abs(Green-70), abs(Blue-70));
    hline(buffer, absx+X+7, absy+Y, absx+X+Width, MyColor);
    hline(buffer, absx+X, absy+Y+7, absx+X+Width-7, MyColor);
    line(buffer, absx+X+7, absy+Y, absx+X, absy+Y+7, MyColor);
    line(buffer, absx+X+Width, absy+Y, absx+X+Width-7, absy+Y+7, MyColor);
    
    // Shadow of outer bar
    MyColor = makeacol(0, 0, 0, 80);
    hline(buffer, absx+X+1, absy+Y+7+1, absx+X+Width-7, MyColor);
    line(buffer, absx+X+Width+1, absy+Y, absx+X+Width-7+1, absy+Y+7, MyColor);
    
    
    // Inner bar
    MyColor = makecol(Red, Green, Blue);
    
    int Temp = 0;
    
    
    for(int i = 1; i < 7; i++)
    {
        Temp = absx+X+int(float(Width)*progress)-i-1;
        if (Temp < (absx + X + 8 - i)) Temp = (absx + X + 8 - i);
        hline(buffer, absx+X+8-i, absy+Y+i, Temp, MyColor);
    }
    
    // Shadow of inner bar
    Temp = absx+X+int(float(Width)*progress)-2;
    if ( Temp < (absx+X+7+1) ) Temp = absx+X+7;
    MyColor = makeacol(abs(Red-40), abs(Green-40), abs(Blue-40), 80);
    hline(buffer, absx+X+7+1, absy+Y+1, Temp, MyColor);
    line(buffer, absx+X+7, absy+Y+1, absx+X+2, absy+Y+7-1, MyColor);

    //rectfill(buffer, absx+7, absy+7, absx+39, absy+9, MyColor);
    
/*
    if (progress != 0) {
        masked_blit(gui_skin.bar.bg.grid[3], buffer,
                0, 0, x, y, buffer->w, buffer->h);
    }
    else {
        masked_blit(gui_skin.bar.bg.grid[0], buffer,
                0, 0, x, y, buffer->w, buffer->h);
    }

    for (int i = 3; i < (w - 3); i++) {
        if (i < progress * w - 3) {
            masked_blit(gui_skin.bar.bg.grid[4], buffer,
                    0, 0, x + 1 * i, y, buffer->w, buffer->h);
        }
        else {
            masked_blit(gui_skin.bar.bg.grid[1], buffer,
                    0, 0, x + 1 * i, y, buffer->w, buffer->h);
        }
    }

    if (progress == 1) {
        masked_blit(gui_skin.bar.bg.grid[5], buffer,
                0, 0, x + w - 3, y, buffer->w, buffer->h);
    }
    else {
        masked_blit(gui_skin.bar.bg.grid[2], buffer,
                0, 0, x + w - 3, y, buffer->w, buffer->h);
    }*/
}

void ProgressBar::setProgress(float progress)
{
    this->progress = progress;
}

float ProgressBar::getProgress()
{
    return progress;
}
