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

#ifndef OPENGLGRAPHICS_H
#define OPENGLGRAPHICS_H

#include "graphics.h"

class OpenGLGraphics : public Graphics
{
    public:
        OpenGLGraphics();

        ~OpenGLGraphics();

        /**
         * Sets whether vertical refresh syncing is enabled. Takes effect after
         * the next call to setVideoMode(). Only implemented on MacOS for now.
         */
        void setSync(bool sync);
        bool getSync() const { return mSync; }

        bool setVideoMode(int w, int h, int bpp, bool fs, bool hwaccel);

        bool drawImage(Image *image,
                       int srcX, int srcY,
                       int dstX, int dstY,
                       int width, int height,
                       bool useColor);

        void drawImagePattern(Image *image,
                              int x, int y,
                              int w, int h);

        void updateScreen();

        void _beginDraw();
        void _endDraw();

        bool pushClipArea(gcn::Rectangle area);
        void popClipArea();

        void setColor(const gcn::Color &color);

        void drawPoint(int x, int y);

        void drawLine(int x1, int y1, int x2, int y2);

        void drawRectangle(const gcn::Rectangle &rect, bool filled);

        void drawRectangle(const gcn::Rectangle &rect);

        void fillRectangle(const gcn::Rectangle &rect);

        void setTargetPlane(int width, int height);

        /**
         * Takes a screenshot and returns it as SDL surface.
         */
        SDL_Surface *getScreenshot();

    protected:
        void setTexturingAndBlending(bool enable);

    private:
        bool mAlpha, mTexture;
        bool mColorAlpha;
        bool mSync;
};

#endif
