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

#include "openglgraphics.h"

#include "log.h"

#include "resources/image.h"

#ifdef USE_OPENGL

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#endif

#include <SDL.h>

#include <cmath>

#ifndef GL_TEXTURE_RECTANGLE_ARB
#define GL_TEXTURE_RECTANGLE_ARB 0x84F5
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB 0x84F8
#endif

const unsigned int vertexBufSize = 500;

GLuint OpenGLGraphics::mLastImage = 0;

OpenGLGraphics::OpenGLGraphics():
    mAlpha(false), mTexture(false), mColorAlpha(false),
    mSync(false),
    mReduceInputLag(true)
{
    mFloatTexArray = new GLfloat[vertexBufSize * 4];
    mIntTexArray = new GLint[vertexBufSize * 4];
    mIntVertArray = new GLint[vertexBufSize * 4];
}

OpenGLGraphics::~OpenGLGraphics()
{
    delete[] mFloatTexArray;
    delete[] mIntTexArray;
    delete[] mIntVertArray;
}

void OpenGLGraphics::setSync(bool sync)
{
    mSync = sync;
}

void OpenGLGraphics::setReduceInputLag(bool reduceInputLag)
{
    mReduceInputLag = reduceInputLag;
}

bool OpenGLGraphics::setVideoMode(int w, int h, int bpp, bool fs, bool hwaccel)
{
    logger->log("Setting video mode %dx%d %s",
            w, h, fs ? "fullscreen" : "windowed");

    int displayFlags = SDL_ANYFORMAT | SDL_OPENGL;

    const double targetRatio = (double) 640 / 360; // 1.77778
    const double requestedRatio = (double) w / h;

    if (requestedRatio < targetRatio) {
        // Screen is higher / narrower than target aspect ratio: calculate
        // scale based on height.
        mScale = (int) std::floor((double) h / 360);
    } else {
        mScale = (int) std::floor((double) w / 640);
    }

    mWidth = w / mScale;
    mHeight = h / mScale;
    mBpp = bpp;
    mFullscreen = fs;
    mHWAccel = hwaccel;

    if (fs)
    {
        displayFlags |= SDL_FULLSCREEN;
    }
    else
    {
        // Resizing currently not supported on Windows, where it would require
        // reuploading all textures.
#if !defined(_WIN32)
        displayFlags |= SDL_RESIZABLE;
#endif
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if (!(mTarget = SDL_SetVideoMode(w, h, bpp, displayFlags)))
        return false;

#ifdef __APPLE__
    if (mSync)
    {
        const GLint VBL = 1;
        CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &VBL);
    }
#endif

    // Setup OpenGL
    glViewport(0, 0, mWidth * mScale, mHeight * mScale);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    int gotDoubleBuffer;
    SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &gotDoubleBuffer);
    logger->log("Using OpenGL %s double buffering.",
                (gotDoubleBuffer ? "with" : "without"));

    char const *glExtensions = (char const *)glGetString(GL_EXTENSIONS);
    GLint texSize;
    bool rectTex = strstr(glExtensions, "GL_ARB_texture_rectangle");
    bool npotTex = strstr(glExtensions, "GL_ARB_texture_non_power_of_two");
    if (rectTex && !npotTex)
    {
        Image::mTextureType = GL_TEXTURE_RECTANGLE_ARB;
        Image::mPowerOfTwoTextures = false;
        glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB, &texSize);
    }
    else
    {
        Image::mTextureType = GL_TEXTURE_2D;
        Image::mPowerOfTwoTextures = !npotTex;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
    }
    Image::mTextureSize = texSize;
    logger->log("OpenGL texture size: %d pixels%s", Image::mTextureSize,
                rectTex ? " (rectangle textures)" : "");

    return true;
}

static inline void drawQuad(Image *image,
                            int srcX, int srcY, int dstX, int dstY,
                            int width, int height)
{
    if (image->getTextureType() == GL_TEXTURE_2D)
    {
        // Find OpenGL normalized texture coordinates.
        const float texX1 = static_cast<float>(srcX) /
                            static_cast<float>(image->getTextureWidth());
        const float texY1 = static_cast<float>(srcY) /
                            static_cast<float>(image->getTextureHeight());
        const float texX2 = static_cast<float>(srcX + width) /
                            static_cast<float>(image->getTextureWidth());
        const float texY2 = static_cast<float>(srcY + height) /
                            static_cast<float>(image->getTextureHeight());

        GLfloat tex[] =
        {
            texX1, texY1,
            texX2, texY1,
            texX2, texY2,
            texX1, texY2
        };

        GLint vert[] =
        {
            dstX, dstY,
            dstX + width, dstY,
            dstX + width, dstY + height,
            dstX, dstY + height
        };

        glVertexPointer(2, GL_INT, 0, &vert);
        glTexCoordPointer(2, GL_FLOAT, 0, &tex);

        glDrawArrays(GL_QUADS, 0, 4);
    }
    else
    {
        GLint tex[] =
        {
            srcX, srcY,
            srcX + width, srcY,
            srcX + width, srcY + height,
            srcX, srcY + height
        };
        GLint vert[] =
        {
            dstX, dstY,
            dstX + width, dstY,
            dstX + width, dstY + height,
            dstX, dstY + height
        };

        glVertexPointer(2, GL_INT, 0, &vert);
        glTexCoordPointer(2, GL_INT, 0, &tex);

        glDrawArrays(GL_QUADS, 0, 4);
    }
}

static inline void drawRescaledQuad(Image *image,
                                    int srcX, int srcY, int dstX, int dstY,
                                    int width, int height,
                                    int desiredWidth, int desiredHeight)
{
    if (image->getTextureType() == GL_TEXTURE_2D)
    {
        // Find OpenGL normalized texture coordinates.
        const float texX1 = static_cast<float>(srcX) /
                            static_cast<float>(image->getTextureWidth());
        const float texY1 = static_cast<float>(srcY) /
                            static_cast<float>(image->getTextureHeight());
        const float texX2 = static_cast<float>(srcX + width) /
                            static_cast<float>(image->getTextureWidth());
        const float texY2 = static_cast<float>(srcY + height) /
                            static_cast<float>(image->getTextureHeight());

        GLfloat tex[] =
        {
            texX1, texY1,
            texX2, texY1,
            texX2, texY2,
            texX1, texY2
        };

        GLint vert[] =
        {
            dstX, dstY,
            dstX + desiredWidth, dstY,
            dstX + desiredWidth, dstY + desiredHeight,
            dstX, dstY + desiredHeight
        };

        glVertexPointer(2, GL_INT, 0, &vert);
        glTexCoordPointer(2, GL_FLOAT, 0, &tex);

        glDrawArrays(GL_QUADS, 0, 4);
    }
    else
    {
        GLint tex[] =
        {
            srcX, srcY,
            srcX + width, srcY,
            srcX + width, srcY + height,
            srcX, srcY + height
        };
        GLint vert[] =
        {
            dstX, dstY,
            dstX + desiredWidth, dstY,
            dstX + desiredWidth, dstY + desiredHeight,
            dstX, dstY + desiredHeight
        };

        glVertexPointer(2, GL_INT, 0, &vert);
        glTexCoordPointer(2, GL_INT, 0, &tex);

        glDrawArrays(GL_QUADS, 0, 4);
    }
}


bool OpenGLGraphics::drawImage(Image *image, int srcX, int srcY,
                               int dstX, int dstY,
                               int width, int height, bool useColor)
{
    if (!image)
        return false;

    srcX += image->mBounds.x;
    srcY += image->mBounds.y;

    if (!useColor)
        glColor4f(1.0f, 1.0f, 1.0f, image->mAlpha);

    bindTexture(Image::mTextureType, image->mGLImage);

    setTexturingAndBlending(true);

    drawQuad(image, srcX, srcY, dstX, dstY, width, height);

    if (!useColor)
    {
        glColor4ub(static_cast<GLubyte>(mColor.r),
                   static_cast<GLubyte>(mColor.g),
                   static_cast<GLubyte>(mColor.b),
                   static_cast<GLubyte>(mColor.a));
    }

    return true;
}

bool OpenGLGraphics::drawRescaledImage(Image *image, int srcX, int srcY,
                                       int dstX, int dstY,
                                       int width, int height,
                                       int desiredWidth, int desiredHeight,
                                       bool useColor)
{
    if (!image)
        return false;

    // Just draw the image normally when no resizing is necessary,
    if (width == desiredWidth && height == desiredHeight)
    {
        return drawImage(image, srcX, srcY, dstX, dstY,
                         width, height, useColor);
    }

    srcX += image->mBounds.x;
    srcY += image->mBounds.y;

    if (!useColor)
        glColor4f(1.0f, 1.0f, 1.0f, image->mAlpha);

    bindTexture(Image::mTextureType, image->mGLImage);

    setTexturingAndBlending(true);

    // Draw a textured quad.
    drawRescaledQuad(image, srcX, srcY, dstX, dstY, width, height,
                     desiredWidth, desiredHeight);

    if (!useColor)
    {
        glColor4ub(static_cast<GLubyte>(mColor.r),
                   static_cast<GLubyte>(mColor.g),
                   static_cast<GLubyte>(mColor.b),
                   static_cast<GLubyte>(mColor.a));
    }

    return true;
}

void OpenGLGraphics::drawImagePattern(Image *image, int x, int y, int w, int h)
{
    if (!image)
        return;

    const int srcX = image->mBounds.x;
    const int srcY = image->mBounds.y;

    const int iw = image->getWidth();
    const int ih = image->getHeight();

    if (iw == 0 || ih == 0)
        return;

    const float tw = static_cast<float>(image->getTextureWidth());
    const float th = static_cast<float>(image->getTextureHeight());

    glColor4f(1.0f, 1.0f, 1.0f, image->mAlpha);

    bindTexture(Image::mTextureType, image->mGLImage);

    setTexturingAndBlending(true);

    unsigned int vp = 0;
    const unsigned int vLimit = vertexBufSize * 4;
    // Draw a set of textured rectangles
    if (image->getTextureType() == GL_TEXTURE_2D)
    {
        float texX1 = static_cast<float>(srcX) / tw;
        float texY1 = static_cast<float>(srcY) / th;

        for (int py = 0; py < h; py += ih)
        {
            const int height = (py + ih >= h) ? h - py : ih;
            const int dstY = y + py;
            for (int px = 0; px < w; px += iw)
            {
                int width = (px + iw >= w) ? w - px : iw;
                int dstX = x + px;

                float texX2 = static_cast<float>(srcX + width) / tw;
                float texY2 = static_cast<float>(srcY + height) / th;

                mFloatTexArray[vp + 0] = texX1;
                mFloatTexArray[vp + 1] = texY1;

                mFloatTexArray[vp + 2] = texX2;
                mFloatTexArray[vp + 3] = texY1;

                mFloatTexArray[vp + 4] = texX2;
                mFloatTexArray[vp + 5] = texY2;

                mFloatTexArray[vp + 6] = texX1;
                mFloatTexArray[vp + 7] = texY2;

                mIntVertArray[vp + 0] = dstX;
                mIntVertArray[vp + 1] = dstY;

                mIntVertArray[vp + 2] = dstX + width;
                mIntVertArray[vp + 3] = dstY;

                mIntVertArray[vp + 4] = dstX + width;
                mIntVertArray[vp + 5] = dstY + height;

                mIntVertArray[vp + 6] = dstX;
                mIntVertArray[vp + 7] = dstY + height;

                vp += 8;
                if (vp >= vLimit)
                {
                    drawQuadArrayfi(vp);
                    vp = 0;
                }
            }
        }
        if (vp > 0)
            drawQuadArrayfi(vp);
    }
    else
    {
        for (int py = 0; py < h; py += ih)
        {
            const int height = (py + ih >= h) ? h - py : ih;
            const int dstY = y + py;
            for (int px = 0; px < w; px += iw)
            {
                int width = (px + iw >= w) ? w - px : iw;
                int dstX = x + px;

                mIntTexArray[vp + 0] = srcX;
                mIntTexArray[vp + 1] = srcY;

                mIntTexArray[vp + 2] = srcX + width;
                mIntTexArray[vp + 3] = srcY;

                mIntTexArray[vp + 4] = srcX + width;
                mIntTexArray[vp + 5] = srcY + height;

                mIntTexArray[vp + 6] = srcX;
                mIntTexArray[vp + 7] = srcY + height;

                mIntVertArray[vp + 0] = dstX;
                mIntVertArray[vp + 1] = dstY;

                mIntVertArray[vp + 2] = dstX + width;
                mIntVertArray[vp + 3] = dstY;

                mIntVertArray[vp + 4] = dstX + width;
                mIntVertArray[vp + 5] = dstY + height;

                mIntVertArray[vp + 6] = dstX;
                mIntVertArray[vp + 7] = dstY + height;

                vp += 8;
                if (vp >= vLimit)
                {
                    drawQuadArrayii(vp);
                    vp = 0;
                }
            }
        }
        if (vp > 0)
            drawQuadArrayii(vp);
    }

    glColor4ub(static_cast<GLubyte>(mColor.r),
               static_cast<GLubyte>(mColor.g),
               static_cast<GLubyte>(mColor.b),
               static_cast<GLubyte>(mColor.a));
}

void OpenGLGraphics::drawRescaledImagePattern(Image *image,
                                              int x, int y,
                                              int w, int h,
                                              int scaledWidth,
                                              int scaledHeight)
{
    if (!image)
        return;

    if (scaledWidth == 0 || scaledHeight == 0)
        return;

    const int srcX = image->mBounds.x;
    const int srcY = image->mBounds.y;

    const int iw = image->getWidth();
    const int ih = image->getHeight();
    if (iw == 0 || ih == 0)
        return;

    glColor4f(1.0f, 1.0f, 1.0f, image->mAlpha);

    bindTexture(Image::mTextureType, image->mGLImage);

    setTexturingAndBlending(true);

    unsigned int vp = 0;
    const unsigned int vLimit = vertexBufSize * 4;

    // Draw a set of textured rectangles
    if (image->getTextureType() == GL_TEXTURE_2D)
    {
        const float tw = static_cast<float>(image->getTextureWidth());
        const float th = static_cast<float>(image->getTextureHeight());

        const float texX1 = static_cast<float>(srcX) / tw;
        const float texY1 = static_cast<float>(srcY) / th;

        const float tFractionW = iw / tw;
        const float tFractionH = ih / th;

        for (int py = 0; py < h; py += scaledHeight)
        {
            const int height = (py + scaledHeight >= h) ? h - py : scaledHeight;
            const int dstY = y + py;
            for (int px = 0; px < w; px += scaledWidth)
            {
                int width = (px + scaledWidth >= w) ? w - px : scaledWidth;
                int dstX = x + px;
                const float visibleFractionW = (float) width / scaledWidth;
                const float visibleFractionH = (float) height / scaledHeight;

                const float texX2 = texX1 + tFractionW * visibleFractionW;
                const float texY2 = texY1 + tFractionH * visibleFractionH;

                mFloatTexArray[vp + 0] = texX1;
                mFloatTexArray[vp + 1] = texY1;

                mFloatTexArray[vp + 2] = texX2;
                mFloatTexArray[vp + 3] = texY1;

                mFloatTexArray[vp + 4] = texX2;
                mFloatTexArray[vp + 5] = texY2;

                mFloatTexArray[vp + 6] = texX1;
                mFloatTexArray[vp + 7] = texY2;

                mIntVertArray[vp + 0] = dstX;
                mIntVertArray[vp + 1] = dstY;

                mIntVertArray[vp + 2] = dstX + width;
                mIntVertArray[vp + 3] = dstY;

                mIntVertArray[vp + 4] = dstX + width;
                mIntVertArray[vp + 5] = dstY + height;

                mIntVertArray[vp + 6] = dstX;
                mIntVertArray[vp + 7] = dstY + height;

                vp += 8;
                if (vp >= vLimit)
                {
                    drawQuadArrayfi(vp);
                    vp = 0;
                }
            }
        }
        if (vp > 0)
            drawQuadArrayfi(vp);
    }
    else
    {
        const float scaleFactorW = (float) scaledWidth / iw;
        const float scaleFactorH = (float) scaledHeight / ih;

        for (int py = 0; py < h; py += scaledHeight)
        {
            const int height = (py + scaledHeight >= h) ? h - py : scaledHeight;
            const int dstY = y + py;
            for (int px = 0; px < w; px += scaledWidth)
            {
                int width = (px + scaledWidth >= w) ? w - px : scaledWidth;
                int dstX = x + px;

                mIntTexArray[vp + 0] = srcX;
                mIntTexArray[vp + 1] = srcY;

                mIntTexArray[vp + 2] = srcX + width / scaleFactorW;
                mIntTexArray[vp + 3] = srcY;

                mIntTexArray[vp + 4] = srcX + width / scaleFactorW;
                mIntTexArray[vp + 5] = srcY + height / scaleFactorH;

                mIntTexArray[vp + 6] = srcX;
                mIntTexArray[vp + 7] = srcY + height / scaleFactorH;

                mIntVertArray[vp + 0] = dstX;
                mIntVertArray[vp + 1] = dstY;

                mIntVertArray[vp + 2] = dstX + width;
                mIntVertArray[vp + 3] = dstY;

                mIntVertArray[vp + 4] = dstX + width;
                mIntVertArray[vp + 5] = dstY + height;

                mIntVertArray[vp + 6] = dstX;
                mIntVertArray[vp + 7] = dstY + height;

                vp += 8;
                if (vp >= vLimit)
                {
                    drawQuadArrayii(vp);
                    vp = 0;
                }
            }
        }
        if (vp > 0)
            drawQuadArrayii(vp);
    }

    glColor4ub(mColor.r, mColor.g, mColor.b, mColor.a);
}

void OpenGLGraphics::updateScreen()
{
    SDL_GL_SwapBuffers();

    /*
     * glFinish flushes all OpenGL commands and makes sure they have been
     * executed before continuing. If we do not do this we allow the next
     * frame to be prepared while the current one isn't even displaying yet,
     * which can cause input lag that is especially noticable at mouse
     * movement.
     *
     * The setting is optional since calling glFinish can reduce performance
     * and increase CPU usage.
     */
    if (mReduceInputLag)
        glFinish();
}

void OpenGLGraphics::_beginDraw()
{
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0.0, (double) mWidth, (double) mHeight, 0.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_SCISSOR_TEST);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    pushClipArea(gcn::Rectangle(0, 0, mWidth, mHeight));
}

void OpenGLGraphics::_endDraw()
{
    popClipArea();
}

SDL_Surface* OpenGLGraphics::getScreenshot()
{
    int h = mTarget->h;
    int w = mTarget->w;
    GLint pack = 1;

    SDL_Surface *screenshot = SDL_CreateRGBSurface(
            SDL_SWSURFACE,
            w, h, 24,
            0xff0000, 0x00ff00, 0x0000ff, 0x000000);

    if (SDL_MUSTLOCK(screenshot))
        SDL_LockSurface(screenshot);

    // Grap the pixel buffer and write it to the SDL surface
    glGetIntegerv(GL_PACK_ALIGNMENT, &pack);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, screenshot->pixels);

    // Flip the screenshot, as OpenGL has 0,0 in bottom left
    unsigned int lineSize = 3 * w;
    GLubyte* buf = (GLubyte*)malloc(lineSize);

    for (int i = 0; i < (h / 2); i++)
    {
        GLubyte *top = (GLubyte*)screenshot->pixels + lineSize * i;
        GLubyte *bot = (GLubyte*)screenshot->pixels + lineSize * (h - 1 - i);

        memcpy(buf, top, lineSize);
        memcpy(top, bot, lineSize);
        memcpy(bot, buf, lineSize);
    }

    free(buf);

    glPixelStorei(GL_PACK_ALIGNMENT, pack);

    if (SDL_MUSTLOCK(screenshot))
        SDL_UnlockSurface(screenshot);

    return screenshot;
}

bool OpenGLGraphics::pushClipArea(gcn::Rectangle area)
{
    int transX = 0;
    int transY = 0;

    if (!mClipStack.empty())
    {
        transX = -mClipStack.top().xOffset;
        transY = -mClipStack.top().yOffset;
    }

    bool result = gcn::Graphics::pushClipArea(area);

    transX += mClipStack.top().xOffset;
    transY += mClipStack.top().yOffset;

    int x = (int) (mClipStack.top().x * mScale);
    int y = mTarget->h - (int) ((mClipStack.top().y +
                                 mClipStack.top().height) * mScale);
    int width = (int) (mClipStack.top().width * mScale);
    int height = (int) (mClipStack.top().height * mScale);

    glPushMatrix();
    glTranslatef(transX, transY, 0);
    glScissor(x, y, width, height);

    return result;
}

void OpenGLGraphics::popClipArea()
{
    gcn::Graphics::popClipArea();

    if (mClipStack.empty())
        return;

    int x = (int) (mClipStack.top().x * mScale);
    int y = mTarget->h - (int) ((mClipStack.top().y +
                                 mClipStack.top().height) * mScale);
    int width = (int) (mClipStack.top().width * mScale);
    int height = (int) (mClipStack.top().height * mScale);

    glPopMatrix();
    glScissor(x, y, width, height);
}

void OpenGLGraphics::setColor(const gcn::Color& color)
{
    mColor = color;
    glColor4ub(color.r, color.g, color.b, color.a);

    mColorAlpha = (color.a != 255);
}

void OpenGLGraphics::drawPoint(int x, int y)
{
    setTexturingAndBlending(false);

    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

void OpenGLGraphics::drawLine(int x1, int y1, int x2, int y2)
{
    setTexturingAndBlending(false);

    glBegin(GL_LINES);
    glVertex2f(x1 + 0.5f, y1 + 0.5f);
    glVertex2f(x2 + 0.5f, y2 + 0.5f);
    glEnd();

    glBegin(GL_POINTS);
    glVertex2f(x2 + 0.5f, y2 + 0.5f);
    glEnd();
}

void OpenGLGraphics::drawRectangle(const gcn::Rectangle& rect)
{
    drawRectangle(rect, false);
}

void OpenGLGraphics::fillRectangle(const gcn::Rectangle& rect)
{
    drawRectangle(rect, true);
}

void OpenGLGraphics::setTexturingAndBlending(bool enable)
{
    if (enable)
    {
        if (!mTexture)
        {
            glEnable(Image::mTextureType);
            mTexture = true;
        }

        if (!mAlpha)
        {
            glEnable(GL_BLEND);
            mAlpha = true;
        }
    }
    else
    {
        mLastImage = 0;
        if (mAlpha && !mColorAlpha)
        {
            glDisable(GL_BLEND);
            mAlpha = false;
        }
        else if (!mAlpha && mColorAlpha)
        {
            glEnable(GL_BLEND);
            mAlpha = true;
        }

        if (mTexture)
        {
            glDisable(Image::mTextureType);
            mTexture = false;
        }
    }
}

void OpenGLGraphics::drawRectangle(const gcn::Rectangle& rect, bool filled)
{
    const float offset = filled ? 0 : 0.5f;

    setTexturingAndBlending(false);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    GLfloat vert[] =
    {
        rect.x + offset, rect.y + offset,
        rect.x + rect.width - offset, rect.y + offset,
        rect.x + rect.width - offset, rect.y + rect.height - offset,
        rect.x + offset, rect.y + rect.height - offset
    };

    glVertexPointer(2, GL_FLOAT, 0, &vert);
    glDrawArrays(filled ? GL_QUADS : GL_LINE_LOOP, 0, 4);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void OpenGLGraphics::bindTexture(GLenum target, GLuint texture)
{
    if (mLastImage != texture)
    {
        mLastImage = texture;
        glBindTexture(target, texture);
    }
}

inline void OpenGLGraphics::drawQuadArrayfi(int size)
{
    glVertexPointer(2, GL_INT, 0, mIntVertArray);
    glTexCoordPointer(2, GL_FLOAT, 0, mFloatTexArray);

    glDrawArrays(GL_QUADS, 0, size / 2);
}

inline void OpenGLGraphics::drawQuadArrayii(int size)
{
    glVertexPointer(2, GL_INT, 0, mIntVertArray);
    glTexCoordPointer(2, GL_INT, 0, mIntTexArray);

    glDrawArrays(GL_QUADS, 0, size / 2);
}

#endif // USE_OPENGL
