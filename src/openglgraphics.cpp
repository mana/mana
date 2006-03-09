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

#ifdef USE_OPENGL

#include "openglgraphics.h"

#include <SDL.h>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#endif

#include <guichan/image.hpp>

#include "log.h"

#include "resources/image.h"

extern volatile int framesToDraw;

OpenGLGraphics::OpenGLGraphics():
    mAlpha(false), mTexture(false), mColorAlpha(false)
{
}

OpenGLGraphics::~OpenGLGraphics()
{
}

bool OpenGLGraphics::setVideoMode(int w, int h, int bpp, bool fs, bool hwaccel)
{
    int displayFlags = SDL_ANYFORMAT | SDL_OPENGL;

    mFullscreen = fs;
    mHWAccel = hwaccel;

    if (fs) {
        displayFlags |= SDL_FULLSCREEN;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if (!(mScreen = SDL_SetVideoMode(w, h, bpp, displayFlags))) {
        return false;
    }

#ifdef __APPLE__
    long VBL = 1;
    CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &VBL);
#endif

    // Setup OpenGL
    glViewport(0, 0, w, h);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    int gotDoubleBuffer;
    SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &gotDoubleBuffer);
    logger->log("Using OpenGL %s double buffering.",
            (gotDoubleBuffer ? "with" : "without"));

    return true;
}

bool OpenGLGraphics::drawImage(Image *image, int srcX, int srcY,
        int dstX, int dstY, int width, int height)
{
    srcX += image->mBounds.x;
    srcY += image->mBounds.y;

    // Find OpenGL texture coordinates
    float texX1 = srcX / (float)image->mTexWidth;
    float texY1 = srcY / (float)image->mTexHeight;
    float texX2 = (srcX + width) / (float)image->mTexWidth;
    float texY2 = (srcY + height) / (float)image->mTexHeight;

    glColor4f(1.0f, 1.0f, 1.0f, image->mAlpha);
    glBindTexture(GL_TEXTURE_2D, image->mGLImage);

    drawTexedQuad(dstX, dstY, width, height, texX1, texY1, texX2, texY2);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    return true;
}

void OpenGLGraphics::updateScreen()
{
    glFlush();
    glFinish();
    SDL_GL_SwapBuffers();

    // Decrement frame counter when using framerate limiting
    if (framesToDraw > 1) framesToDraw--;

    // Wait while we're not allowed to draw next frame yet
    while (framesToDraw == 1)
    {
        SDL_Delay(10);
    }
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

    if (SDL_MUSTLOCK(screenshot)) {
        SDL_LockSurface(screenshot);
    }

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

    if (SDL_MUSTLOCK(screenshot)) {
        SDL_UnlockSurface(screenshot);
    }

    return screenshot;
}

bool OpenGLGraphics::pushClipArea(gcn::Rectangle area)
{
    int transX = 0;
    int transY = 0;

    if (!mClipStack.empty()) {
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
    {
        return;
    }

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

void OpenGLGraphics::drawImage(const gcn::Image* image,
                               int srcX, int srcY,
                               int dstX, int dstY,
                               int width, int height)
{
    // The following code finds the real width and height of the texture.
    // OpenGL only supports texture sizes that are powers of two
    int realImageWidth = 1;
    int realImageHeight = 1;
    while (realImageWidth < image->getWidth())
    {
        realImageWidth *= 2;
    }
    while (realImageHeight < image->getHeight())
    {
        realImageHeight *= 2;
    }

    // Find OpenGL texture coordinates
    float texX1 = srcX / (float)realImageWidth;
    float texY1 = srcY / (float)realImageHeight;
    float texX2 = (srcX + width) / (float)realImageWidth;
    float texY2 = (srcY + height) / (float)realImageHeight;

    // Please dont look too closely at the next line, it is not pretty.
    // It uses the image data as a pointer to a GLuint
    glBindTexture(GL_TEXTURE_2D, *((GLuint *)(image->_getData())));

    drawTexedQuad(dstX, dstY, width, height, texX1, texY1, texX2, texY2);
}

void OpenGLGraphics::drawPoint(int x, int y)
{
    setTexturingAndBlending(false);

    glBegin(GL_POINTS);
    glVertex3i(x, y, 0);
    glEnd();
}

void OpenGLGraphics::drawLine(int x1, int y1, int x2, int y2)
{
    setTexturingAndBlending(false);

    glBegin(GL_LINES);
    glVertex3f(x1 + 0.5f, y1 + 0.5f, 0);
    glVertex3f(x2 + 0.5f, y2 + 0.5f, 0);
    glEnd();

    glBegin(GL_POINTS);
    glVertex3f(x2 + 0.5f, y2 + 0.5f, 0);
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
    if (enable) {
        if (!mTexture) {
            glEnable(GL_TEXTURE_2D);
            mTexture = true;
        };

        if (!mAlpha)
        {
            glEnable(GL_BLEND);
            mAlpha = true;
        }
    } else {
        if (mAlpha && !mColorAlpha) {
            glDisable(GL_BLEND);
            mAlpha = false;
        } else if (!mAlpha && mColorAlpha) {
            glEnable(GL_BLEND);
            mAlpha = true;
        }

        if (mTexture) {
            glDisable(GL_TEXTURE_2D);
            mTexture = false;
        }
    }
}

void OpenGLGraphics::drawRectangle(const gcn::Rectangle& rect, bool filled)
{
    float offset = filled ? 0 : 0.5f;

    setTexturingAndBlending(false);

    glBegin(filled ? GL_QUADS : GL_LINE_LOOP);
    glVertex3f(rect.x + offset, rect.y + offset, 0);
    glVertex3f(rect.x + rect.width - offset, rect.y + offset, 0);
    glVertex3f(rect.x + rect.width - offset, rect.y + rect.height - offset, 0);
    glVertex3f(rect.x + offset, rect.y + rect.height - offset, 0);
    glEnd();
}

void OpenGLGraphics::drawTexedQuad(int x, int y, int w, int h,
        float texX1, float texY1, float texX2, float texY2)
{
    setTexturingAndBlending(true);

    // Draw a textured quad
    glBegin(GL_QUADS);
    glTexCoord2f(texX1, texY1);
    glVertex3i(x, y, 0);

    glTexCoord2f(texX2, texY1);
    glVertex3i(x + w, y, 0);

    glTexCoord2f(texX2, texY2);
    glVertex3i(x + w, y + h, 0);

    glTexCoord2f(texX1, texY2);
    glVertex3i(x, y + h, 0);
    glEnd();
}

#endif // USE_OPENGL
