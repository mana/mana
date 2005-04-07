/*
 *  The Mana World
 *  Copyright 2004-2005 The Mana World Development Team
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
 
#define MINIMAP_W 100
#define MINIMAP_H 100

#include "minimap.h"
#include "../being.h"

Minimap::Minimap()
{
    setContentSize(MINIMAP_W, MINIMAP_H);
    setPosition(20, 20);
    
    mapBackground = SDL_AllocSurface(SDL_SWSURFACE, MINIMAP_W, MINIMAP_H,
            (screen->format->BytesPerPixel*8), 0, 0, 0, 0);
    Uint32 mapColor = SDL_MapRGB(screen->format, 255, 255, 255);
    SDL_Rect sourceRect;
    sourceRect.x = sourceRect.y = 0;
    sourceRect.w = MINIMAP_W;
    sourceRect.h = MINIMAP_H;

    if (mapBackground)
    { 
        SDL_FillRect(mapBackground, &sourceRect, mapColor);
        SDL_SetAlpha(mapBackground, SDL_SRCALPHA, 120);
    }
}

void Minimap::draw(gcn::Graphics *graphics)
{
    int x, y;
    
    getAbsolutePosition(x, y);
    
    if ((mapBackground->w != getWidth()) || (mapBackground->h != getHeight()))
    {
        SDL_FreeSurface(mapBackground);
        mapBackground = SDL_AllocSurface(SDL_SWSURFACE,
                getWidth(), getHeight(), 
                (screen->format->BytesPerPixel * 8), 0, 0, 0, 0);
        Uint32 mapColor = SDL_MapRGB(screen->format, 52, 149, 210);

        if (mapBackground)
        { 
            SDL_Rect sourceRect;
            sourceRect.x = sourceRect.y = 0;
            sourceRect.w = getWidth();
            sourceRect.h = getHeight();
            SDL_FillRect(mapBackground, &sourceRect, mapColor);
            SDL_SetAlpha(mapBackground, SDL_SRCALPHA, 120);
        }
    }

    if (mapBackground)
    {
        SDL_Rect screenRect;
        screenRect.w = getWidth();
        screenRect.h = getHeight();
        screenRect.x = x;
        screenRect.y = y;

        SDL_BlitSurface(mapBackground, NULL, screen, &screenRect);
    }

    graphics->setColor(gcn::Color(0, 0, 0));
    graphics->drawRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));
    graphics->setColor(gcn::Color(209, 52, 61));
    graphics->fillRectangle(gcn::Rectangle(player_node->x / 2,
            player_node->y / 2, 3, 3));
}
