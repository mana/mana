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

#ifndef SETUP_H
#define SETUP_H

#include "gui/widgets/window.h"

#include "guichanfwd.h"

#include <guichan/actionlistener.hpp>

#include <list>

class SetupTab;

/**
 * The setup dialog. Displays several tabs for configuring different aspects
 * of the game.
 *
 * @see Setup_Audio
 * @see Setup_Colors
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

    private:
        std::list<SetupTab*> mTabs;
        gcn::Button *mResetWindows;
};

extern Setup* setupWindow;

#endif
