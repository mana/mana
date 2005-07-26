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
#include "window.h"
#include "windowcontainer.h"
#include "focushandler.h"
#include "../net/protocol.h"
#include "../main.h"
#include "../engine.h"
#include "../game.h"
#include "../log.h"
#include "../resources/resourcemanager.h"

extern Being* autoTarget;

// Guichan stuff
Gui *gui;
Graphics *guiGraphics;                 // Graphics driver
gcn::SDLInput *guiInput;               // GUI input
WindowContainer *guiTop;               // The top container

// Fonts used in showing hits
gcn::ImageFont *hitRedFont;
gcn::ImageFont *hitBlueFont;
gcn::ImageFont *hitYellowFont;
// Font used to display speech and player names
gcn::ImageFont *speechFont;

Gui::Gui(Graphics *graphics):
    mHostImageLoader(NULL),
    mMouseCursor(NULL),
    mCustomCursor(false),
    mMouseWalk(false),
    mMouseX(0), mMouseY(0)
{
    // Set graphics
    guiGraphics = graphics;
    //setGraphics(graphics);
    if (useOpenGL) {
        setGraphics((gcn::OpenGLGraphics*)graphics);
    }
    else {
        setGraphics((gcn::SDLGraphics*)graphics);
    }

    // Set input
    guiInput = new gcn::SDLInput();
    setInput(guiInput);

    // Set image loader
#ifdef USE_OPENGL
    if (useOpenGL) {
        mHostImageLoader = new gcn::SDLImageLoader();
        mImageLoader = new gcn::OpenGLImageLoader(mHostImageLoader);
    }
    else {
        mImageLoader = new gcn::SDLImageLoader();
    }
#else
    mImageLoader = new gcn::SDLImageLoader();
#endif

    gcn::Image::setImageLoader(mImageLoader);

    // Set focus handler
    delete mFocusHandler;
    mFocusHandler = new FocusHandler();

    // Initialize top GUI widget
    guiTop = new WindowContainer();
    guiTop->setDimension(gcn::Rectangle(0, 0,
                graphics->getWidth(), graphics->getHeight()));
    guiTop->setOpaque(false);
    guiTop->addMouseListener(this);
    Window::setWindowContainer(guiTop);
    setTop(guiTop);

    // Set global font
    try {
        mGuiFont = new gcn::ImageFont(
                TMW_DATADIR "data/graphics/gui/sansserif8.png",
                " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ["
                "\\]^_`abcdefghijklmnopqrstuvwxyz{|}~|"
                );
    }
    catch (gcn::Exception e)
    {
        try {
            mGuiFont = new gcn::ImageFont(
                    "data/graphics/gui/sansserif8.png",
                    " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVW"
                    "XYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~|"
                    );
        }
        catch (gcn::Exception e)
        {
            logger->error("Unable to load sansserif8.png!");
        }
    }

    // Set speech font
    try {
        speechFont = new gcn::ImageFont(
                TMW_DATADIR "data/graphics/gui/rpgfont_wider.png",
                " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "0123456789.,!?-+/():;%&`'*#=[]\"<>{}^~|_@&\\"
                );
    }
    catch (gcn::Exception e)
    {
        try {
            speechFont = new gcn::ImageFont(
                    "data/graphics/gui/rpgfont_wider.png",
                    " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "0123456789.,!?-+/():;%&`'*#=[]\"<>{}^~|_@&\\"
                    );
        }
        catch (gcn::Exception e)
        {
            logger->error("Unable to load rpgfont_wider.png!");
        }
    }

    gcn::Widget::setGlobalFont(mGuiFont);

    // Load hits' colourful fonts
    try {
        hitRedFont = new gcn::ImageFont(
                TMW_DATADIR "data/graphics/gui/hits_red.png",
                "0123456789");
        hitBlueFont = new gcn::ImageFont(
                TMW_DATADIR "data/graphics/gui/hits_blue.png",
                "0123456789");
        hitYellowFont = new gcn::ImageFont(
                TMW_DATADIR "data/graphics/gui/hits_yellow.png",
                "mis");
    }
    catch (gcn::Exception e)
    {
        try {
            hitRedFont = new gcn::ImageFont(
                    "data/graphics/gui/hits_red.png",
                    "0123456789");
            hitBlueFont = new gcn::ImageFont(
                    "data/graphics/gui/hits_blue.png",
                    "0123456789");
            hitYellowFont = new gcn::ImageFont(
                    "data/graphics/gui/hits_yellow.png",
                    "mis");
        }
        catch (gcn::Exception e)
        {
            logger->error("Unable to load colored hits' fonts!");
        }
    }

    // Initialize mouse cursor and listen for changes to the option
    setUseCustomCursor(config.getValue("customcursor", 1) == 1);
    config.addListener("customcursor", this);
}

Gui::~Gui()
{
    // Fonts used in showing hits
    delete hitRedFont;
    delete hitBlueFont;
    delete hitYellowFont;

    if (mMouseCursor) {
        mMouseCursor->decRef();
    }

    delete mGuiFont;
    delete guiTop;
    delete mImageLoader;

    if (mHostImageLoader) {
        delete mHostImageLoader;
    }

    delete guiInput;
}

void Gui::logic()
{
    gcn::Gui::logic();

    // Work around Guichan bug of only applying focus on mouse or keyboard
    // events.
    mFocusHandler->applyChanges();

    if (mMouseWalk) {
        Map *tiledMap = engine->getCurrentMap();
        int tilex = mMouseX / 32 + camera_x;
        int tiley = mMouseY / 32 + camera_y;

        if (state == GAME && tiledMap->getWalk(tilex, tiley)) {
            walk(tilex, tiley, 0);
            player_node->setDestination(tilex, tiley);

            autoTarget = NULL;
        }
    }
}

void Gui::draw()
{
    if (useOpenGL) {
        dynamic_cast<gcn::OpenGLGraphics*>(guiGraphics)->pushClipArea(guiTop->getDimension());
        guiTop->draw((gcn::OpenGLGraphics*)guiGraphics);
    }
    else {
        dynamic_cast<gcn::SDLGraphics*>(guiGraphics)->pushClipArea(guiTop->getDimension());
        guiTop->draw((gcn::SDLGraphics*)guiGraphics);
    }

    int mouseX, mouseY;
    Uint8 button = SDL_GetMouseState(&mouseX, &mouseY);

    if ((SDL_GetAppState() & SDL_APPMOUSEFOCUS || button & SDL_BUTTON(1))
            && mCustomCursor)
    {
        guiGraphics->drawImage(mMouseCursor, mouseX - 5, mouseY - 2);
    }

    if (useOpenGL) {
        dynamic_cast<gcn::OpenGLGraphics*>(guiGraphics)->popClipArea();
    }
    else {
        dynamic_cast<gcn::SDLGraphics*>(guiGraphics)->popClipArea();
    }
}

void Gui::mousePress(int mx, int my, int button)
{
    // Mouse pressed on window container (basically, the map)

    // When conditions for walking are met, start moving by mouse
    if (player_node && player_node->action != DEAD && current_npc == 0 &&
            button == gcn::MouseInput::LEFT)
    {
        mMouseWalk = true;
    }
}

void Gui::mouseRelease(int x, int y, int button)
{
    gcn::MouseListener::mouseRelease(x, y, button);
    mMouseWalk = false;
}

void Gui::mouseMotion(int mx, int my)
{
    mMouseX = mx;
    mMouseY = my;
}

gcn::ImageFont *Gui::getFont()
{
    return mGuiFont;
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

void
Gui::optionChanged(const std::string &name)
{
    if (name == "customcursor") {
        setUseCustomCursor(config.getValue("customcursor", 1) == 1);
    }
}
