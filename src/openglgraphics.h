/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OPENGLGRAPHICS_H
#define OPENGLGRAPHICS_H

#include "graphics.h"

#ifdef USE_OPENGL
#define NO_SDL_GLEXT

#include <SDL_opengl.h>

class OpenGLGraphics : public Graphics
{
    public:
        OpenGLGraphics();

        ~OpenGLGraphics();

        /**
         * Sets whether vertical refresh syncing is enabled. Takes effect
         * immediately.
         */
        void setSync(bool sync);
        bool getSync() const { return mSync; }

        /**
         * Sets whether input lag should be reduced.
         *
         * This means waiting until the graphics card has finished drawing and
         * displaying the current frame until continuing towards the next,
         * possibly at the cost of performance and CPU usage.
         */
        void setReduceInputLag(bool reduceInputLag);
        bool getReduceInputLag() const { return mReduceInputLag; }

        bool setVideoMode(int w, int h, bool fs);

        void videoResized(int w, int h);

        bool drawImage(Image *image,
                       int srcX, int srcY,
                       int dstX, int dstY,
                       int width, int height,
                       bool useColor);

        /**
         * Draws a rescaled version of the image
         */
        bool drawRescaledImage(Image *image, int srcX, int srcY,
                               int dstX, int dstY,
                               int width, int height,
                               int desiredWidth, int desiredHeight,
                               bool useColor);

        void drawImagePattern(Image *image,
                              int x, int y,
                              int w, int h);

        /**
         * Draw a pattern based on a rescaled version of the given image...
         */
        void drawRescaledImagePattern(Image *image,
                                      int x, int y, int w, int h,
                                      int scaledWidth, int scaledHeight);

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

        /**
         * Takes a screenshot and returns it as SDL surface.
         */
        SDL_Surface *getScreenshot();

        static void bindTexture(GLenum target, GLuint texture);

        static GLuint mLastImage;

    protected:
        void setTexturingAndBlending(bool enable);

    private:
        void drawQuadArrayfi(int size);

        void drawQuadArrayii(int size);

        SDL_GLContext mContext = nullptr;
        GLfloat *mFloatTexArray;
        GLint *mIntTexArray;
        GLint *mIntVertArray;
        bool mAlpha, mTexture;
        bool mColorAlpha;
        bool mSync;
        bool mReduceInputLag;
};
#endif //USE_OPENGL

#endif
