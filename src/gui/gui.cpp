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

#include <guichan/exception.hpp>
#include <guichan/image.hpp>
#include <guichan/imagefont.hpp>

#include "focushandler.h"
#include "gui.h"
#include "sdlinput.h"
#include "truetypefont.h"
#include "viewport.h"
#include "window.h"
#include "windowcontainer.h"

#include "../configlistener.h"
#include "../configuration.h"
#include "../graphics.h"
#include "../log.h"

#include "../resources/image.h"
#include "../resources/imageset.h"
#include "../resources/imageloader.h"
#include "../resources/resourcemanager.h"

// Guichan stuff
Gui *gui = 0;
Viewport *viewport = 0;                    /**< Viewport on the map. */
SDLInput *guiInput = 0;

// Fonts used in showing hits
gcn::Font *hitRedFont = 0;
gcn::Font *hitBlueFont = 0;
gcn::Font *hitYellowFont = 0;

// Bolded font
gcn::Font *boldFont = 0;

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
    mMouseCursorAlpha(1.0f),
    mMouseInactivityTimer(0),
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

    ResourceManager *resman = ResourceManager::getInstance();

    // Set global font
    std::string path = resman->getPath("fonts/dejavusans.ttf");
    try {
        const int fontSize = config.getValue("fontSize", 11);
        mGuiFont = new TrueTypeFont(path, fontSize);
    }
    catch (gcn::Exception e)
    {
        logger->error(std::string("Unable to load dejavusans.ttf: ")
            + e.getMessage());
    }

    // Set bold font
    path = resman->getPath("fonts/dejavusans-bold.ttf");
    try {
        boldFont = new TrueTypeFont(path, 11);
    }
    catch (gcn::Exception e)
    {
        logger->error(std::string("Unable to load dejavusans-bold.ttf: ")
            + e.getMessage());
    }

    gcn::Widget::setGlobalFont(mGuiFont);

    // Load hits' colourful fonts
    try {
        hitRedFont = new gcn::ImageFont("graphics/gui/hits_red.png",
                "0123456789crit! ");
        hitBlueFont = new gcn::ImageFont("graphics/gui/hits_blue.png",
                "0123456789crit! ");
        hitYellowFont = new gcn::ImageFont("graphics/gui/hits_yellow.png",
                "0123456789misxp ");
    }
    catch (gcn::Exception e)
    {
        logger->error(std::string("Unable to load colored hits' fonts: ")
                + e.getMessage());
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

    if (mMouseCursors)
        mMouseCursors->decRef();

    delete mGuiFont;
    delete boldFont;
    delete viewport;
    delete getTop();

    delete guiInput;
}

void Gui::logic()
{
    // Fade out mouse cursor after extended inactivity
    if (mMouseInactivityTimer < 100 * 15) {
        ++mMouseInactivityTimer;
        mMouseCursorAlpha = std::min(1.0f, mMouseCursorAlpha + 0.05f);
    } else {
        mMouseCursorAlpha = std::max(0.0f, mMouseCursorAlpha - 0.005f);
    }

    gcn::Gui::logic();
}

void Gui::draw()
{
    mGraphics->pushClipArea(getTop()->getDimension());
    getTop()->draw(mGraphics);

    int mouseX, mouseY;
    Uint8 button = SDL_GetMouseState(&mouseX, &mouseY);

    if ((SDL_GetAppState() & SDL_APPMOUSEFOCUS || button & SDL_BUTTON(1))
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

void Gui::handleMouseMoved(const gcn::MouseInput &mouseInput)
{
    gcn::Gui::handleMouseMoved(mouseInput);
    mMouseInactivityTimer = 0;
}
