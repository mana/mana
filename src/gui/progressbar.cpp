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
#include "../resources/resourcemanager.h"

ProgressBar::ProgressBar(float progress, int x, int y, int width, int height, unsigned char red, unsigned green, unsigned char blue) :
gcn::Widget()
{
    setProgress(progress);
    Red = red;
    Green = green;
    Blue = blue;
    setX(x);
    setY(y);
    setWidth(width);
    setHeight(height);
    
    // Load dialog title bar image
    ResourceManager *resman = ResourceManager::getInstance();
    Image *dBorders = resman->getImage("core/graphics/gui/vscroll_grey.png");
    dBackground = resman->getImage("core/graphics/gui/bg_quad_dis.png");

    dTopBorder = dBorders->getSubImage(4, 0, 3, 4);
    dLeftBorder = dBorders->getSubImage(0, 4, 4, 10);
    dRightBorder = dBorders->getSubImage(7, 4, 4, 10);
    dBottomBorder = dBorders->getSubImage(4, 15, 3, 4);
    
    dTopLeftBorder = dBorders->getSubImage(0, 0, 4, 4);
    dTopRightBorder = dBorders->getSubImage(7, 0, 4, 4);
    dBottomRightBorder = dBorders->getSubImage(7, 15, 4, 4);
    dBottomLeftBorder = dBorders->getSubImage(0, 15, 4, 4);
    
    dBackground->setAlpha(1.0f);
    
    dTopBorder->setAlpha(1.0f);
    dBottomBorder->setAlpha(1.0f);
    dLeftBorder->setAlpha(1.0f);
    dRightBorder->setAlpha(1.0f);
    
    dTopLeftBorder->setAlpha(1.0f);
    dTopRightBorder->setAlpha(1.0f);
    dBottomLeftBorder->setAlpha(1.0f);
    dBottomRightBorder->setAlpha(1.0f);
    
    // The color bar
    ColorBar = SDL_CreateRGBSurface(SDL_SWSURFACE, width-8, height-8, 32,
            red, green, blue, 160);
    
}

ProgressBar::~ProgressBar()
{
    #ifndef USE_OPENGL
    SDL_FreeSurface(ColorBar);
    #endif
}

void ProgressBar::draw(gcn::Graphics *graphics)
{
#ifndef USE_OPENGL
    int absx, absy;
    getAbsolutePosition(absx, absy);
    
    // We're drawing the bar itself first
    // Background
    dBackground->drawPattern(screen, absx+4, absy+4, getWidth()-8, getHeight()-8);
    // The corners
    dTopLeftBorder->draw(screen, absx, absy);
    dTopRightBorder->draw(screen, absx+getWidth()-4, absy);
    dBottomLeftBorder->draw(screen, absx, absy+getHeight()-4);
    dBottomRightBorder->draw(screen, absx+getWidth()-4, absy+getHeight()-4);
    
    // The borders
    dTopBorder->drawPattern(screen, absx+4, absy, getWidth()-8, 4);
    dBottomBorder->drawPattern(screen, absx+4, absy+getHeight()-4, getWidth()-8, 4);
    dLeftBorder->drawPattern(screen, absx, absy+4, 4, getHeight()-8);
    dRightBorder->drawPattern(screen, absx+getWidth()-4, absy+4, 4, getHeight()-8);

    // And then, we color the bar to show the progress
    SDL_Rect srcRect, destRect;
    destRect.x = absx+4;
    destRect.y = absy+4;
    srcRect.x = 0;
    srcRect.y = 0;
    srcRect.w = getWidth();
    srcRect.h = getHeight();

    if ( ColorBar )
    {
        if ( ColorBar->w < (srcRect.w)) srcRect.w=ColorBar->w;
        if ( ColorBar->h < (srcRect.h)) srcRect.h=ColorBar->h;
        SDL_BlitSurface(ColorBar, &srcRect, screen, &destRect);
    }
    
#endif
}

void ProgressBar::setProgress(float progress)
{
    this->progress = progress;
}

float ProgressBar::getProgress()
{
    return progress;
}
