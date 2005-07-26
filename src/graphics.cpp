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
#include "resources/resourcemanager.h"
#include "main.h"
#include "log.h"

extern volatile int framesToDraw;


Graphics::Graphics(SDL_Surface *screen):
    mScreen(screen)
{
    if (useOpenGL) {
#ifdef USE_OPENGL
        // Setup OpenGL
        glViewport(0, 0, 800, 600);
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
        int gotDoubleBuffer;
        SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &gotDoubleBuffer);
        logger->log("Using OpenGL %s double buffering.",
                (gotDoubleBuffer ? "with" : "without"));

        setTargetPlane(800, 600);
#endif
    }
    else {
        setTarget(mScreen);
    }

    // Initialize for drawing
    if (useOpenGL) {
        gcn::OpenGLGraphics::_beginDraw() ;
    }
    else {
        gcn::SDLGraphics::_beginDraw();
    }
    //_beginDraw();
}

Graphics::~Graphics()
{
    // Deinitialize for drawing
    if (useOpenGL) {
        gcn::OpenGLGraphics::_endDraw() ;
    }
    else {
        gcn::SDLGraphics::_endDraw();
    }
    //_endDraw();
}

void Graphics::setFont(gcn::ImageFont *font)
{
    if (useOpenGL) {
        gcn::OpenGLGraphics::setFont(font);
    }
    else {
        gcn::SDLGraphics::setFont(font);
    }
}

void Graphics::drawText(const std::string &text,
		    int x,
		    int y,
		    unsigned int alignment)
{
    if (useOpenGL) {
        gcn::OpenGLGraphics::drawText(text, x, y, alignment);
    }
    else {
        gcn::SDLGraphics::drawText(text, x, y, alignment);
    }
}

void Graphics::setColor(gcn::Color color)
{
    if (useOpenGL) {
        gcn::OpenGLGraphics::setColor(color);
    }
    else {
        gcn::SDLGraphics::setColor(color);
    }
}

int Graphics::getWidth()
{
    return mScreen->w;
}

int Graphics::getHeight()
{
    return mScreen->h;
}

void Graphics::drawImage(Image *image, int x, int y)
{
    image->draw_deprecated(mScreen, x, y);
}

void Graphics::drawImagePattern(Image *image, int x, int y, int w, int h)
{
    int iw = image->getWidth();
    int ih = image->getHeight();
    if (iw == 0 || ih == 0) return;

    int px = 0;                       // X position on pattern plane
    int py = 0;                       // Y position on pattern plane

    while (py < h) {
        while (px < w) {
            int dw = (px + iw >= w) ? w - px : iw;
            int dh = (py + ih >= h) ? h - py : ih;
            image->draw_deprecated(mScreen, 0, 0, x + px, y + py, dw, dh);
            px += iw;
        }
        py += ih;
        px = 0;
    }
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
    drawImagePattern(center,
            x + topLeft->getWidth(), y + topLeft->getHeight(),
            w - topLeft->getWidth() - topRight->getWidth(),
            h - topLeft->getHeight() - bottomLeft->getHeight());

    // Draw the sides
    drawImagePattern(top,
            x + topLeft->getWidth(), y,
            w - topLeft->getWidth() - topRight->getWidth(), top->getHeight());
    drawImagePattern(bottom,
            x + bottomLeft->getWidth(), y + h - bottom->getHeight(),
            w - bottomLeft->getWidth() - bottomRight->getWidth(),
            bottom->getHeight());
    drawImagePattern(left,
            x, y + topLeft->getHeight(),
            left->getWidth(),
            h - topLeft->getHeight() - bottomLeft->getHeight());
    drawImagePattern(right,
            x + w - right->getWidth(), y + topRight->getHeight(),
            right->getWidth(),
            h - topRight->getHeight() - bottomRight->getHeight());

    // Draw the corners
    drawImage(topLeft, x, y);
    drawImage(topLeft, x, y);
    drawImage(topRight, x + w - topRight->getWidth(), y);
    drawImage(bottomLeft, x, y + h - bottomLeft->getHeight());
    drawImage(bottomRight,
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
    if (useOpenGL) {
#ifdef USE_OPENGL
        glFlush();
        glFinish();
        SDL_GL_SwapBuffers();
#endif
    }
    else {
        SDL_Flip(mScreen);
    }

    // Decrement frame counter when using framerate limiting
    if (framesToDraw > 1) framesToDraw--;

    // Wait while we're not allowed to draw next frame yet
    while (framesToDraw == 1)
    {
        SDL_Delay(10);
    }
}

void Graphics::setScreen(SDL_Surface *screen)
{
    mScreen = screen;
}
