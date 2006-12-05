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

// Should stay here because of Guichan being sensitive to headers order
#include <guichan/sdl/sdlinput.hpp>

#ifdef USE_OPENGL
#include "../resources/openglsdlimageloader.h"
#endif

#include "focushandler.h"
#include "window.h"
#include "windowcontainer.h"
#include "viewport.h"

#include "../configlistener.h"
#include "../configuration.h"
#include "../graphics.h"
#include "../log.h"

#include "../resources/image.h"
#include "../resources/resourcemanager.h"
#include "../resources/sdlimageloader.h"

// Guichan stuff
Gui *gui;
Viewport *viewport;                    /**< Viewport on the map. */
gcn::SDLInput *guiInput;               /**< GUI input. */

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
    mHostImageLoader(NULL),
    mMouseCursor(NULL),
    mCustomCursor(false)
{
    logger->log("Initializing GUI...");
    // Set graphics
    setGraphics(graphics);

    // Set image loader
#ifdef USE_OPENGL
    if (config.getValue("opengl", 0)) {
        mImageLoader = new OpenGLSDLImageLoader();
    } else
#endif
    {
        mImageLoader = new SDLImageLoader();
    }

    // Set input
    guiInput = new gcn::SDLInput();
    setInput(guiInput);

    gcn::Image::setImageLoader(mImageLoader);

    // Set focus handler
    delete mFocusHandler;
    mFocusHandler = new FocusHandler();

    // Initialize top GUI widget
    WindowContainer *guiTop = new WindowContainer();
    guiTop->setDimension(gcn::Rectangle(0, 0,
                graphics->getWidth(), graphics->getHeight()));
    guiTop->setOpaque(false);
    Window::setWindowContainer(guiTop);
    setTop(guiTop);

    // Set global font
    try {
        mGuiFont = new gcn::ImageFont("graphics/gui/sansserif8.png",
                " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ["
                "\\]^_`abcdefghijklmnopqrstuvwxyz{|}~|"
                );
    }
    catch (gcn::Exception e)
    {
        logger->error("Unable to load sansserif8.png!");
    }

    // Set speech font
    try {
        speechFont = new gcn::ImageFont("graphics/gui/rpgfont_wider.png",
                " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "0123456789.,!?-+/():;%&`'*#=[]\"<>{}^~|_@$\\"
                "áÁéÉíÍóÓúÚçë¥£¢¡¿àãõêñÑöüäÖÜÄßøèÈåÅ"
                );
    }
    catch (gcn::Exception e)
    {
        logger->error("Unable to load rpgfont_wider.png!");
    }

    gcn::Widget::setGlobalFont(mGuiFont);

    // Load hits' colourful fonts
    try {
        hitRedFont = new gcn::ImageFont("graphics/gui/hits_red.png",
                "0123456789");
        hitBlueFont = new gcn::ImageFont("graphics/gui/hits_blue.png",
                "0123456789");
        hitYellowFont = new gcn::ImageFont("graphics/gui/hits_yellow.png",
                "mis");
    }
    catch (gcn::Exception e)
    {
        logger->error("Unable to load colored hits' fonts!");
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

    if (mMouseCursor) {
        mMouseCursor->decRef();
    }

    delete mGuiFont;
    delete speechFont;
    delete viewport;
    delete mTop;
    delete mImageLoader;
    delete mHostImageLoader;

    delete guiInput;
}

void
Gui::logic()
{
    gcn::Gui::logic();

    // Work around Guichan bug of only applying focus on mouse or keyboard
    // events.
    mFocusHandler->applyChanges();
}

void
Gui::draw()
{
    mGraphics->pushClipArea(mTop->getDimension());
    mTop->draw(mGraphics);

    int mouseX, mouseY;
    Uint8 button = SDL_GetMouseState(&mouseX, &mouseY);

    if ((SDL_GetAppState() & SDL_APPMOUSEFOCUS || button & SDL_BUTTON(1))
            && mCustomCursor)
    {
        dynamic_cast<Graphics*>(mGraphics)->drawImage(mMouseCursor,
                                                      mouseX - 5,
                                                      mouseY - 2);
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
            mMouseCursor = resman->getImage("graphics/gui/mouse.png");
            if (!mMouseCursor) {
                logger->error("Unable to load mouse cursor.");
            }
        }
        else
        {
            // Show the SDL mouse cursor
            SDL_ShowCursor(SDL_ENABLE);

            // Unload the mouse cursor
            if (mMouseCursor) {
                mMouseCursor->decRef();
                mMouseCursor = NULL;
            }
        }
    }
}
