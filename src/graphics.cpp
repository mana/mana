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

#include "graphics.h"
#include "log.h"
#include "resources/resourcemanager.h"
#include "main.h"

SDL_Surface *screen;


Graphics::Graphics():
    mouseCursor(NULL)
{
#ifdef USE_OPENGL
    if (useOpenGL) {
        // Setup OpenGL
        glViewport(0, 0, 800, 600);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        int gotDoubleBuffer;
        SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &gotDoubleBuffer);
        logger->log("Using OpenGL %s double buffering.",
                (gotDoubleBuffer ? "with" : "without"));
    }
#endif

#ifdef USE_OPENGL
    setTargetPlane(800, 600);
#else
    setTarget(SDL_GetVideoSurface());
#endif

    // Hide the system mouse cursor
    SDL_ShowCursor(SDL_DISABLE);

    // Load the mouse cursor
    ResourceManager *resman = ResourceManager::getInstance();
    mouseCursor = resman->getImage("graphics/gui/mouse.png", IMG_ALPHA);
    if (!mouseCursor) {
        logger->error("Unable to load mouse cursor.");
    }

    // Initialize for drawing
    _beginDraw();
}

Graphics::~Graphics()
{
    // Deinitialize for drawing
    _endDraw();
}

int Graphics::getWidth()
{
    return screen->w;
}

int Graphics::getHeight()
{
    return screen->h;
}

void Graphics::drawImageRect(
        int x, int y, int w, int h,
        Image *topLeft, Image *topRight,
        Image *bottomLeft, Image *bottomRight,
        Image *top, Image *right,
        Image *bottom, Image *left,
        Image *center)
{
    // Draw the center area
    center->drawPattern(screen,
            x + topLeft->getWidth(), y + topLeft->getHeight(),
            w - topLeft->getWidth() - topRight->getWidth(),
            h - topLeft->getHeight() - bottomLeft->getHeight());

    // Draw the sides
    top->drawPattern(screen,
            x + topLeft->getWidth(), y,
            w - topLeft->getWidth() - topRight->getWidth(), top->getHeight());
    bottom->drawPattern(screen,
            x + bottomLeft->getWidth(), y + h - bottom->getHeight(),
            w - bottomLeft->getWidth() - bottomRight->getWidth(),
            bottom->getHeight());
    left->drawPattern(screen,
            x, y + topLeft->getHeight(),
            left->getWidth(),
            h - topLeft->getHeight() - bottomLeft->getHeight());
    right->drawPattern(screen,
            x + w - right->getWidth(), y + topRight->getHeight(),
            right->getWidth(),
            h - topRight->getHeight() - bottomRight->getHeight());

    // Draw the corners
    topLeft->draw(screen, x, y);
    topRight->draw(screen, x + w - topRight->getWidth(), y);
    bottomLeft->draw(screen, x, y + h - bottomLeft->getHeight());
    bottomRight->draw(screen,
            x + w - bottomRight->getWidth(),
            y + h - bottomRight->getHeight());
}

void Graphics::drawImageRect(
        int x, int y, int w, int h,
        const ImageRect &imgRect)
{
    drawImageRect(x, y, w, h,
            imgRect.grid[0], imgRect.grid[2], imgRect.grid[6], imgRect.grid[8],
            imgRect.grid[1], imgRect.grid[5], imgRect.grid[7], imgRect.grid[3],
            imgRect.grid[4]);
}

void Graphics::updateScreen()
{
    int mouseX, mouseY;
    Uint8 button = SDL_GetMouseState(&mouseX, &mouseY);

    if (SDL_GetAppState() & SDL_APPMOUSEFOCUS || button & SDL_BUTTON(1))
    {
        // Draw mouse before flipping
        mouseCursor->draw(screen, mouseX - 5, mouseY - 2);
    }

#ifdef USE_OPENGL
    if (useOpenGL) {
        glFlush();
        glFinish();
        SDL_GL_SwapBuffers();
    }
    else {
        SDL_Flip(screen);
    }
#else
    SDL_Flip(screen);
#endif

    // Decrement frame counter when using framerate limiting
    if (framesToDraw > 1) framesToDraw--;

    // Wait while we're not allowed to draw next frame yet
    while (framesToDraw == 1)
    {
        SDL_Delay(10);
    }
}
