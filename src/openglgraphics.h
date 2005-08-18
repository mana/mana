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

#ifndef _TMW_OPENGLGRAPHICS_H
#define _TMW_OPENGLGRAPHICS_H

#include "graphics.h"

class OpenGLGraphics : public Graphics
{
    public:
        OpenGLGraphics();

        ~OpenGLGraphics();

        bool setVideoMode(int w, int h, int bpp, bool fs, bool hwaccel);

        bool drawImage(Image *image, int srcX, int srcY, int dstX, int dstY, int width, int height);

        void updateScreen();

        void _beginDraw();
        void _endDraw();

        bool pushClipArea(gcn::Rectangle area);
        void popClipArea();

        void setColor(const gcn::Color &color);

        void drawImage(const gcn::Image* image, int srcX, int srcY,
                int dstX, int dstY, int width, int height);

        void drawPoint(int x, int y);

        void drawLine(int x1, int y1, int x2, int y2);

        void drawRectangle(const gcn::Rectangle& rectangle);

        void fillRectangle(const gcn::Rectangle &rectangle);

        void setTargetPlane(int width, int height);

    private:
        bool mAlpha, mTexture;
        bool mColorAlpha;
};

#endif
