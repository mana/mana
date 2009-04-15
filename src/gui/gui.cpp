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

#include "gui/gui.h"

#include "gui/focushandler.h"
#include "gui/palette.h"
#include "gui/sdlinput.h"
#include "gui/skin.h"
#include "gui/truetypefont.h"
#include "gui/viewport.h"

#include "gui/widgets/window.h"
#include "gui/widgets/windowcontainer.h"

#include "configlistener.h"
#include "configuration.h"
#include "graphics.h"
#include "log.h"

#include "resources/image.h"
#include "resources/imageset.h"
#include "resources/imageloader.h"
#include "resources/resourcemanager.h"

#include <guichan/exception.hpp>
#include <guichan/image.hpp>

// Guichan stuff
Gui *gui = 0;
Viewport *viewport = 0;                    /**< Viewport on the map. */
SDLInput *guiInput = 0;

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
            if (name == "customcursor")
            {
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
    WindowContainer *guiTop = new WindowContainer;
    guiTop->setDimension(gcn::Rectangle(0, 0,
                graphics->getWidth(), graphics->getHeight()));
    guiTop->setOpaque(false);
    Window::setWindowContainer(guiTop);
    setTop(guiTop);

    ResourceManager *resman = ResourceManager::getInstance();

    // Set global font
    const int fontSize = (int) config.getValue("fontSize", 11);
    std::string fontFile = branding.getValue("font", "fonts/dejavusans.ttf");
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

    gcn::Widget::setGlobalFont(mGuiFont);

    // Initialize mouse cursor and listen for changes to the option
    setUseCustomCursor(config.getValue("customcursor", 1) == 1);
    mConfigListener = new GuiConfigListener(this);
    config.addListener("customcursor", mConfigListener);

    // Create the viewport
    viewport = new Viewport;
    viewport->setDimension(gcn::Rectangle(0, 0,
                graphics->getWidth(), graphics->getHeight()));
    guiTop->add(viewport);
}

Gui::~Gui()
{
    config.removeListener("customcursor", mConfigListener);
    delete mConfigListener;

    if (mMouseCursors)
        mMouseCursors->decRef();

    delete mGuiFont;
    delete boldFont;
    delete mInfoParticleFont;
    delete viewport;
    delete getTop();

    delete guiInput;

    SkinLoader::deleteInstance();
}

void Gui::logic()
{
    // Fade out mouse cursor after extended inactivity
    if (mMouseInactivityTimer < 100 * 15)
    {
        ++mMouseInactivityTimer;
        mMouseCursorAlpha = std::min(1.0f, mMouseCursorAlpha + 0.05f);
    }
    else
        mMouseCursorAlpha = std::max(0.0f, mMouseCursorAlpha - 0.005f);

    guiPalette->advanceGradient();

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
