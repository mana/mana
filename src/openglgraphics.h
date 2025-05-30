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

#pragma once

#ifdef USE_OPENGL
#include "graphics.h"

#define NO_SDL_GLEXT

#include <SDL_opengl.h>

#include <memory>

class VideoSettings;

class OpenGLGraphics final : public Graphics
{
    public:
        static std::unique_ptr<OpenGLGraphics> create(SDL_Window *window,
                                                      const VideoSettings &settings);

        OpenGLGraphics(SDL_Window *window, SDL_GLContext glContext);

        ~OpenGLGraphics() override;

        void setVSync(bool sync) override;

        /**
         * Sets whether input lag should be reduced.
         *
         * This means waiting until the graphics card has finished drawing and
         * displaying the current frame until continuing towards the next,
         * possibly at the cost of performance and CPU usage.
         */
        void setReduceInputLag(bool reduceInputLag);
        bool getReduceInputLag() const { return mReduceInputLag; }

        void updateSize(int windowWidth, int windowHeight, float scale) override;

        /**
         * Draws a rescaled version of the image
         */
        bool drawRescaledImage(const Image *image, int srcX, int srcY,
                               int dstX, int dstY,
                               int width, int height,
                               int desiredWidth, int desiredHeight,
                               bool useColor) override;

        bool drawRescaledImageF(const Image *image,
                                int srcX, int srcY,
                                float dstX, float dstY,
                                int width, int height,
                                float desiredWidth, float desiredHeight,
                                bool useColor) override;

        void drawImagePattern(const Image *image,
                              int x, int y,
                              int w, int h) override;

        /**
         * Draw a pattern based on a rescaled version of the given image...
         */
        void drawRescaledImagePattern(const Image *image,
                                      int srcX, int srcY,
                                      int srcW, int srcH,
                                      int dstX, int dstY,
                                      int dstW, int dstH,
                                      int scaledWidth, int scaledHeight) override;

        void updateScreen() override;

        void windowToLogical(int windowX, int windowY,
                             float &logicalX, float &logicalY) const override;

        bool pushClipArea(gcn::Rectangle area) override;
        void popClipArea() override;

        void setColor(const gcn::Color &color) override;

        void drawPoint(int x, int y) override;

        void drawLine(int x1, int y1, int x2, int y2) override;

        void drawRectangle(const gcn::Rectangle &rect, bool filled);

        void drawRectangle(const gcn::Rectangle &rect) override;

        void fillRectangle(const gcn::Rectangle &rect) override;

        /**
         * Takes a screenshot and returns it as SDL surface.
         */
        SDL_Surface *getScreenshot() override;

        static void bindTexture(GLenum target, GLuint texture);

        static GLuint mLastImage;

    protected:
        void setTexturingAndBlending(bool enable);

        void updateClipRect() override;

    private:
        void drawQuadArrayfi(int size);

        void drawQuadArrayii(int size);

        SDL_Window *mWindow = nullptr;
        SDL_GLContext mContext = nullptr;
        GLfloat *mFloatTexArray;
        GLint *mIntTexArray;
        GLint *mIntVertArray;
        float mUserScale = 1.0f;
        float mScaleX = 1.0f;
        float mScaleY = 1.0f;
        bool mAlpha = false;
        bool mTexture = false;
        bool mColorAlpha = false;
        bool mReduceInputLag = true;
};
#endif //USE_OPENGL
