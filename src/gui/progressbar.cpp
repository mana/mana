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
#include "../graphics.h"
#include "../resources/resourcemanager.h"

ProgressBar::ProgressBar(float progress, int x, int y, int width, int height,
        int red, int green, int blue):
    gcn::Widget(),
    red(red), green(green), blue(blue)
{
    setProgress(progress);
    setX(x);
    setY(y);
    setWidth(width);
    setHeight(height);
    redToGo = red;
    greenToGo = green;
    blueToGo = blue;

    // Load dialog title bar image
    ResourceManager *resman = ResourceManager::getInstance();
    Image *dBorders = resman->getImage("graphics/gui/vscroll_grey.png");
    dBackground = resman->getImage("graphics/gui/bg_quad_dis.png");

    dTopBorder = dBorders->getSubImage(4, 0, 3, 4);
    dLeftBorder = dBorders->getSubImage(0, 4, 4, 10);
    dRightBorder = dBorders->getSubImage(7, 4, 4, 10);
    dBottomBorder = dBorders->getSubImage(4, 15, 3, 4);

    dTopLeftBorder = dBorders->getSubImage(0, 0, 4, 4);
    dTopRightBorder = dBorders->getSubImage(7, 0, 4, 4);
    dBottomRightBorder = dBorders->getSubImage(7, 15, 4, 4);
    dBottomLeftBorder = dBorders->getSubImage(0, 15, 4, 4);
}

ProgressBar::~ProgressBar()
{
}

void ProgressBar::logic()
{
    // Smoothly changing the color for a nicer effect.
    if (redToGo > red) red++;
    if (redToGo < red) red--;
    if (greenToGo > green) green++;
    if (greenToGo < green) green--;
    if (blueToGo > blue) blue++;
    if (blueToGo < blue) blue--;
}

void ProgressBar::draw(gcn::Graphics *graphics)
{
    int absx, absy;
    getAbsolutePosition(absx, absy);

    // Background
    dBackground->drawPattern(screen,
            absx + 4, absy + 4,
            getWidth() - 8, getHeight() - 8);

    // The corners
    dTopLeftBorder->draw(screen, absx, absy);
    dTopRightBorder->draw(screen, absx + getWidth() - 4, absy);
    dBottomLeftBorder->draw(screen, absx, absy + getHeight() - 4);
    dBottomRightBorder->draw(screen,
            absx+getWidth() - 4, absy+getHeight() - 4);

    // The borders
    dTopBorder->drawPattern(screen, absx + 4, absy, getWidth() - 8, 4);
    dBottomBorder->drawPattern(screen, absx + 4, absy + getHeight() - 4,
            getWidth() - 8, 4);
    dLeftBorder->drawPattern(screen, absx, absy + 4, 4, getHeight() - 8);
    dRightBorder->drawPattern(screen, absx + getWidth() - 4, absy + 4,
            4, getHeight() - 8);

    // The bar
    if (progress > 0)
    {
        graphics->setColor(gcn::Color(red, green, blue, 200));
        graphics->fillRectangle(gcn::Rectangle(4, 4,
                    (int)(progress * (getWidth() - 8)), getHeight() - 8));
    }
}

void ProgressBar::setProgress(float progress)
{
    if (progress < 0.0f) this->progress = 0.0;
    else if (progress > 1.0f) this->progress = 1.0;
    else this->progress = progress;
}

float ProgressBar::getProgress()
{
    return progress;
}

void ProgressBar::setColor(int red, int green, int blue)
{
    this->redToGo = red;
    this->greenToGo = green;
    this->blueToGo = blue;
}
