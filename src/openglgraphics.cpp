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

#include <SDL.h>

#include "log.h"
#include "openglgraphics.h"

#include "resources/image.h"

#ifdef USE_OPENGL

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#endif

#ifndef GL_TEXTURE_RECTANGLE_ARB
#define GL_TEXTURE_RECTANGLE_ARB 0x84F5
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB 0x84F8
#endif

OpenGLGraphics::OpenGLGraphics():
    mAlpha(false), mTexture(false), mColorAlpha(false),
    mSync(false)
{
}

OpenGLGraphics::~OpenGLGraphics()
{
}

void OpenGLGraphics::setSync(bool sync)
{
    mSync = sync;
}

bool OpenGLGraphics::setVideoMode(int w, int h, int bpp, bool fs, bool hwaccel)
{
    logger->log("Setting video mode %dx%d %s",
            w, h, fs ? "fullscreen" : "windowed");

    int displayFlags = SDL_ANYFORMAT | SDL_OPENGL;

    mFullscreen = fs;
    mHWAccel = hwaccel;

    if (fs)
        displayFlags |= SDL_FULLSCREEN;

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if (!(mScreen = SDL_SetVideoMode(w, h, bpp, displayFlags)))
        return false;

#ifdef __APPLE__
    if (mSync)
    {
        const GLint VBL = 1;
        CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &VBL);
    }
#endif

    // Setup OpenGL
    glViewport(0, 0, w, h);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    int gotDoubleBuffer;
    SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &gotDoubleBuffer);
    logger->log("Using OpenGL %s double buffering.",
            (gotDoubleBuffer ? "with" : "without"));

    char const *glExtensions = (char const *)glGetString(GL_EXTENSIONS);
    GLint texSize;
    bool rectTex = strstr(glExtensions, "GL_ARB_texture_rectangle");
    if (rectTex)
    {
        Image::mTextureType = GL_TEXTURE_RECTANGLE_ARB;
        glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB, &texSize);
    }
    else
    {
        Image::mTextureType = GL_TEXTURE_2D;
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
        float texX1 = srcX / (float) image->getTextureWidth();
        float texY1 = srcY / (float) image->getTextureHeight();
        float texX2 = (srcX + width) / (float) image->getTextureWidth();
        float texY2 = (srcY + height) / (float) image->getTextureHeight();

        glTexCoord2f(texX1, texY1);
        glVertex2i(dstX, dstY);
        glTexCoord2f(texX2, texY1);
        glVertex2i(dstX + width, dstY);
        glTexCoord2f(texX2, texY2);
        glVertex2i(dstX + width, dstY + height);
        glTexCoord2f(texX1, texY2);
        glVertex2i(dstX, dstY + height);
    }
    else
    {
        glTexCoord2i(srcX, srcY);
        glVertex2i(dstX, dstY);
        glTexCoord2i(srcX + width, srcY);
        glVertex2i(dstX + width, dstY);
        glTexCoord2i(srcX + width, srcY + height);
        glVertex2i(dstX + width, dstY + height);
        glTexCoord2i(srcX, srcY + height);
        glVertex2i(dstX, dstY + height);
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

    glBindTexture(Image::mTextureType, image->mGLImage);

    setTexturingAndBlending(true);

    // Draw a textured quad.
    glBegin(GL_QUADS);
    drawQuad(image, srcX, srcY, dstX, dstY, width, height);
    glEnd();

    if (!useColor)
        glColor4ub(mColor.r, mColor.g, mColor.b, mColor.a);

    return true;
}

/* Optimising the functions that Graphics::drawImagePattern would call,
 * so that glBegin...glEnd are outside the main loop. */
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

    glColor4f(1.0f, 1.0f, 1.0f, image->mAlpha);

    glBindTexture(Image::mTextureType, image->mGLImage);

    setTexturingAndBlending(true);

    // Draw a set of textured rectangles
    glBegin(GL_QUADS);

    for (int py = 0; py < h; py += ih)
    {
        const int height = (py + ih >= h) ? h - py : ih;
        const int dstY = y + py;
        for (int px = 0; px < w; px += iw)
        {
            int width = (px + iw >= w) ? w - px : iw;
            int dstX = x + px;

            drawQuad(image, srcX, srcY, dstX, dstY, width, height);
        }
    }

    glEnd();

    glColor4ub(mColor.r, mColor.g, mColor.b, mColor.a);
}

void OpenGLGraphics::updateScreen()
{
    glFlush();
    glFinish();
    SDL_GL_SwapBuffers();
}

void OpenGLGraphics::_beginDraw()
{
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0.0, (double)mScreen->w, (double)mScreen->h, 0.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_SCISSOR_TEST);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    pushClipArea(gcn::Rectangle(0, 0, mScreen->w, mScreen->h));
}

void OpenGLGraphics::_endDraw()
{
}

SDL_Surface* OpenGLGraphics::getScreenshot()
{
    int h = mScreen->h;
    int w = mScreen->w;

    SDL_Surface *screenshot = SDL_CreateRGBSurface(
            SDL_SWSURFACE,
            w, h, 24,
            0xff0000, 0x00ff00, 0x0000ff, 0x000000);

    if (SDL_MUSTLOCK(screenshot))
        SDL_LockSurface(screenshot);

    // Grap the pixel buffer and write it to the SDL surface
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

    glPushMatrix();
    glTranslatef(transX, transY, 0);
    glScissor(mClipStack.top().x,
            mScreen->h - mClipStack.top().y - mClipStack.top().height,
            mClipStack.top().width,
            mClipStack.top().height);

    return result;
}

void OpenGLGraphics::popClipArea()
{
    gcn::Graphics::popClipArea();

    if (mClipStack.empty())
        return;

    glPopMatrix();
    glScissor(mClipStack.top().x,
              mScreen->h - mClipStack.top().y - mClipStack.top().height,
              mClipStack.top().width,
              mClipStack.top().height);
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

void OpenGLGraphics::setTargetPlane(int width, int height)
{
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

    glBegin(filled ? GL_QUADS : GL_LINE_LOOP);
    glVertex2f(rect.x + offset, rect.y + offset);
    glVertex2f(rect.x + rect.width - offset, rect.y + offset);
    glVertex2f(rect.x + rect.width - offset, rect.y + rect.height - offset);
    glVertex2f(rect.x + offset, rect.y + rect.height - offset);
    glEnd();
}

#endif // USE_OPENGL
