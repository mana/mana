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

#ifndef _CHAR_SELECT_H
#define _CHAR_SELECT_H

#include "confirm_dialog.h"
#include "window.h"

#include "../guichanfwd.h"

#include <SDL_events.h>

class PlayerBox;
struct PLAYER_INFO;

/**
 * Character selection dialog.
 *
 * \ingroup Interface
 */
class CharSelectDialog : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor.
         */
        CharSelectDialog();

        void action(const std::string& eventId);

        void setPlayerInfo(PLAYER_INFO* pi);

        void logic();

    private:
        gcn::Button *selectButton;
        gcn::Button *cancelButton;
        gcn::Button *newCharButton;
        gcn::Button *delCharButton;

        gcn::Label *nameLabel;
        gcn::Label *levelLabel;
        gcn::Label *jobLevelLabel;
        gcn::Label *moneyLabel;

        PlayerBox *playerBox;

        /**
         * Communicate character deletion to the server.
         */
        void serverCharDelete();

        /**
         * Communicate character selection to the server.
         */
        void serverCharSelect();

        /**
         * Listener for confirming character deletion.
         */
        class CharDeleteConfirm : public ConfirmDialog
        {
            public:
                CharDeleteConfirm(CharSelectDialog *master);
                void action(const std::string &eventId);
            private:
                CharSelectDialog *master;
        };
};

/**
 * Character creation dialog.
 *
 * \ingroup GUI
 */
class CharCreateDialog : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor.
         */
        CharCreateDialog(Window *parent = NULL);

        /**
         * Destructor.
         */
        ~CharCreateDialog();

        void action(const std::string& eventId);

        std::string getName();

    private:
        gcn::TextField *nameField;
        gcn::Label *nameLabel;
        gcn::Button *nextHairColorButton;
        gcn::Button *prevHairColorButton;
        gcn::Label *hairColorLabel;
        gcn::Button *nextHairStyleButton;
        gcn::Button *prevHairStyleButton;
        gcn::Label *hairStyleLabel;
        gcn::Button *createButton;
        gcn::Button *cancelButton;

        PlayerBox *playerBox;

        /**
         * Communicate character creation to the server and receive new char
         * info.
         */
        void serverCharCreate();
};

void charSelectInputHandler(SDL_KeyboardEvent *keyEvent);

#endif
