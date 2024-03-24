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

#ifndef SDLGRAPHICS_H
#define SDLGRAPHICS_H

#include "graphics.h"

#include <memory>

class VideoSettings;

class SDLGraphics final : public Graphics
{
public:
    static std::unique_ptr<Graphics> create(SDL_Window *window,
                                            const VideoSettings &settings);

    SDLGraphics(SDL_Renderer *renderer);
    ~SDLGraphics() override;

    void setVSync(bool sync) override;

    void updateSize(int windowWidth, int windowHeight, float scale) override;

    bool drawRescaledImage(Image *image,
                           int srcX, int srcY,
                           int dstX, int dstY,
                           int width, int height,
                           int desiredWidth, int desiredHeight,
                           bool useColor) override;

#if SDL_VERSION_ATLEAST(2, 0, 10)
    bool drawRescaledImageF(Image *image,
                            int srcX, int srcY,
                            float dstX, float dstY,
                            int width, int height,
                            float desiredWidth, float desiredHeight,
                            bool useColor) override;
#endif

    void drawRescaledImagePattern(Image *image,
                                  int x, int y,
                                  int w, int h,
                                  int scaledWidth,
                                  int scaledHeight) override;

    void updateScreen() override;

    void windowToLogical(int windowX, int windowY,
                         float &logicalX, float &logicalY) const override;

    SDL_Surface *getScreenshot() override;

    bool pushClipArea(gcn::Rectangle area) override;

    void popClipArea() override;

    void drawPoint(int x, int y) override;

    void drawLine(int x1, int y1, int x2, int y2) override;

    void drawRectangle(const gcn::Rectangle &rectangle) override;

    void fillRectangle(const gcn::Rectangle &rectangle) override;

private:
    void updateSDLClipRect();

    SDL_Renderer *mRenderer = nullptr;
};

#endif // SDLGRAPHICS_H
