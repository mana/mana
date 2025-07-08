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

#include "gui/gui.h"

#include "gui/focushandler.h"
#include "gui/palette.h"
#include "gui/sdlinput.h"
#include "gui/truetypefont.h"

#include "gui/widgets/textfield.h"
#include "gui/widgets/window.h"
#include "gui/widgets/windowcontainer.h"

#include "client.h"
#include "configuration.h"
#include "graphics.h"
#include "log.h"

#include "resources/resourcemanager.h"
#include "resources/theme.h"
#include "utils/filesystem.h"

#include <guichan/exception.hpp>
#include <guichan/image.hpp>

#include <algorithm>

#include <SDL_image.h>

// Guichan stuff
Gui *gui = nullptr;
SDLInput *guiInput = nullptr;

// Bolded font
gcn::Font *boldFont = nullptr;

// Mono font
gcn::Font *monoFont = nullptr;

bool Gui::debugDraw;

Gui::Gui(Graphics *graphics, const std::string &themePath)
    : mAvailableThemes(Theme::getAvailableThemes())
    , mCustomCursorScale(Client::getVideo().settings().scale())
{
    // Try to find the requested theme, using the first one as fallback
    auto themeIt = std::find_if(mAvailableThemes.begin(),
                                mAvailableThemes.end(),
                                [&themePath](const ThemeInfo &theme) {
                                    return theme.getPath() == themePath;
                                });

    setTheme(themeIt != mAvailableThemes.end() ? *themeIt : mAvailableThemes.front());

    Log::info("Initializing GUI...");
    // Set graphics
    setGraphics(graphics);

    // Set input
    guiInput = new SDLInput;
    setInput(guiInput);

    // Replace focus handler
    delete mFocusHandler;
    mFocusHandler = new FocusHandler;

    // Initialize top GUI widget
    auto *guiTop = new WindowContainer;
    guiTop->setFocusable(true);
    guiTop->setSize(graphics->getWidth(), graphics->getHeight());
    guiTop->setOpaque(false);
    Window::setWindowContainer(guiTop);
    setTop(guiTop);

    // Set global font
    const int fontSize = config.fontSize;
    std::string fontFile = branding.getValue("font", "fonts/dejavusans.ttf");
    std::string path = ResourceManager::getPath(fontFile);

    // Initialize the font scale before creating the fonts
    TrueTypeFont::updateFontScale(graphics->getScale());

    try
    {
        mGuiFont = new TrueTypeFont(path, fontSize);
        mInfoParticleFont = new TrueTypeFont(path, fontSize, TTF_STYLE_BOLD);
    }
    catch (gcn::Exception e)
    {
        Log::critical(std::string("Unable to load '") + fontFile +
                      "': " + e.getMessage());
    }

    // Set bold font
    fontFile = branding.getValue("boldFont", "fonts/dejavusans-bold.ttf");
    path = ResourceManager::getPath(fontFile);
    try
    {
        boldFont = new TrueTypeFont(path, fontSize);
    }
    catch (gcn::Exception e)
    {
        Log::critical(std::string("Unable to load '") + fontFile +
                      "': " + e.getMessage());
    }

    // Set mono font
    fontFile = branding.getValue("monoFont", "fonts/dejavusans-mono.ttf");
    path = ResourceManager::getPath(fontFile);
    try
    {
        monoFont = new TrueTypeFont(path, fontSize);
    }
    catch (gcn::Exception e)
    {
        Log::critical(std::string("Unable to load '") + fontFile +
                      "': " + e.getMessage());
    }

    loadCustomCursors();
    loadSystemCursors();

    gcn::Widget::setGlobalFont(mGuiFont);

    // Initialize mouse cursor and listen for changes to the option
    setUseCustomCursor(config.customCursor);

    listen(Event::ConfigChannel);
}

Gui::~Gui()
{
    for (auto cursor : mSystemMouseCursors)
        SDL_FreeCursor(cursor);

    for (auto cursor : mCustomMouseCursors)
        SDL_FreeCursor(cursor);

    delete mGuiFont;
    delete boldFont;
    delete monoFont;
    delete mInfoParticleFont;
    delete getTop();

    delete guiInput;
}

void Gui::logic()
{
    // Hide mouse cursor after extended inactivity
    if (mMouseActivityTimer.passed())
        SDL_ShowCursor(SDL_DISABLE);

    Palette::advanceGradients();

    gcn::Gui::logic();

    while (!guiInput->isTextQueueEmpty())
    {
        TextInput textInput = guiInput->dequeueTextInput();
        handleTextInput(textInput);
    }
}

void Gui::event(Event::Channel channel, const Event &event)
{
    if (channel == Event::ConfigChannel)
    {
        if (event.getType() == Event::ConfigOptionChanged &&
            event.hasValue(&Config::customCursor))
        {
            setUseCustomCursor(config.customCursor);
        }
    }
}

bool Gui::videoResized(int width, int height)
{
    const float graphicsScale = static_cast<Graphics*>(mGraphics)->getScale();
    const float userScale = Client::getVideo().settings().scale();

    TrueTypeFont::updateFontScale(graphicsScale);

    if (mCustomCursorScale != userScale)
    {
        mCustomCursorScale = userScale;
        loadCustomCursors();
        updateCursor();
    }

    auto *top = static_cast<WindowContainer*>(getTop());

    int oldWidth = top->getWidth();
    int oldHeight = top->getHeight();
    if (oldWidth == width && oldHeight == height)
        return false;

    top->setSize(width, height);
    top->adjustAfterResize(oldWidth, oldHeight);
    return true;
}

void Gui::setUseCustomCursor(bool customCursor)
{
    if (mCustomCursor == customCursor)
        return;

    mCustomCursor = customCursor;
    updateCursor();
}

void Gui::setCursorType(Cursor cursor)
{
    if (mCursorType == cursor)
        return;

    mCursorType = cursor;
    updateCursor();
}

void Gui::setTheme(const ThemeInfo &theme)
{
    mTheme = std::make_unique<Theme>(theme);
}

void Gui::updateCursor()
{
    if (mCustomCursor && !mCustomMouseCursors.empty())
        SDL_SetCursor(mCustomMouseCursors[static_cast<int>(mCursorType)]);
    else
        SDL_SetCursor(mSystemMouseCursors[static_cast<int>(mCursorType)]);
}

void Gui::handleMouseMoved(const gcn::MouseInput &mouseInput)
{
    gcn::Gui::handleMouseMoved(mouseInput);
    mMouseActivityTimer.set(15000);

    // Make sure the cursor is visible
    SDL_ShowCursor(SDL_ENABLE);
}

void Gui::handleTextInput(const TextInput &textInput)
{
    if (auto focused = mFocusHandler->getFocused())
    {
        if (auto textField = dynamic_cast<TextField*>(focused))
        {
            textField->textInput(textInput);
        }
    }
}

static SDL_Surface *loadSurface(const std::string &path)
{
    if (SDL_RWops *file = FS::openRWops(path))
        return IMG_Load_RW(file, 1);
    return nullptr;
}

void Gui::loadCustomCursors()
{
    for (auto cursor : mCustomMouseCursors)
        SDL_FreeCursor(cursor);

    mCustomMouseCursors.clear();

    const std::string cursorPath = mTheme->resolvePath("mouse.png");
    SDL_Surface *mouseSurface = loadSurface(cursorPath);
    if (!mouseSurface)
    {
        Log::warn("Unable to load mouse cursor file (%s): %s",
                  cursorPath.c_str(), SDL_GetError());
        return;
    }

    SDL_SetSurfaceBlendMode(mouseSurface, SDL_BLENDMODE_NONE);

    constexpr int cursorSize = 40;
    const int targetCursorSize = cursorSize * mCustomCursorScale;
    const int columns = mouseSurface->w / cursorSize;

    SDL_Surface *cursorSurface = SDL_CreateRGBSurfaceWithFormat(
                0, targetCursorSize, targetCursorSize, 32,
                SDL_PIXELFORMAT_RGBA32);

    for (int i = 0; i < static_cast<int>(Cursor::Count); ++i)
    {
        int x = i % columns * cursorSize;
        int y = i / columns * cursorSize;

        SDL_Rect srcrect = { x, y, cursorSize, cursorSize };
        SDL_Rect dstrect = { 0, 0, targetCursorSize, targetCursorSize };
        SDL_BlitScaled(mouseSurface, &srcrect, cursorSurface, &dstrect);

        SDL_Cursor *cursor = SDL_CreateColorCursor(cursorSurface,
                                                   15 * mCustomCursorScale,
                                                   17 * mCustomCursorScale);
        if (!cursor)
        {
            Log::warn("Unable to create cursor: %s", SDL_GetError());
        }

        mCustomMouseCursors.push_back(cursor);
    }

    SDL_FreeSurface(cursorSurface);
    SDL_FreeSurface(mouseSurface);
}

void Gui::loadSystemCursors()
{
    constexpr struct {
        Cursor cursor;
        SDL_SystemCursor systemCursor;
    } cursors[static_cast<int>(Cursor::Count)] = {
        { Cursor::Pointer,           SDL_SYSTEM_CURSOR_ARROW },
        { Cursor::ResizeAcross,      SDL_SYSTEM_CURSOR_SIZEWE },
        { Cursor::ResizeDown,        SDL_SYSTEM_CURSOR_SIZENS },
        { Cursor::ResizeDownLeft,    SDL_SYSTEM_CURSOR_SIZENESW },
        { Cursor::ResizeDownRight,   SDL_SYSTEM_CURSOR_SIZENWSE },
        { Cursor::Fight,             SDL_SYSTEM_CURSOR_HAND },
        { Cursor::PickUp,            SDL_SYSTEM_CURSOR_HAND },
        { Cursor::Talk,              SDL_SYSTEM_CURSOR_HAND },
        { Cursor::Action,            SDL_SYSTEM_CURSOR_HAND },
        { Cursor::Left,              SDL_SYSTEM_CURSOR_ARROW },
        { Cursor::Up,                SDL_SYSTEM_CURSOR_ARROW },
        { Cursor::Right,             SDL_SYSTEM_CURSOR_ARROW },
        { Cursor::Down,              SDL_SYSTEM_CURSOR_ARROW },
        { Cursor::Drag,              SDL_SYSTEM_CURSOR_SIZEALL },
        { Cursor::Hand,              SDL_SYSTEM_CURSOR_HAND },
    };

    for (auto cursor : cursors)
        mSystemMouseCursors.push_back(SDL_CreateSystemCursor(cursor.systemCursor));
}
