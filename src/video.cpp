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

#include "video.h"

#include "log.h"
#include "sdlgraphics.h"
#include "utils/stringutils.h"

#ifdef USE_OPENGL
#include "openglgraphics.h"
#endif

#include <algorithm>

static constexpr int WINDOW_MIN_WIDTH = 640;
static constexpr int WINDOW_MIN_HEIGHT = 400;

int VideoSettings::scale() const
{
    if (userScale == 0)
        return autoScale();

    return std::clamp(userScale, 1, maxScale());
}

int VideoSettings::autoScale() const
{
    // Automatic scaling factor based on at least 800x600 logical resolution
    return std::max(1, std::min(width / 800, height / 600));
}

int VideoSettings::maxScale() const
{
    // Logical resolution needs to stay at least the minimum size
    return std::max(1, std::min(width / WINDOW_MIN_WIDTH,
                                height / WINDOW_MIN_HEIGHT));
}

Video::~Video()
{
    mGraphics.reset();  // reset graphics first

    if (mWindow)
        SDL_DestroyWindow(mWindow);
}

Graphics *Video::initialize(const VideoSettings &settings)
{
    mSettings = settings;

    if (!initDisplayModes())
    {
        Log::info("Failed to initialize display modes: %s", SDL_GetError());
    }

    SDL_DisplayMode displayMode;

    if (mSettings.windowMode == WindowMode::Fullscreen)
    {
        SDL_DisplayMode requestedMode;
        requestedMode.format = 0;
        requestedMode.w = mSettings.width;
        requestedMode.h = mSettings.height;
        requestedMode.refresh_rate = 0;
        requestedMode.driverdata = nullptr;

        if (SDL_GetClosestDisplayMode(mSettings.display, &requestedMode, &displayMode) == nullptr)
        {
            Log::info("SDL_GetClosestDisplayMode failed: %s, falling back to borderless mode", SDL_GetError());
            mSettings.windowMode = WindowMode::WindowedFullscreen;
        }
    }

    int windowFlags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    const char *videoMode = "windowed";

    switch (mSettings.windowMode)
    {
    case WindowMode::Windowed:
        // In windowed mode, the window is initially created hidden, we'll show
        // it once we have finished setting it up and done an initial paint to
        // avoid startup flicker on X11 and Windows.
        windowFlags |= SDL_WINDOW_HIDDEN;
        break;
    case WindowMode::Fullscreen:
        windowFlags |= SDL_WINDOW_FULLSCREEN;
        videoMode = "fullscreen";
        break;
    case WindowMode::WindowedFullscreen:
        windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        // On Windows, fullscreen desktop with OpenGL actually flickers worse
        // when the window is initially hidden.
        if (!mSettings.openGL)
            windowFlags |= SDL_WINDOW_HIDDEN;
        videoMode = "windowed fullscreen";
        break;
    }

    if (mSettings.openGL)
        windowFlags |= SDL_WINDOW_OPENGL;

    Log::info("Setting video mode %dx%d %s",
              mSettings.width,
              mSettings.height,
              videoMode);

    mWindowShown = !(windowFlags & SDL_WINDOW_HIDDEN);
    mWindow = SDL_CreateWindow("Mana",
                               SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               mSettings.width,
                               mSettings.height,
                               windowFlags);

    if (!mWindow)
    {
        Log::critical(strprintf("Failed to create window: %s", SDL_GetError()));
        return nullptr;
    }

    SDL_SetWindowMinimumSize(mWindow, WINDOW_MIN_WIDTH, WINDOW_MIN_HEIGHT);

    if (mSettings.windowMode == WindowMode::Fullscreen)
    {
        if (SDL_SetWindowDisplayMode(mWindow, &displayMode) != 0)
        {
            Log::info("SDL_SetWindowDisplayMode failed: %s", SDL_GetError());
        }
    }

    // Make sure the resolution is reflected in current settings
    SDL_GetWindowSize(mWindow, &mSettings.width, &mSettings.height);

#ifdef USE_OPENGL
    if (mSettings.openGL)
    {
        mGraphics = OpenGLGraphics::create(mWindow, mSettings);
        if (!mGraphics)
        {
            Log::info("Failed to create OpenGL context, falling back to SDL renderer: %s",
                        SDL_GetError());
            mSettings.openGL = false;
        }
    }
#endif

    if (!mGraphics)
        mGraphics = SDLGraphics::create(mWindow, mSettings);

    mGraphics->updateSize(mSettings.width, mSettings.height, mSettings.scale());

    return mGraphics.get();
}

bool Video::apply(const VideoSettings &settings)
{
    if (mSettings == settings)
        return true;

    // When changing to fullscreen mode, we set the display mode first
    if (settings.windowMode == WindowMode::Fullscreen)
    {
        SDL_DisplayMode displayMode;
        if (SDL_GetWindowDisplayMode(mWindow, &displayMode) != 0)
        {
            Log::critical(strprintf("SDL_GetCurrentDisplayMode failed: %s", SDL_GetError()));
            return false;
        }

        if (displayMode.w != settings.width || displayMode.h != settings.height)
        {
#ifdef __APPLE__
            // Workaround SDL2 issue when switching display modes while already
            // fullscreen on macOS (tested as of SDL 2.30.0).
            if (SDL_GetWindowFlags(mWindow) & SDL_WINDOW_FULLSCREEN)
                SDL_SetWindowFullscreen(mWindow, 0);
#endif

            displayMode.w = settings.width;
            displayMode.h = settings.height;

            if (SDL_SetWindowDisplayMode(mWindow, &displayMode) != 0)
            {
                Log::critical(strprintf("SDL_SetWindowDisplayMode failed: %s", SDL_GetError()));
                return false;
            }
        }
    }

    int windowFlags = 0;
    switch (settings.windowMode)
    {
    case WindowMode::Windowed:
        break;
    case WindowMode::WindowedFullscreen:
        windowFlags = SDL_WINDOW_FULLSCREEN_DESKTOP;
        break;
    case WindowMode::Fullscreen:
        windowFlags = SDL_WINDOW_FULLSCREEN;
        break;
    }

    if (SDL_SetWindowFullscreen(mWindow, windowFlags) != 0)
    {
        Log::critical(strprintf("SDL_SetWindowFullscreen failed: %s", SDL_GetError()));
        return false;
    }

    if (settings.windowMode == WindowMode::Windowed &&
            (settings.width != mSettings.width ||
             settings.height != mSettings.height)) {
#ifdef __APPLE__
        // Workaround SDL2 issue when setting the window size on a window
        // which the user has put in fullscreen. Unfortunately, this mode can't
        // be distinguished from a maximized window (tested as of SDL 2.30.0).
        if (!(SDL_GetWindowFlags(mWindow) & SDL_WINDOW_MAXIMIZED))
#endif
            SDL_SetWindowSize(mWindow, settings.width, settings.height);
    }

    mSettings = settings;

    SDL_GetWindowSize(mWindow, &mSettings.width, &mSettings.height);

    mGraphics->setVSync(mSettings.vsync);
    mGraphics->updateSize(mSettings.width, mSettings.height, mSettings.scale());

    return true;
}

void Video::updateWindowSize()
{
    SDL_GetWindowSize(mWindow, &mSettings.width, &mSettings.height);
    mGraphics->updateSize(mSettings.width,
                          mSettings.height,
                          mSettings.scale());
}

void Video::present()
{
    mGraphics->updateScreen();

    if (!mWindowShown)
    {
        SDL_ShowWindow(mWindow);
        mWindowShown = true;
    }
}

bool Video::initDisplayModes()
{
    const int displayIndex = mSettings.display;
    SDL_DisplayMode mode;
    if (SDL_GetDesktopDisplayMode(displayIndex, &mode) != 0)
        return false;

    mDesktopDisplayMode.width = mode.w;
    mDesktopDisplayMode.height = mode.h;

    // Get available fullscreen/hardware modes
    const int numModes = SDL_GetNumDisplayModes(displayIndex);
    for (int i = 0; i < numModes; i++)
    {
        if (SDL_GetDisplayMode(displayIndex, i, &mode) != 0)
            return false;

        // Skip the unreasonably small modes
        if (mode.w < WINDOW_MIN_WIDTH || mode.h < WINDOW_MIN_HEIGHT)
            continue;

        // Only list each resolution once
        // (we currently don't support selecting the refresh rate)
        if (std::find_if(mDisplayModes.cbegin(),
                         mDisplayModes.cend(),
                         [&mode](const DisplayMode &other) {
                             return mode.w == other.width && mode.h == other.height;
                         }) != mDisplayModes.cend())
            continue;

        mDisplayModes.push_back(DisplayMode { mode.w, mode.h });
    }

    return true;
}
