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

#include "gui.h"

#include <guichan/exception.hpp>
#include <guichan/image.hpp>
#include <guichan/imagefont.hpp>
#include <SDL/SDL_ttf.h>

#include "focushandler.h"
#include "sdlinput.h"
#include "truetypefont.h"
#include "viewport.h"
#include "window.h"
#include "windowcontainer.h"

#include "../configlistener.h"
#include "../configuration.h"
#include "../graphics.h"
#include "../log.h"

#include "../resources/imageset.h"
#include "../resources/resourcemanager.h"
#include "../resources/imageloader.h"

// Guichan stuff
Gui *gui;
Viewport *viewport;
SDLInput *guiInput;

// Fonts used in showing hits
gcn::Font *hitRedFont;
gcn::Font *hitBlueFont;
gcn::Font *hitYellowFont;
// Font used to display speech and player names
gcn::Font *speechFont;

class GuiConfigListener : public ConfigListener
{
    public:
        GuiConfigListener(Gui *g):
            mGui(g)
        {}

        void optionChanged(const std::string &name)
        {
            if (name == "customcursor") {
                bool bCustomCursor = config.getValue("customcursor", 1) == 1;
                mGui->setUseCustomCursor(bCustomCursor);
            }
        }
    private:
        Gui *mGui;
};

Gui::Gui(Graphics *graphics):
    mCustomCursor(false),
    mMouseCursors(NULL),
    mCursorType(CURSOR_POINTER)
{
    logger->log("Initializing GUI...");
    // Set graphics
    setGraphics(graphics);

    // Set image loader
    static ImageLoader imageLoader;
    gcn::Image::setImageLoader(&imageLoader);

    // Set input
    guiInput = new SDLInput;
    setInput(guiInput);

    // Set focus handler
    delete mFocusHandler;
    mFocusHandler = new FocusHandler;

    // Initialize top GUI widget
    WindowContainer *guiTop = new WindowContainer();
    guiTop->setDimension(gcn::Rectangle(0, 0,
                graphics->getWidth(), graphics->getHeight()));
    guiTop->setOpaque(false);
    Window::setWindowContainer(guiTop);
    setTop(guiTop);

    // Set global font
    try {
        mGuiFont = new TrueTypeFont("data/fonts/dejavusans.ttf", 11);
    }
    catch (gcn::Exception e)
    {
        logger->log("Unable to load dejavusans.ttf: %s",
            e.getMessage().c_str());
        throw;
    }

    // Set speech font
    try
    {
        // FIXME: use another font?
        speechFont = new TrueTypeFont("data/fonts/dejavusans.ttf", 11);
    }
    catch (gcn::Exception e)
    {
        logger->log("Unable to load dejavusans.ttf: %s",
            e.getMessage().c_str());
        throw;
    }

    gcn::Widget::setGlobalFont(mGuiFont);

    // Load hits' colourful fonts
    try {
        hitRedFont = new gcn::ImageFont("graphics/gui/hits_red.png",
                "0123456789");
        hitBlueFont = new gcn::ImageFont("graphics/gui/hits_blue.png",
                "0123456789");
        hitYellowFont = new gcn::ImageFont("graphics/gui/hits_yellow.png",
                "0123456789misxp ");
    }
    catch (gcn::Exception e)
    {
        logger->log("Unable to load colored hits' fonts: %s", e.getMessage().c_str());
        throw;
    }

    // Initialize mouse cursor and listen for changes to the option
    setUseCustomCursor(config.getValue("customcursor", 1) == 1);
    mConfigListener = new GuiConfigListener(this);
    config.addListener("customcursor", mConfigListener);

    // Create the viewport
    viewport = new Viewport();
    viewport->setDimension(gcn::Rectangle(0, 0,
                graphics->getWidth(), graphics->getHeight()));
    guiTop->add(viewport);
}

Gui::~Gui()
{
    config.removeListener("customcursor", mConfigListener);
    delete mConfigListener;

    // Fonts used in showing hits
    delete hitRedFont;
    delete hitBlueFont;
    delete hitYellowFont;

    if (mMouseCursors) {
        mMouseCursors->decRef();
    }

    delete mGuiFont;
    delete speechFont;
    delete viewport;
    delete mTop;

    delete guiInput;
}

void
Gui::draw()
{
    mGraphics->pushClipArea(mTop->getDimension());
    mTop->draw(mGraphics);

    int mouseX, mouseY;
    Uint8 button = SDL_GetMouseState(&mouseX, &mouseY);

    if ((SDL_GetAppState() & SDL_APPMOUSEFOCUS || button & SDL_BUTTON(1)) &&
            mCustomCursor)
    {
        static_cast<Graphics*>(mGraphics)->drawImage(
                mMouseCursors->get(mCursorType),
                mouseX - 15,
                mouseY - 17);
    }

    mGraphics->popClipArea();
}

void
Gui::setUseCustomCursor(bool customCursor)
{
    if (customCursor != mCustomCursor)
    {
        mCustomCursor = customCursor;

        if (mCustomCursor)
        {
            // Hide the SDL mouse cursor
            SDL_ShowCursor(SDL_DISABLE);

            // Load the mouse cursor
            ResourceManager *resman = ResourceManager::getInstance();
            mMouseCursors =
                resman->getImageSet("graphics/gui/mouse.png", 40, 40);

            if (!mMouseCursors) {
                logger->error("Unable to load mouse cursors.");
            }
        }
        else
        {
            // Show the SDL mouse cursor
            SDL_ShowCursor(SDL_ENABLE);

            // Unload the mouse cursor
            if (mMouseCursors) {
                mMouseCursors->decRef();
                mMouseCursors = NULL;
            }
        }
    }
}
