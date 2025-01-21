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

#include "graphics.h"

#include <memory>
#include <vector>

static constexpr int defaultScreenWidth = 1280;
static constexpr int defaultScreenHeight = 720;

enum class WindowMode
{
    Windowed = 0,
    WindowedFullscreen = 1,
    Fullscreen = 2,
};

struct DisplayMode
{
    int width = 0;
    int height = 0;
};

struct VideoSettings
{
    WindowMode windowMode = WindowMode::Windowed;
    int width = defaultScreenWidth;
    int height = defaultScreenHeight;
    int display = 0;
    int userScale = 0;
    bool vsync = true;
    bool openGL = false;

    int scale() const;
    int autoScale() const;
    int maxScale() const;

    bool operator==(const VideoSettings &other) const
    {
        return width == other.width &&
                height == other.height &&
                windowMode == other.windowMode &&
                display == other.display &&
                userScale == other.userScale &&
                vsync == other.vsync &&
                openGL == other.openGL;
    }
};

class Video
{
public:
    Video() = default;
    ~Video();

    const VideoSettings &settings() const { return mSettings; }
    SDL_Window *window() const { return mWindow; }
    Graphics *graphics() const { return mGraphics.get(); }

    /**
     * Try to create a window with the given settings.
     */
    Graphics *initialize(const VideoSettings &settings);

    /**
     * Try to apply the given video settings.
     */
    bool apply(const VideoSettings &settings);

    /**
     * Handle a change in window size, possibly adjusting the scale.
     */
    void windowSizeChanged(int width, int height);

    const DisplayMode &desktopDisplayMode() const
    {
        return mDesktopDisplayMode;
    }

    const std::vector<DisplayMode> &displayModes() const
    {
        return mDisplayModes;
    }

private:
    bool initDisplayModes();

    VideoSettings mSettings;
    DisplayMode mDesktopDisplayMode;
    std::vector<DisplayMode> mDisplayModes;
    std::unique_ptr<Graphics> mGraphics;
    SDL_Window *mWindow = nullptr;
};
