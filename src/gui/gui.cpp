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

#include <guichan/exception.hpp>
#include <guichan/image.hpp>

#include <SDL_image.h>

// Guichan stuff
Gui *gui = nullptr;
SDLInput *guiInput = nullptr;

// Bolded font
gcn::Font *boldFont = nullptr;

// Mono font
gcn::Font *monoFont = nullptr;

bool Gui::debugDraw;

Gui::Gui(Graphics *graphics)
    : mCustomCursorScale(graphics->getScale())
{
    logger->log("Initializing GUI...");
    // Set graphics
    setGraphics(graphics);

    // Set input
    guiInput = new SDLInput;
    setInput(guiInput);

    // Set focus handler
    delete mFocusHandler;
    mFocusHandler = new FocusHandler;

    // Initialize top GUI widget
    auto *guiTop = new WindowContainer;
    guiTop->setFocusable(true);
    guiTop->setSize(graphics->getWidth(), graphics->getHeight());
    guiTop->setOpaque(false);
    Window::setWindowContainer(guiTop);
    setTop(guiTop);

    ResourceManager *resman = ResourceManager::getInstance();

    // Set global font
    const int fontSize = config.getValue("fontSize", 11);
    std::string fontFile = branding.getValue("font", "fonts/dejavusans.ttf");
    std::string path = resman->getPath(fontFile);

    // Initialize the font scale before creating the fonts
    TrueTypeFont::updateFontScale(graphics->getScale());

    try
    {
        mGuiFont = new TrueTypeFont(path, fontSize);
        mInfoParticleFont = new TrueTypeFont(path, fontSize, TTF_STYLE_BOLD);
    }
    catch (gcn::Exception e)
    {
        logger->error(std::string("Unable to load '") + fontFile +
                      std::string("': ") + e.getMessage());
    }

    // Set bold font
    fontFile = branding.getValue("boldFont", "fonts/dejavusans-bold.ttf");
    path = resman->getPath(fontFile);
    try
    {
        boldFont = new TrueTypeFont(path, fontSize);
    }
    catch (gcn::Exception e)
    {
        logger->error(std::string("Unable to load '") + fontFile +
                      std::string("': ") + e.getMessage());
    }

    // Set mono font
    fontFile = branding.getValue("monoFont", "fonts/dejavusans-mono.ttf");
    path = resman->getPath(fontFile);
    try
    {
        monoFont = new TrueTypeFont(path, fontSize);
    }
    catch (gcn::Exception e)
    {
        logger->error(std::string("Unable to load '") + fontFile +
                      std::string("': ") + e.getMessage());
    }

    loadCustomCursors();
    loadSystemCursors();

    gcn::Widget::setGlobalFont(mGuiFont);

    // Initialize mouse cursor and listen for changes to the option
    setUseCustomCursor(config.getBoolValue("customcursor"));

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

    Theme::deleteInstance();
}

void Gui::logic()
{
    // Hide mouse cursor after extended inactivity
    if (get_elapsed_time(mLastMouseActivityTime) > 15000)
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
            event.getString("option") == "customcursor")
        {
            setUseCustomCursor(config.getBoolValue("customcursor"));
        }
    }
}

bool Gui::videoResized(int width, int height)
{
    const float scale = static_cast<Graphics*>(mGraphics)->getScale();

    TrueTypeFont::updateFontScale(scale);

    if (mCustomCursorScale != scale)
    {
        mCustomCursorScale = scale;
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
    mLastMouseActivityTime = tick_time;

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
    if (SDL_RWops *file = ResourceManager::getInstance()->open(path))
        return IMG_Load_RW(file, 1);
    return nullptr;
}

void Gui::loadCustomCursors()
{
    for (auto cursor : mCustomMouseCursors)
        SDL_FreeCursor(cursor);

    mCustomMouseCursors.clear();

    const std::string cursorPath = Theme::resolveThemePath("mouse.png");
    SDL_Surface *mouseSurface = loadSurface(cursorPath);
    if (!mouseSurface)
    {
        logger->log("Warning: Unable to load mouse cursor file (%s): %s",
                    cursorPath.c_str(), SDL_GetError());
        return;
    }

    SDL_SetSurfaceBlendMode(mouseSurface, SDL_BLENDMODE_NONE);

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    const Uint32 rmask = 0xff000000;
    const Uint32 gmask = 0x00ff0000;
    const Uint32 bmask = 0x0000ff00;
    const Uint32 amask = 0x000000ff;
#else
    const Uint32 rmask = 0x000000ff;
    const Uint32 gmask = 0x0000ff00;
    const Uint32 bmask = 0x00ff0000;
    const Uint32 amask = 0xff000000;
#endif

    constexpr int cursorSize = 40;
    const int targetCursorSize = cursorSize * mCustomCursorScale;
    const int columns = mouseSurface->w / cursorSize;

    SDL_Surface *cursorSurface = SDL_CreateRGBSurface(
                0, targetCursorSize, targetCursorSize, 32,
                rmask, gmask, bmask, amask);

    for (int i = 0; i <= static_cast<int>(Cursor::DOWN); ++i)
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
            logger->log("Warning: Unable to create cursor: %s", SDL_GetError());
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
    } cursors[] = {
        { Cursor::POINTER,           SDL_SYSTEM_CURSOR_ARROW },
        { Cursor::RESIZE_ACROSS,     SDL_SYSTEM_CURSOR_SIZEWE },
        { Cursor::RESIZE_DOWN,       SDL_SYSTEM_CURSOR_SIZENS },
        { Cursor::RESIZE_DOWN_LEFT,  SDL_SYSTEM_CURSOR_SIZENESW },
        { Cursor::RESIZE_DOWN_RIGHT, SDL_SYSTEM_CURSOR_SIZENWSE },
        { Cursor::FIGHT,             SDL_SYSTEM_CURSOR_HAND },
        { Cursor::PICKUP,            SDL_SYSTEM_CURSOR_HAND },
        { Cursor::TALK,              SDL_SYSTEM_CURSOR_HAND },
        { Cursor::ACTION,            SDL_SYSTEM_CURSOR_HAND },
        { Cursor::LEFT,              SDL_SYSTEM_CURSOR_ARROW },
        { Cursor::UP,                SDL_SYSTEM_CURSOR_ARROW },
        { Cursor::RIGHT,             SDL_SYSTEM_CURSOR_ARROW },
        { Cursor::DOWN,              SDL_SYSTEM_CURSOR_ARROW }
    };

    for (auto cursor : cursors)
        mSystemMouseCursors.push_back(SDL_CreateSystemCursor(cursor.systemCursor));
}
