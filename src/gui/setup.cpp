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

/*
 *  This module takes care of everything relating to the
 *  setup dialog.
 */

#include "setup.h"
#include "button.h"
#include "checkbox.h"
#include "scrollarea.h"
#include "listbox.h"
#include "radiobutton.h"
#include "ok_dialog.h"
#include "../main.h"

#ifndef WIN32
extern Sound sound;
#endif /* no WIN32 */

struct Modes {
    int height, width;
    char *desc;
};

static Modes modes[] = {
    { 640,480, "640x480"},
    { 800,600, "800x600" },
    { 1024,768, "1024x768" }
};

int ModeListModel::getNumberOfElements() {
    // TODO after moving to SDL
    return 3;
}

std::string ModeListModel::getElementAt(int i) {
    // TODO: after moving to SDL
    return modes[i].desc;
}

Setup::Setup():
    Window("Setup")
{
    modeListModel = new ModeListModel();
    displayLabel = new gcn::Label("Display settings");
    modeList = new ListBox(modeListModel);
    scrollArea = new ScrollArea(modeList);
    fsCheckBox = new CheckBox("Full screen", false);
    soundLabel = new gcn::Label("Sound settings");
    soundCheckBox = new CheckBox("Sound", false);
    disabledRadio = new RadioButton("Disabled", "Modes", false);
    applyButton = new Button("Apply");
    cancelButton = new Button("Cancel");


    // Set events
    applyButton->setEventId("apply");
    cancelButton->setEventId("cancel");

    // Set dimensions/positions
    setSize(240, 216);
    scrollArea->setDimension(gcn::Rectangle(10, 40, 90, 50));
    modeList->setDimension(gcn::Rectangle(0, 0, 60, 50));
    displayLabel->setDimension(gcn::Rectangle(10, 10, 100,16));
    cancelButton->setPosition(
            240 - 5 - cancelButton->getWidth(),
            216 - 5 - cancelButton->getHeight());
    applyButton->setPosition(
            cancelButton->getX() - 5 - applyButton->getWidth(),
            216 - 5 - applyButton->getHeight());
    soundLabel->setPosition(10, 110);
    fsCheckBox->setPosition(120, 36);
    soundCheckBox->setPosition(10, 130);
    disabledRadio->setPosition(10, 140);

    // Listen for actions
    applyButton->addActionListener(this);
    cancelButton->addActionListener(this);

    // Assemble dialog
    add(scrollArea);
    add(displayLabel);
    add(fsCheckBox);
    add(soundLabel);
    add(soundCheckBox);
    //add(disabledRadio);
    add(applyButton);
    add(cancelButton);

    setLocationRelativeTo(getParent());

    // load default settings
    modeList->setSelected(1);
    if (config.getValue("screen", 0) == 1) {
        fsCheckBox->setMarked(true);
    }
    soundCheckBox->setMarked(config.getValue("sound", 0));
}

Setup::~Setup() {
    delete modeListModel;
    delete modeList;
    delete scrollArea;
    delete fsCheckBox;
    delete soundCheckBox;
    delete soundLabel;
    delete displayLabel;
    delete applyButton;
    delete cancelButton;
}

void Setup::action(const std::string& eventId)
{
    if (eventId == "apply") {
        setVisible(false);
        int sel = modeList->getSelected();

        // Display settings
        if (fsCheckBox->isMarked() && config.getValue("screen", 0) == 2)
        {
            config.setValue("screen", 1);
            set_gfx_mode(GFX_AUTODETECT_FULLSCREEN,
                    modes[sel].height, modes[sel].width, 0, 0);

        }
        else if (!fsCheckBox->isMarked() && config.getValue("screen", 0) == 1)
        {
            config.setValue("screen", 2);
            set_gfx_mode(GFX_AUTODETECT_WINDOWED,
                    modes[sel].height, modes[sel].width, 0, 0);
        }

        // Sound settings
#ifndef WIN32
        if (soundCheckBox->isMarked()) {
            config.setValue("sound",1);
            try {
                sound.init(32, 20);
            }
            catch (const char *err) {
                new OkDialog(this, "Sound Engine", err);
                warning(err);   
            }
        } else {
            config.setValue("sound", 0);
            sound.close();
        }
#endif /* not WIN32 */
    } else if (eventId == "cancel") {
        setVisible(false);
    }
}

Setup *Setup::ptr = NULL;
Setup *Setup::create_setup() {
    if (ptr == NULL) {
        ptr = new Setup();
    }
    else {
        ptr->setVisible(true);
    }

    return ptr;
}
