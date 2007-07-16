/*
 *  The Mana World
 *  Copyright 2007 The Mana World Development Team
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
 */

#include "setup_keyboard.h"

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "ok_dialog.h"

#include "../configuration.h"
#include "../keyboardconfig.h"
#include "../log.h"

#include "../utils/tostring.h"

#include <SDL_keyboard.h>

Setup_Keyboard::Setup_Keyboard()
{
    setOpaque(false);

    keyboard.setSetupKeyboard(this);

    mKeyLabel = new gcn::Label[keyboard.KEY_TOTAL];
    mKeyButton = new Button[keyboard.KEY_TOTAL];

    for (int i=0; i < keyboard.KEY_TOTAL; i++)
    {
        refreshAssignedKey(i);
        mKeyLabel[i].setPosition(10, 10+(i*20));
        add(&mKeyLabel[i]);

        mKeyButton[i].setCaption("Set");
        mKeyButton[i].adjustSize();
        mKeyButton[i].addActionListener(this);
        mKeyButton[i].setActionEventId("sk"+toString(i));
        mKeyButton[i].setPosition(150,5+(i*20));
        add(&mKeyButton[i]);
    }
    mMakeDefaultButton = new Button("Default", "makeDefault", this);
    mMakeDefaultButton->setPosition(200, 5);
    mMakeDefaultButton->addActionListener(this);
    add(mMakeDefaultButton);
}

Setup_Keyboard::~Setup_Keyboard()
{
    delete [] mKeyLabel;
    delete [] mKeyButton;
    delete mMakeDefaultButton;
}

void Setup_Keyboard::apply()
{
    if (keyboard.hasConflicts())
    {
        new OkDialog("Key Conflict(s) Detected.",
            "One or more key conflicts has been detected. "
            "Resolve them immediately, "
            "or gameplay might result in unpredictable behaviour");
    }
    keyboard.setEnabled(true);
    keyboard.store();
}

void Setup_Keyboard::cancel()
{
    keyboard.retrieve();
    keyboard.setEnabled(true);
    refreshKeys();
}

void Setup_Keyboard::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "makeDefault")
    {
        keyboard.makeDefault();
        refreshKeys();
        return;
    }
    for (int i=0; i < keyboard.KEY_TOTAL; i++)
    {
        if (event.getId() == "sk"+toString(i))
        {
            keyboard.setEnabled(false);
            keyboard.setNewKeyIndex(i);
            enableButtons(false);
            mKeyLabel[i].setCaption(keyboard.getKeyCaption(i) + ": ?");
        }
    }
}


void Setup_Keyboard::enableButtons(bool bValue)
{
    for (int i=0; i < keyboard.KEY_TOTAL; i++)
    {
        mKeyButton[i].setEnabled(bValue);
    }
}

void Setup_Keyboard::refreshAssignedKey(const int index)
{
     char *temp = SDL_GetKeyName(
         (SDLKey) keyboard.getKeyValue(index));
     mKeyLabel[index].setCaption(
         keyboard.getKeyCaption(index) + ": " + toString(temp));
     mKeyLabel[index].adjustSize();
}

void Setup_Keyboard::newKeyCallback(const int index)
{
    refreshAssignedKey(index);
    enableButtons(true);
}

void Setup_Keyboard::refreshKeys()
{
    for(int i=0; i < keyboard.KEY_TOTAL; i++)
    {
        refreshAssignedKey(i);
    }
}


