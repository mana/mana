/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef GUI_H
#define GUI_H

#include "guichanfwd.h"

#include <guichan/gui.hpp>

class Graphics;
class GuiConfigListener;
class ImageSet;
class SDLInput;

/**
 * \defgroup GUI Core GUI related classes (widgets)
 */

/**
 * \defgroup Interface User interface related classes (windows, dialogs)
 */

/**
 * Main GUI class.
 *
 * \ingroup GUI
 */
class Gui : public gcn::Gui
{
    public:
        Gui(Graphics *screen);

        ~Gui();

        /**
         * Performs logic of the GUI. Overridden to track mouse pointer
         * activity.
         */
        void logic();

        /**
         * Draws the whole Gui by calling draw functions down in the
         * Gui hierarchy. It also draws the mouse pointer.
         */
        void draw();

        gcn::FocusHandler *getFocusHandler() const
        { return mFocusHandler; }

        /**
         * Return game font.
         */
        gcn::Font *getFont() const
        { return mGuiFont; }

        /**
         * Return the Font used for "Info Particles", i.e. ones showing, what
         * you picked up, etc.
         */
        gcn::Font *getInfoParticleFont() const
        { return mInfoParticleFont; }

        /**
         * Sets whether a custom cursor should be rendered.
         */
        void setUseCustomCursor(bool customCursor);

        /**
         * Sets which cursor should be used.
         */
        void setCursorType(int index)
        { mCursorType = index; }

        /**
         * Cursors are in graphic order from left to right.
         * CURSOR_POINTER should be left untouched.
         * CURSOR_TOTAL should always be last.
         */
        enum {
            CURSOR_POINTER = 0,
            CURSOR_RESIZE_ACROSS,
            CURSOR_RESIZE_DOWN,
            CURSOR_RESIZE_DOWN_LEFT,
            CURSOR_RESIZE_DOWN_RIGHT,
            CURSOR_FIGHT,
            CURSOR_PICKUP,
            CURSOR_TALK,
            CURSOR_TOTAL
        };

    protected:
        void handleMouseMoved(const gcn::MouseInput &mouseInput);

    private:
        GuiConfigListener *mConfigListener;
        gcn::Font *mGuiFont;                  /**< The global GUI font */
        gcn::Font *mInfoParticleFont;         /**< Font for Info Particles*/
        bool mCustomCursor;                   /**< Show custom cursor */
        ImageSet *mMouseCursors;              /**< Mouse cursor images */
        float mMouseCursorAlpha;
        int mMouseInactivityTimer;
        int mCursorType;
};

extern Gui *gui;                              /**< The GUI system */
extern SDLInput *guiInput;                    /**< GUI input */

/**
 * Bolded text font
 */
extern gcn::Font *boldFont;

#endif // GUI_H
