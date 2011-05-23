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

#ifndef SETUP_H
#define SETUP_H

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>

#include <list>

class SetupTab;

/**
 * The setup dialog. Displays several tabs for configuring different aspects
 * of the game.
 *
 * @see Setup_Audio
 * @see Setup_Colors
 * @see Setup_Interface
 * @see Setup_Joystick
 * @see Setup_Keyboard
 * @see Setup_Players
 * @see Setup_Video
 *
 * \ingroup GUI
 */
class Setup : public Window, public gcn::ActionListener
{
    public:
        Setup();
        ~Setup();

        /**
         * Event handling method.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Enables the reset button when in game.
         */
        void setInGame(bool inGame);

        void registerWindowForReset(Window *window);

        void clearWindowsForReset()
        { mWindowsToReset.clear(); }

    private:
        std::list<SetupTab*> mTabs;
        std::list<Window*> mWindowsToReset;
        gcn::Button *mResetWindows;
};

extern Setup* setupWindow;

#endif
