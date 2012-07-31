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

#include "gui/widgets/window.h"
#include "gui/widgets/windowcontainer.h"

#include "client.h"
#include "configuration.h"
#include "eventlistener.h"
#include "graphics.h"
#include "log.h"

#include "resources/image.h"
#include "resources/imageset.h"
#include "resources/resourcemanager.h"
#include "resources/theme.h"

#include <guichan/exception.hpp>
#include <guichan/image.hpp>

// Guichan stuff
Gui *gui = 0;
SDLInput *guiInput = 0;

// Bolded font
gcn::Font *boldFont = 0;

// Mono font
gcn::Font *monoFont = 0;

class GuiConfigListener : public EventListener
{
    public:
        GuiConfigListener(Gui *g):
            mGui(g)
        {}

        void event(Event::Channel channel, const Event &event)
        {
            if (channel == Event::ConfigChannel)
            {
                if (event.getType() == Event::ConfigOptionChanged &&
                    event.getString("option") == "customcursor")
                {
                    bool bCustomCursor = config.getBoolValue("customcursor");
                    mGui->setUseCustomCursor(bCustomCursor);
                }
            }
        }
    private:
        Gui *mGui;
};

Gui::Gui(Graphics *graphics):
    mCustomCursor(false),
    mMouseCursors(NULL),
    mMouseCursorAlpha(1.0f),
    mMouseInactivityTimer(0),
    mCursorType(CURSOR_POINTER)
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
    WindowContainer *guiTop = new WindowContainer;
    guiTop->setFocusable(true);
    guiTop->setSize(graphics->getWidth(), graphics->getHeight());
    guiTop->setOpaque(false);
    Window::setWindowContainer(guiTop);
    setTop(guiTop);

    ResourceManager *resman = ResourceManager::getInstance();

    // Set global font
    const int fontSize = config.getValue("fontSize", 11);
    std::string fontFile = branding.getValue("font", "fonts/DejaVuSerifCondensed.ttf");
    std::string path = resman->getPath(fontFile);

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
    fontFile = branding.getValue("boldFont", "fonts/DejaVuSerifCondensed-Bold.ttf");
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

    gcn::Widget::setGlobalFont(mGuiFont);

    // Initialize mouse cursor and listen for changes to the option
    setUseCustomCursor(config.getBoolValue("customcursor"));
    mConfigListener = new GuiConfigListener(this);
    mConfigListener->listen(Event::ConfigChannel);
}

Gui::~Gui()
{
    delete mConfigListener;

    if (mMouseCursors)
        mMouseCursors->decRef();

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
    ResourceManager *resman = ResourceManager::getInstance();
    resman->clearScheduled();

    // Fade out mouse cursor after extended inactivity
    if (mMouseInactivityTimer < 100 * 15)
    {
        ++mMouseInactivityTimer;
        mMouseCursorAlpha = std::min(1.0f, mMouseCursorAlpha + 0.05f);
    }
    else
        mMouseCursorAlpha = std::max(0.0f, mMouseCursorAlpha - 0.005f);

    Palette::advanceGradients();

    gcn::Gui::logic();
}

void Gui::draw()
{
    mGraphics->pushClipArea(getTop()->getDimension());
    getTop()->draw(mGraphics);

    int mouseX, mouseY;
    Uint8 button = SDL_GetMouseState(&mouseX, &mouseY);

    mouseX /= graphics->getScale();
    mouseY /= graphics->getScale();

    if ((Client::hasMouseFocus() || button & SDL_BUTTON(1))
            && mCustomCursor
            && mMouseCursorAlpha > 0.0f)
    {
        Image *mouseCursor = mMouseCursors->get(mCursorType);
        mouseCursor->setAlpha(mMouseCursorAlpha);

        static_cast<Graphics*>(mGraphics)->drawImage(
                mouseCursor,
                mouseX - 15,
                mouseY - 17);
    }

    mGraphics->popClipArea();
}

void Gui::videoResized()
{
    WindowContainer *top = static_cast<WindowContainer*>(getTop());

    int oldWidth = top->getWidth();
    int oldHeight = top->getHeight();

    top->setSize(graphics->getWidth(), graphics->getHeight());
    top->adjustAfterResize(oldWidth, oldHeight);
}

void Gui::setUseCustomCursor(bool customCursor)
{
    if (customCursor != mCustomCursor)
    {
        mCustomCursor = customCursor;

        if (mCustomCursor)
        {
            // Hide the SDL mouse cursor
            SDL_ShowCursor(SDL_DISABLE);

            // Load the mouse cursor
            mMouseCursors = Theme::getImageSetFromTheme("mouse.png", 40, 40);

            if (!mMouseCursors)
                logger->error("Unable to load mouse cursors.");
        }
        else
        {
            // Show the SDL mouse cursor
            SDL_ShowCursor(SDL_ENABLE);

            // Unload the mouse cursor
            if (mMouseCursors)
            {
                mMouseCursors->decRef();
                mMouseCursors = NULL;
            }
        }
    }
}

void Gui::handleMouseMoved(const gcn::MouseInput &mouseInput)
{
    gcn::Gui::handleMouseMoved(mouseInput);
    mMouseInactivityTimer = 0;
}
