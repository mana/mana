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

#include <allegro.h>

#include "../main.h"
#include "../net/network.h"
#include "gui.h"
#include "button.h"
#include <guichan/allegro.hpp>

/**
 * Character selection dialog.
 *
 * \ingroup GUI
 */
class CharSelectDialog : public Window, public gcn::ActionListener {
    private:
        Button *selectButton;
        Button *cancelButton;
        Button *newCharButton;
        Button *delCharButton;

        gcn::Label *nameLabel;
        gcn::Label *levelLabel;
        gcn::Label *jobLevelLabel;
        gcn::Label *moneyLabel;

    public:
        CharSelectDialog(gcn::Container *parent);
        ~CharSelectDialog();

        void action(const std::string& eventId);
        void setName(const std::string name)
        {
            nameLabel->setCaption(name);
        }
        void setLevel(const std::string level)
        {
            levelLabel->setCaption(level);
        }
        void setJobLevel(const std::string level)
        {
            jobLevelLabel->setCaption(level);
        }
        void setMoney(const std::string money)
        {
            moneyLabel->setCaption(money);
        }
};

/**
 * Character creation dialog.
 *
 * \ingroup GUI
 */
class CharCreateDialog : public Window, public gcn::ActionListener {
    private:
        gcn::TextField *nameField;
        gcn::Label *nameLabel;
        Button *nextHairColorButton;
        Button *prevHairColorButton;
        gcn::Label *hairColorLabel;
        Button *nextHairStyleButton;
        Button *prevHairStyleButton;
        gcn::Label *hairStyleLabel;

        Button *createButton;

    public:
        CharCreateDialog(gcn::Container *parent);
        ~CharCreateDialog();

        void action(const std::string& eventId);
        std::string getName() { return nameField->getText(); }
};

void charSelect();
void serverCharSelect();

void charCreate();
void serverCharDelete();
void serverCharCreate();

#endif
